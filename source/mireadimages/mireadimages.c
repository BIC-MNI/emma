#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gpw.h"
#include "mex.h"
#include "minc.h"
#include "mierrors.h"         /* mine and Mark's */
#include "mexutils.h"         /* N.B. must link in mexutils.o */
#include "mincutil.h"

#define PROGNAME "mireadimages"

/*
 * Constants to check for argument number and position
 */

/* mireadimages ('minc_file', slice_vector, frame_vector[, start_line[, num_lines[, options]]]) */

#define MAX_OPTIONS        3        /* # of elements in options vector */
#define MIN_IN_ARGS        1
#define MAX_IN_ARGS        6

/* ...POS macros: 1-based, used to determine if input args are present */

#define SLICES_POS         2
#define FRAMES_POS         3
#define START_ROW_POS      4
#define NUM_ROWS_POS       5
#define OPTIONS_POS        6

/*
 * Macros to access the input and output arguments from/to MATLAB
 * (N.B. these only work in mexFunction())
 */

#define MINC_FILENAME  prhs[0]
#define SLICES         prhs[1]       /* slices to read - vector */
#define FRAMES         prhs[2]       /* ditto for frames */
#define START_ROW      prhs[3]
#define NUM_ROWS       prhs[4]
#define OPTIONS        prhs[5]
#define VECTOR_IMAGES  plhs[0]       /* array of images: one per columns */

#define MAX_READABLE   160           /* max number of slices or frames that
                                        can be read at a time */

/*
 * Global variables (with apologies).  Interesting note:  when ErrMsg is
 * declared as char [256] here, MATLAB freezes (infinite, CPU-hogging
 * loop the first time any routine tries to sprintf to it).  Dynamically
 * allocating it seems to work fine, though... go figure.
 */

Boolean    debug;
char       *ErrMsg ;             /* set as close to the occurence of the
                                    error as possible; displayed by whatever
                                    code exits */



/* ----------------------------- MNI Header -----------------------------------
@NAME       : ErrAbort
@INPUT      : msg - character to string to print just before aborting
              PrintUsage - whether or not to print a usage summary before
                aborting
              ExitCode - one of the standard codes from mierrors.h -- NOTE!  
                this parameter is NOT currently used, but I've included it for
                consistency with other functions named ErrAbort in other
                programs
@OUTPUT     : none - function does not return!!!
@RETURNS    : 
@DESCRIPTION: Optionally prints a usage summary, and calls mexErrMsgTxt with
              the supplied msg, which ABORTS the mex-file!!!
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
@NAME       : CheckBounds
@INPUT      : Slices[], Frames[] - lists of desired slices/frames
              NumSlices, NumFrames - number of elements used in each array
	      StartRow - desired starting row number (ie. offset into y-space) 
	      NumRows - number of rows to read
              Image - pointer to struct describing the image:
                # of frames/slices, etc.
@OUTPUT     : 
@RETURNS    : TRUE if no member of Slices[] or Frames[] is invalid (i.e.
              larger than, respectively, Images->Slices or Images->Frames)
              FALSE otherwise, with ErrMsg set to appropriate chastisement
@DESCRIPTION: 
@METHOD     : 
@GLOBALS    : ErrMsg
@CALLS      : 
@CREATED    : 
@MODIFIED   : 
---------------------------------------------------------------------------- */
Boolean CheckBounds (long Slices[], long Frames[],
		     long NumSlices, long NumFrames,
		     long StartRow, long NumRows,
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

   if (StartRow >= Image->Height)
   {
      sprintf (ErrMsg, "Starting row too large (max %ld)", Image->Height-1);
      return (FALSE);
   }

   if (StartRow + NumRows > Image->Height)
   {
      sprintf (ErrMsg, "Trying to read too many rows for starting row %ld (total rows: %ld)", StartRow, Image->Height);
      return (FALSE);
   }

   return (TRUE);
}     /* CheckBounds */



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
@COMMENTS   : currently handles the no-time-dimension case, but there is
              parallel code for the no-z-dimension case.
