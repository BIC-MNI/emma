#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gpw.h"
#include "mex.h"
#include "minc.h"
#include "mierrors.h"         /* mine and Mark's */
#include "mexutils.h"			/* N.B. must link in mexutils.o */
#include "mincutil.h"

#define PROGNAME "mireadimages"

/*
 * Constants to check for argument number and position
 */

/* mireadimages ('minc_file', slice_vector, frame_vector[, options]) */

#define MAX_OPTIONS        3        /* # of elements in options vector */
#define MIN_IN_ARGS        1        /* only filename required */
#define MAX_IN_ARGS        4
#define SLICES_POS         2        /* these are 1-based! */
#define FRAMES_POS         3        /* (used to determine if certain */
#define OPTIONS_POS        4        /* input args are present) */

/*
 * Macros to access the input and output arguments from/to MATLAB
 * (N.B. these only work in mexFunction())
 */

#define MINC_FILENAME  prhs[0]
#define SLICES         prhs[1]       /* slices to read - vector */
#define FRAMES         prhs[2]       /* ditto for frames */
#define OPTIONS        prhs[3]
#define VECTOR_IMAGES  plhs[0]       /* array of images: one per columns */


/*
 * Global variables (with apologies).  Interesting note:  when ErrMsg is
 * declared as char [256] here, MATLAB freezes (infinite, CPU-hogging
 * loop the first time any routine tries to sprintf to it).  Dynamically
 * allocating it seems to work fine, though... go figure.
 */

Boolean    debug;
char       *ErrMsg ;		         /* set as close to the occurence of the
                                    error as possible; displayed by whatever
                                    code exits */



/* ----------------------------- MNI Header -----------------------------------
@NAME       : ErrAbort
@INPUT      : msg - character to string to print just before aborting
              PrintUsage - whether or not to print a usage summary before aborting
              ExitCode - one of the standard codes from mierrors.h -- NOTE!  
                this parameter is NOT currently used, but I've included it for
                consistency with other functions named ErrAbort in other
                programs
@OUTPUT     : none - function does not return!!!
@RETURNS    : 
@DESCRIPTION: Optionally prints a usage summary, and calls mexErrMsgTxt with the 
              supplied msg, which ABORTS the mex-file!!!
@METHOD     : 
@GLOBALS    : requires PROGNAME macro
@CALLS      : standard mex functions
@CREATED    : 93-6-6, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
void ErrAbort (char msg[], Boolean PrintUsage, int ExitCode)
{
   if (PrintUsage)
   {
      (void) mexPrintf ("Usage: %s ('MINC_file'[, slices", PROGNAME);
      (void) mexPrintf ("[, frames[, options]]])\n");
   }
   (void) mexErrMsgTxt (msg);
}



/* ----------------------------- MNI Header -----------------------------------
@NAME       : VerifyVectors
@INPUT      : Slices[], Frames[] - lists of desired slices/frames
              NumSlices, NumFrames - number of elements used in each array
              Image - pointer to struct describing the image:
                # of frames/slices, etc.
@OUTPUT     : 
@RETURNS    : TRUE if no member of Slices[] or Frames[] is invalid (i.e.
              larger than, respectively, Images->Slices or Images->Frames)
              FALSE otherwise
@DESCRIPTION: 
@METHOD     : 
@GLOBALS    : ErrMsg
@CALLS      : 
@CREATED    : 
@MODIFIED   : 
---------------------------------------------------------------------------- */
Boolean VerifyVectors (long Slices[], long Frames[],
                       int NumSlices, int NumFrames,
                       ImageInfoRec *Image)
{
   int   i;

   if (debug)
   { 
      printf ("Checking %d slices and %d frames for validity...\n",
              NumSlices, NumFrames);
      printf ("No slice >= %ld or frame >= %ld allowed\n",
              Image->Slices, Image->Frames);
   }

   if ((NumSlices > 1) && (NumFrames > 1))
   {
      sprintf (ErrMsg, "Cannot read both multiple slices and multiple frames");
      return (FALSE);
   }

   for (i = 0; i < NumSlices; i++)
   {
      if (debug)
      {
         printf ("User slice %d is study slice %d\n", i, Slices[i]);
      }
      if ((Slices [i] >= Image->Slices) || (Slices [i] < 0))
      {
         sprintf (ErrMsg, "Bad slice number: %ld (max %ld)", 
                  Slices[i], Image->Slices-1);
         return (FALSE);
      }
   }     /* for i - loop slices */

   for (i = 0; i < NumFrames; i++)
   {
      if (debug)
      {
         printf ("User frame %d is study frame %d\n", i, Frames[i]);
      }
      if ((Frames [i] >= Image->Frames) || (Frames [i] < 0))
      {
         sprintf (ErrMsg, "Bad frame number: %ld (max %ld)", 
                  Frames[i], Image->Frames-1);
         return (FALSE);
      }

   }     /* for i - loop frames */

   return (TRUE);
}     /* VerifyVectors */



/* ----------------------------- MNI Header -----------------------------------
@NAME       : ReadImages
@INPUT      : *Image - struct describing the image
              Slices[] - vector of zero-based slice numbers to read
              Frames[] - vector of zero-based frame numbers to read
              NumSlices - number of elements in Slices[]
              NumFrames - number of elements in Frames[]
@OUTPUT     : *Mimages - pointer to MATLAB matrix (allocated by ReadImages)
              containing the images specified by Slices[] and Frames[].
              The matrix will have Image->ImageSize rows, and each column
              will correspond to one image, with the highest dimension
              of the image variable varying fastest.  Eg., if xspace is
              the highest image dimension, then each contiguous 128 element
              block of the output matrix will correspond to one row 
              of the image.
@RETURNS    : ERR_NONE if all went well
              ERR_IN_MINC if there was an error reading the MINC file;
                this should NOT happen!!  Any errors in the input
                (eg. invalid slices or frames) should be detected before
                we reach this stage, and if miicv_get returns an error,
                that counts as a bug in THIS program.
@DESCRIPTION: Given a struct describing a MINC image variable and vectors 
              listing the slices and frames to read, reads in a series of
              images from the MINC file.  The Slices and Frames vectors
              should contain valid zero-based slice and frame numbers for
              the given MINC file.
@METHOD     : 
@GLOBALS    : debug, ErrMsg
@CALLS      : standard library, MINC functions
@CREATED    : 93-6-6, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
int ReadImages (ImageInfoRec *Image,
                long    Slices [],
                long    Frames [],
                long    NumSlices,
                long    NumFrames,
                Matrix  **Mimages)
{
   long     slice, frame;
   long     Start [MAX_NC_DIMS], Count [MAX_NC_DIMS];
   double   *VectorImages;
   Boolean  DoFrames;
   int      RetVal;           /* from miicv_get -- if this is MI_ERROR */
                              /* we have a problem!!  Should NOT!!! happen */

   /*
    * First ensure that we will always read an *entire* image, but only
    * one slice/frame at a time (no matter how many slices/frames we
    * may be reading)
    */

   Start [Image->HeightDim] = 0L;
   Count [Image->HeightDim] = Image->Height;
   Start [Image->WidthDim] = 0L;
   Count [Image->WidthDim] = Image->Width;
   Count [Image->SliceDim] = 1L;

	/*
	 * Note: the following check for missing time dimension is based on
    *	ImageInfoRec: -1 for a "dimension number" means the dimension
	 * does not exist in the MIimage variable.
	 */

	if ((Image->FrameDim == -1) || (Image->Frames == 0))
   {
      DoFrames = FALSE;
      NumFrames = 1;			  /* so that we at least get into the frames loop */
   }
   else
   {
      Count [Image->FrameDim] = 1;
      DoFrames = TRUE;
   }

   if (debug)
   {
      printf ("Reading %ld slices, %ld frames: %ld total images.",
              NumSlices, NumFrames, NumSlices*NumFrames);
      printf ("  Any time dimension: %s\n", DoFrames ? "YES" : "NO");
   }

   /* 
    * Now allocate a MATLAB matrix to put the images into, and point our local
    * VectorImages at the real part of it.
    */

   *Mimages = mxCreateFull(Image->ImageSize, NumSlices*NumFrames, REAL);
   VectorImages = mxGetPr (*Mimages);

   /*
    * Now loop through slices and frames to read in the images, one at a time.
    */

   for (slice = 0; slice < NumSlices; slice++)
   {  
      Start [Image->SliceDim] = Slices [slice];

      for (frame = 0L; frame < NumFrames; frame++)
      {
         if (DoFrames)
         {
            Start [Image->FrameDim] = Frames [frame];
         }

			if (debug)
			{
				printf ("Reading: user slice %d, study slice%d",
						  slice, Slices[slice]);
				if (DoFrames)
				{
					printf ("; user frame %d, study frame %d\n",
							  frame, Frames[frame]);
				}
			}

         RetVal = miicv_get (Image->ICV, Start, Count, VectorImages);
         if (RetVal == MI_ERROR)
         {
            sprintf (ErrMsg, "INTERNAL BUG: error code %d set by miicv_get",
                     ncerr);
            return (ERR_IN_MINC);
         }

         VectorImages += Image->ImageSize;

      }     /* for frame */

   }     /* for slice */

   return (ERR_NONE);

}     /* ReadImages */