---------------------------------------------------------------------------- */
int ReadImages (ImageInfoRec *Image,
                long    Slices [],
                long    Frames [],
                long    NumSlices,
                long    NumFrames,
		long	StartRow,
		long	NumRows,
                Matrix  **Mimages)
{
   long     slice, frame;
   long     Start [MAX_NC_DIMS], Count [MAX_NC_DIMS];
   double   *VectorImages;
   Boolean  DoFrames;
   int      RetVal;           /* from miicv_get -- if this is MI_ERROR */
                              /* we have a problem!!  Should NOT!!! happen */

   /*
    * Setup start/count vectors.  We will always read from one image at
    * a time, because the user is allowed to specify slices/frames such
    * that non-contiguous images are read.  However, the image rows read
    * are always contiguous, so we'll set the Height elements of Start/
    * Count just once -- right here -- and leave them alone in the loops.
    */

   Start [Image->HeightDim] = StartRow;
















   Count [Image->HeightDim] = NumRows;
   Start [Image->WidthDim] = 0L;
   Count [Image->WidthDim] = Image->Width;
   Count [Image->SliceDim] = 1L;

   /*
    * Note: the following check for missing time dimension is based on
    * ImageInfoRec: -1 for a "dimension number" means the dimension
    * does not exist in the MIimage variable.
    */

   if ((Image->FrameDim == -1) || (Image->Frames == 0))
   {
      DoFrames = FALSE;
      NumFrames = 1;         /* so that we at least get into the frames loop */
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

   *Mimages = mxCreateFull(Image->Width*NumRows, NumSlices*NumFrames, REAL);
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

#if 0
         if (debug)
         {
            printf ("Reading: user slice %d, study slice%d",
                    slice, Slices[slice]);
            if (DoFrames)
            {
               printf ("; user frame %d, study frame %d\n",
                       frame, Frames[frame]);
            }
            else printf ("\n");
         }
#endif

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




/* ----------------------------- MNI Header -----------------------------------
@NAME       : mexFunction
@INPUT      : nlhs, nrhs - number of output/input arguments (from MATLAB)
              prhs - actual input arguments 
@OUTPUT     : plhs - actual output arguments
@RETURNS    : (void)
@DESCRIPTION: 
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 
@MODIFIED   : 
---------------------------------------------------------------------------- */
void mexFunction(int    nlhs,
                 Matrix *plhs[],
                 int    nrhs,
                 Matrix *prhs[])
{
   char        *Filename;
   ImageInfoRec ImInfo;
   long         Slice[MAX_READABLE];
   long         Frame[MAX_READABLE];
   long         NumSlices;
   long         NumFrames;
   long         StartRow;
   long         NumRows;
   FILE        *InFile;
   int          Result;

   debug = FALSE;
   ErrMsg = (char *) mxCalloc (256, sizeof (char));

   /* First make sure a valid number of arguments was given. */

   if ((nrhs < MIN_IN_ARGS) || (nrhs > MAX_IN_ARGS))
   {
      sprintf (ErrMsg, "Incorrect number of arguments");
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
    * Parse the filename option (N.B. we know it's there because we checked
    * above that nrhs >= MIN_IN_ARGS
    */

   if (ParseStringArg (MINC_FILENAME, &Filename) == NULL)
   {
      ErrAbort ("Error in filename", TRUE, ERR_ARGS);
   }

   /* Open MINC file, get info about image, and setup ICV */

   Result = OpenImage (Filename, &ImInfo, NC_NOWRITE);
   if (Result != ERR_NONE)
   {
      ErrAbort (ErrMsg, TRUE, Result);
   }

   /* 
    * If the vector of slices is given, parse it into a vector of longs.
    * If not, just read slice 0 by default.  Note that if the slice (z)
    * dimension does not exist, NumSlices is set to 0.  If the caller
    * tried to supply a list of slices anyway, a warning is printed.
    */

   if (nrhs >= SLICES_POS)
   {
      NumSlices = ParseIntArg (SLICES, MAX_READABLE, Slice);
      if (NumSlices < 0)
      {
         CloseImage (&ImInfo);
         switch (NumSlices)
         {
            case mexARGS_TOO_BIG:
               ErrMsg = "Too many slices specified";
               break;
            case mexARGS_INVALID:
               ErrMsg = "Slice vector bad format: must be numeric and one-dimensional";
               break;
         }
         ErrAbort (ErrMsg, TRUE, ERR_ARGS);
      }
      if ((ImInfo.SliceDim == -1) && (NumSlices > 0))
      {
         printf ("Warning: file has no z dimension, slices vector ignored");
         NumSlices = 0;
      }
   }
   else                    /* caller did *not* specify slices vector */
   { 
      if (ImInfo.SliceDim == -1)    /* file doesn't even have slices */
      {                             /* so don't even try to read any */
         NumSlices = 0;
      }
      else
      {
         Slice [0] = 0;       /* else just read slice 0 by default */
         NumSlices = 1;
      }
   }

   /* Now do the exact same thing for frames. */

   if (nrhs >= FRAMES_POS)
   {
      NumFrames = ParseIntArg (FRAMES, MAX_READABLE, Frame);
      if (NumFrames < 0)
      {
         CloseImage (&ImInfo);
         switch (NumFrames)
         {
            case mexARGS_TOO_BIG:
               ErrMsg = "Too many frames specified";
               break;
            case mexARGS_INVALID:
               ErrMsg = "Frame vector bad format: must be numeric and one-dimensional";
               break;
         }
         ErrAbort (ErrMsg, TRUE, ERR_ARGS);

      }
      if ((ImInfo.FrameDim == -1) && (NumFrames > 0))
      {
         printf ("Warning: file has no time dimension, frames vector ignored");
         NumFrames = 0;
      }
   }
   else
   {
      if (ImInfo.FrameDim == -1)    /* file doesn't even have frames */
      {                             /* so don't even try to read any */
         NumFrames = 0;
      }
      else
      {
         Frame [0] = 0;       /* else just read frame 0 by default */
         NumFrames = 1;
      }
   }

   /* If starting row number supplied, fetch it; likewise for row count */

   if (nrhs >= START_ROW_POS)
   {
      StartRow = (long) *(mxGetPr (START_ROW));
   }
   else
   {
      StartRow = 0;
   }

   if (nrhs >= NUM_ROWS_POS)
   {
      NumRows =  (long) *(mxGetPr (NUM_ROWS));
   }
   else   
   {
      NumRows = ImInfo.Height;
   }

   if (debug)
   {
      printf ("Starting row: %ld; Number of rows: %ld\n", StartRow, NumRows);
   }

   /* Make sure the supplied slice, frame, and row numbers are within bounds */

   if (!CheckBounds(Slice,Frame,NumSlices,NumFrames,StartRow,NumRows,&ImInfo))
   {
      CloseImage (&ImInfo);
      ErrAbort (ErrMsg, TRUE, ERR_ARGS);

   }
   

   /* And read the images to a MATLAB Matrix (of doubles!) */

   Result = ReadImages (&ImInfo, 
                        Slice, Frame, 
                        NumSlices, NumFrames, 
			StartRow, NumRows,
                        &VECTOR_IMAGES);
   if (Result != ERR_NONE) 
   {
      CloseImage (&ImInfo);
      ErrAbort (ErrMsg, TRUE, Result);
   }

   CloseImage (&ImInfo);

}     /* mexFunction */