void mexFunction(int    nlhs,
                 Matrix *plhs[],
                 int    nrhs,
                 Matrix *prhs[])
{
   char        *Filename;
   ImageInfoRec   ImInfo;
   long        Slice[MAX_NC_DIMS];
   long        Frame[MAX_NC_DIMS];
   long        NumSlices;
   long        NumFrames;
   FILE        *InFile;
   int         Result;

   debug = TRUE;        /* default for development -- can be overridden by */
                        /* OPTIONS input argument */
	ErrMsg = (char *) mxCalloc (256, sizeof (char));

   /* First make sure a valid number of arguments was given. */

   if ((nrhs < MIN_IN_ARGS) || (nrhs > MAX_IN_ARGS))
   {
      sprintf (ErrMsg, "Incorrect number of arguments (%d; min %d, max %d)",
               nrhs, MIN_IN_ARGS, MAX_IN_ARGS);
      ErrAbort (ErrMsg, TRUE, ERR_ARGS);
   }


   /* If anything was given as an options vector, parse it. */

   if (nrhs >= OPTIONS_POS)
   {
      Result = ParseOptions (OPTIONS, 1, &debug);
		if (!(Result > 0)) 
		{
			ErrAbort ("Error parsing options vector", TRUE, ERR_ARGS);
		}
   }

	/*
    * Parse the filename option -- this is required by the above check
	 * for number of arguments, so don't need to ensure that MINC_FILENAME
	 * actaually exists.
	 */

	if (debug) printf ("Parsing filename\n");
   if (ParseStringArg (MINC_FILENAME, &Filename) == NULL)
   {
		ErrAbort ("Error in filename", TRUE, ERR_ARGS);
   }

   /* Open MINC file, get info about image, and setup ICV */

	if (debug) printf ("Opening file\n");
   Result = OpenImage (Filename, &ImInfo);
	if (Result != ERR_NONE)
   {
      ErrAbort (ErrMsg, TRUE, Result);
   }

	if (debug) printf ("Parsing numerics\n");

	/* 
	 * If the vector of slices is given, parse it into a vector of longs.
	 * If not, just read slice 0 by default.
	 */

	if (nrhs >= SLICES_POS)
	{
		NumSlices = ParseIntArg (SLICES, MAX_NC_DIMS, Slice);
		if (NumSlices < 0)
		{
			CloseImage (&ImInfo);
			ErrAbort ("Error: slices must be specified in an all-integer vector",
						 TRUE, ERR_ARGS);
		}
	}
	else							/* caller did *not* specify slices vector */
	{
		Slice [0] = 0;			/* so read just slice 0 by default */
		NumSlices = 1;
	}

	/* Now do the exact same thing for frames. */

	if (nrhs >= FRAMES_POS)
	{
		NumFrames = ParseIntArg (FRAMES, MAX_NC_DIMS, Frame);
		if (NumFrames < 0)
		{
			CloseImage (&ImInfo);
			ErrAbort ("Error: frames must be specified in an all-integer vector",
						 TRUE, ERR_ARGS);
		}
	}
	else
	{
		Frame [0] = 0;				/* read just frame 0 by default */
		NumFrames = 1;
	}

	/* Make sure the supplied slice and frame numbers are within bounds */
   if (!VerifyVectors (Slice, Frame, NumSlices, NumFrames, &ImInfo))
   {
      CloseImage (&ImInfo);
      ErrAbort (ErrMsg, TRUE, ERR_ARGS);
   }
	

	/* And read the images to a MATLAB Matrix (of doubles!) */

   Result = ReadImages (&ImInfo, 
                        Slice, Frame, 
                        NumSlices, NumFrames, 
                        &VECTOR_IMAGES);
   if (Result != ERR_NONE) 
   {
      CloseImage (&ImInfo);
      ErrAbort (ErrMsg, TRUE, Result);
   }

   CloseImage (&ImInfo);

}     /* mexFunction */
