#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include "minc.h"
#include "mincutil.h"
#include "mierrors.h"

#define NUM_ARGS       4           /* STRICT requirement!! */

#define MINC_FILE      argv[1]
#define SLICE_VECTOR   argv[2]
#define FRAME_VECTOR   argv[3]
#define TEMP_FILE      argv[4]
#define PROGNAME       "miwriteimages"

#define MAX_WRITEABLE 160	/* maximum number of images we can write */
				/* in one go (this is the same as  */
				/* MAXREADABLE in mireadimages.c */
#define DEBUG

/* Error codes returned by GetVector: */

#define VECTOR_BAD -1
#define VECTOR_LONG -2
#define VECTOR_EMPTY -3

typedef enum { false=0, true=1 } Boolean;

/*
 * Global variables (with apologies)
 */

Boolean    debug = false;       /* for mincutil.c only */
char       *ErrMsg;		/* set as close to the occurence of the */
                                /* error as possible; displayed by whatever */
                                /* code exits */


/* ----------------------------- MNI Header -----------------------------------
@NAME       : ErrAbort
@INPUT      : msg - string to print before printing usage and exiting
              PrintUsage - whether or not to print a brief syntax summary
              ExitCode - code to return to caller via exit()
@OUTPUT     : (DOES NOT RETURN)
@RETURNS    : (DOES NOT RETURN)
@DESCRIPTION: Prints diagnostic and usage information for current
              program and ABORTS!!!
@METHOD     : none
@GLOBALS    : none
@CALLS      : none
@CREATED    : 93-6-4, Greg Ward
@MODIFIED   :
---------------------------------------------------------------------------- */
void ErrAbort (char msg[], Boolean PrintUsage, int ExitCode) 
{
   (void) fprintf (stderr, "Error: %s\n\n", msg);

   if (PrintUsage)
   {
      (void) fprintf (stderr, "Usage: \n");
      (void) fprintf (stderr, "%s <file name> ",PROGNAME);
      (void) fprintf (stderr, "<slices> <frames> <raw file>\n\n");

      (void) fprintf (stderr, "where the raw file consists of doubles, with height*width values per image\n");
      (void) fprintf (stderr, "(height and width are determined from the MINC file).\n\n");
   }
   exit (ExitCode);
}



/* ----------------------------- MNI Header -----------------------------------
@NAME       : GetVector
@INPUT      : vector_string -> A character string containing vector elements
                               separated by commas (eg. 1,2,3)
                               *or* an empty string *or* a string starting
                               with a - (both of which => empty vector)
              max_elements  -> The maximum number of vector elements.
@OUTPUT     : vector        -> An array of longs containing the vector
                               elements
@RETURNS    : number of vector members found (<= max_elements), or
              a negative number on failure
@DESCRIPTION: Takes a string containing a vector, and produces a one
              dimensional array of longs containing the vector elements.
@METHOD     : none
@GLOBALS    : none
@CALLS      : none
@CREATED    : June 1, 1993 by MW
@MODIFIED   : June 3, 1993 by GPW -- changed return value to an int
              indicating how many vector members were found, and added
              provision for empty vectors 
---------------------------------------------------------------------------- */
int GetVector (char vector_string[], long vector[], int max_elements)
{
   int member;
   char *token;

   /*
    * vector is empty (0 members) if string is empty OR starts with a -
    */

   if ((strlen (vector_string) == 0) || vector_string[0] == '-')
   {
      return (0);
   }
   
   member = 0;
   
   token = strtok (vector_string, ",");
   if (token != NULL)
   {
      while (token != NULL)
      {
         if (isdigit(token[0]))
         {
            vector[member++] = atoi (token);
         }
         else
         {
            return (-1);         /* non-numeric input */
         }
         if (member == max_elements)
         {
            return(-2);          /* too many members */
         }
         token = strtok (NULL, ",");
      }
   }
   else 
   {
      return (-3);               /* no tokens found (I think) */
   }
   return (member);
}     /* GetVector */



/* ----------------------------- MNI Header -----------------------------------
@NAME       : GVErrMsg
@INPUT      : Descr - description of what the list of things being
              parsed by GetVector was that resulted in error (eg.,
	      "slices" or "frames")
	      GVReturn - the return value from GetVector
@OUTPUT     : Buffer - a nice descriptive error message based on GVReturn
@RETURNS    : (void)
@DESCRIPTION: Generates human-readable error messages for the three
              possible errors returned by GetVector.  Since GetVector
	      can process a list of any type of integer, the Descr
	      argument is required to give the user some idea of what
	      list was being processed (eg., slices or frames) when
	      GetVector bombed.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 93-12-1, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
void GVErrMsg (char *Buffer, char *Descr, int GVReturn)
{
   switch (GVReturn)
   {
      case VECTOR_BAD: 
      {
	 sprintf (Buffer, "List of %s must be all numbers, "
		  "separated by commas (no spaces)", Descr);
	 break;
      }
      case VECTOR_LONG:
      {
	 sprintf (Buffer, "Too many %s specified (max %d)", 
		  Descr, MAX_WRITEABLE);
	 break;
      }
      case VECTOR_EMPTY:
      {
	 sprintf (Buffer, "List of %s not found", Descr);
	 break;
      }
   }
}     /* GVErrMsg () */




/* ----------------------------- MNI Header -----------------------------------
@NAME       : CheckBounds
@INPUT      : Slices[], Frames[] - lists of desired slices/frames
              NumSlices, NumFrames - number of elements used in each array
              Image - pointer to struct describing the image:
                # of frames/slices, etc.
@OUTPUT     : 
@RETURNS    : false if any member of Slices[] or Frames[] is out-of-bounds
	      false if the user does not supply a list of slices/frames
	         for a dimension that is present in the file
	      true otherwise
@DESCRIPTION: Ensures that no slice or frame number is out of bounds (i.e.
              less than zero, or greater than or equal to the number of
	      slices/frames: note zero-based!).  Also checks to see
	      if 1) user supplied slices (frames) and file has no slices
	      (frames); and 2) file does contain slices (frames), and user
	      supplied none.  If the first holds, a warning that the 
	      user-supplied slice (frame) list will be ignored is printed.
	      If the second holds, it's an error.
@METHOD     : 
@GLOBALS    : ErrMsg
@CALLS      : 
@CREATED    : 
@MODIFIED   : 94-12-17, GW: added check for missing/extra slice/frame lists
---------------------------------------------------------------------------- */
Boolean CheckBounds (long Slices[], long Frames[],
		     int NumSlices, int NumFrames,
		     ImageInfoRec *Image)
{
   int   i;

   /* Check if the user supplied list for a non-present image dimension */

   if ((Image->SliceDim == -1) && (NumSlices > 0))
   {
      fprintf (stderr, "Warning: file has no slice dimension; supplied slice "
	       "list will be ignored\n");
   }

   if ((Image->FrameDim == -1) && (NumFrames > 0))
   {
      fprintf (stderr, "Warning: file has no frame dimension; supplied frame "
	       "list will be ignored\n");
   }

   /* Now check if the user did not supply a list for an image dimension
    * that IS present (this is an error; the above merely a warning)
    */

   if ((Image->SliceDim != -1) && (NumSlices == 0))
   {
      sprintf (ErrMsg, "File contains slice dimension; "
	       "slice list must be provided");
      return (false);
   }

   if ((Image->FrameDim != -1) && (NumFrames == 0))
   {
      sprintf (ErrMsg, "File contains frame dimension; "
	       "frame list must be provided");
      return (false);
   }


#ifdef DEBUG
   printf("Checking slices (%d listed) and frames (%d listed) for validity\n",
	  NumSlices, NumFrames);
   printf ("No slice >= %ld or frame >= %ld allowed\n",
           Image->Slices, Image->Frames);
#endif

   for (i = 0; i < NumSlices; i++)
   {
#ifdef DEBUG
      printf ("User slice %d is study slice %ld\n", i, Slices[i]);
#endif
      if ((Slices [i] >= Image->Slices) || (Slices [i] < 0))
      {
         sprintf (ErrMsg, "Bad slice number: %ld (must be < %ld)", 
                  Slices[i], Image->Slices);
         return (false);
      }
   }     /* for i - loop slices */

   for (i = 0; i < NumFrames; i++)
   {
#ifdef DEBUG
      printf ("User frame %d is study frame %ld\n", i, Frames[i]);
#endif
      if ((Frames [i] >= Image->Frames) || (Frames [i] < 0))
      {
         sprintf (ErrMsg, "Bad frame number: %ld (must be < %ld)", 
                  Frames[i], Image->Frames);
         return (false);
      }

   }     /* for i - loop frames */

   return (true);
}     /* CheckBounds */



/* ----------------------------- MNI Header -----------------------------------
@NAME       : OpenTempFile
@INPUT      : Filename - name of the file to open
@OUTPUT     : 
@RETURNS    : pointer to the file structure if successful, NULL otherwise
@DESCRIPTION: Open a file for (binary) input.
@METHOD     : 
@GLOBALS    : ErrMsg
@CALLS      : 
@CREATED    : 93-6-3, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
FILE *OpenTempFile (char Filename [])
{
   FILE  *Newfile;

   Newfile = fopen (Filename, "rb");
   if (Newfile == NULL)
   {
      sprintf (ErrMsg, "Error opening input file %s", Filename);
      return (NULL);
   }

#ifdef DEBUG
   printf ("Temp file %s succesfully opened\n", Filename);
#endif
   
   return (Newfile);
}     /* OpenTempFile */


/* ----------------------------- MNI Header -----------------------------------
@NAME       : ReadNextImage
@INPUT      : Buffer - pointer to array of doubles where data will be put
                (must be preallocated with ImageSize doubles)
              ImageSize - number of doubles to read fromthe file
              InFile - the file to read from
@OUTPUT     : fills in Buffer
@RETURNS    : true if image was read successfully
              false if not all elements were read
@DESCRIPTION: 
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 93-6-3, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
Boolean ReadNextImage (double *Buffer, long ImageSize, FILE *InFile)
{ 
   int   AmtRead;

   AmtRead = fread (Buffer, sizeof (double), (size_t) ImageSize, InFile);

#ifdef DEBUG
   printf ("Read an image from temp file; file position now at byte %ld\n",
           ftell(InFile));
   printf ("%d doubles read; wanted %ld\n", AmtRead, ImageSize);

   if (feof (InFile))
   {
      printf ("At end of file\n");
   }
#endif

   if ((long) AmtRead != ImageSize)
   {
      return (false);
   }
   else
   {
      return (true);
   }
}     /* ReadNextImage */



/* ----------------------------- MNI Header -----------------------------------
@NAME       : PutMaxMin
@INPUT      : ImInfo - pointer to struct describing the image variable
              ImVals - pointer to array of doubles containing the image data
              SliceNum, FrameNum - needed to correctly place the max and min
                values into the MIimagemax and MIimagemin variables
              DoFrames - whether or not there is a time dimension in this file
@OUTPUT     : (none)
@RETURNS    : (void)
@DESCRIPTION: Finds the max and min values of an image, and puts them
              into the MIimagemax and MIimagemin variables associated
              with the specified image variable.  Note: the caller must
              make sure that MIimagemax and MIimagemin exist in the
              file, and ensure that ImInfo->MaxID and ImInfo->MinID contain
              their variable ID's.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 93-6-3, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
void PutMaxMin (ImageInfoRec *ImInfo, double *ImVals, 
                long SliceNum, long FrameNum, 
                Boolean DoSlices, Boolean DoFrames)
{
   int      i;
   double   Max, Min;
   long     Coord [2];          /* might use 0, 1 or 2 elements */

#ifdef DEBUG
   int      NumDims;            /* number of dimensions in imagemax/imagemin */
   int      Dims [4];           /* dimension ID's of imagemax/imagemin */

   printf ("Slice dimension is %d\n", ImInfo->SliceDim);
   printf ("Frame dimension is %d\n", ImInfo->FrameDim);

   ncvarinq (ImInfo->CDF, ImInfo->MaxID, NULL, NULL, &NumDims, Dims, NULL);
   printf ("MIimagemax has %d dimensions: ", NumDims);
   for (i = 0; i < NumDims; i++)
   {
      printf ("%5d", Dims [i]);
   }
   putchar ('\n');

   ncvarinq (ImInfo->CDF, ImInfo->MinID, NULL, NULL, &NumDims, Dims, NULL);
   printf ("MIimagemin has %d dimensions: ", NumDims);
   for (i = 0; i < NumDims; i++)
   {
      printf ("%5d", Dims [i]);
   }
   putchar ('\n');
#endif

   Max = - DBL_MAX;
   Min = DBL_MAX;

   /*
    * Find the actual max and min values in the buffer
    */

   for (i = 0; i < ImInfo->ImageSize; i++)
   {
      if (ImVals [i] > Max)
      {
         Max = ImVals [i];
      }

      if (ImVals [i] < Min)
      {
         Min = ImVals [i];
      }
   }     /* for i */

   /*
    * Now figure out the Coord vector (where to put the max and min
    * within the MIimagemax and MIimagemin variables), and put 'em there
    */

   if (DoFrames)        /* i.e. some frame was specified */
   { 
      Coord [ImInfo->FrameDim] = FrameNum;
   }

   if (DoSlices)
   {
      Coord [ImInfo->SliceDim] = SliceNum;
   }

#ifdef DEBUG
   printf ("Slice %ld, frame %ld: max is %lg, min is %lg\n", 
           (DoSlices) ? (SliceNum) : -1,
           (DoFrames) ? (FrameNum) : -1,
           Max, Min);
   if (DoSlices && DoFrames)
      printf ("Coord vector is: %ld %ld\n", Coord [0], Coord [1]);
   
#endif

   mivarput1 (ImInfo->CDF, ImInfo->MaxID, Coord, NC_DOUBLE, MI_SIGNED, &Max);
   mivarput1 (ImInfo->CDF, ImInfo->MinID, Coord, NC_DOUBLE, MI_SIGNED, &Min);

}     /* PutMaxMin */


/* ----------------------------- MNI Header -----------------------------------
@NAME       : WriteImages
@INPUT      : TempFile - where the images come from
              Image - where they're going
              Slices - vector containing list of slices to write
              Frames - vector containing list of frames to write
              NumSlices - the number of elements of Slices[] actually used
              NumFrames - the number of elements of Frames[] actually used
@OUTPUT     : 
@RETURNS    : an error code as defined in mierrors.h
              ERR_NONE = all went well
              ERR_IN_TEMP = ran out of data reading the temp file
              ERR_OUT_MINC = some problem writing to MINC file
                (this should not happen!!!)
              also sets ErrMsg in the event of an error
@DESCRIPTION: Reads images sequentially from TempFile, and writes them into
              the image variable specified by *Image at the slice/frame
              locations specified by Slices[] and Frames[].  Smart enough
              to handle files with no time dimension, but assumes there
              is always a z dimension.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 93-6-3, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
int WriteImages (FILE *TempFile,
                 ImageInfoRec *Image,
                 long Slices[], 
                 long Frames[],
                 long NumSlices,
                 long NumFrames)
{
   int      slice, frame;
   long     Start [MAX_NC_DIMS], Count [MAX_NC_DIMS];
   double   *Buffer;
   Boolean  DoFrames;
   Boolean  DoSlices;
   Boolean  Success;
   int      RetVal;

   Buffer = (double *) calloc (Image->ImageSize, sizeof (double));

   /*
    * First ensure that we will always read an *entire* image, but only
    * one slice/frame at a time (no matter how many slices/frames we
    * may be reading)
    */

   Start [Image->HeightDim] = 0; Count [Image->HeightDim] = Image->Height;
   Start [Image->WidthDim] = 0;  Count [Image->WidthDim] = Image->Width;

   /*
    * Handle files with missing frames or slices.  See the function
    * ReadImages in the file mireadimages.c for a detailed explanation.
    */

   if (NumFrames > 0)
   {
      Count [Image->FrameDim] = 1;
      DoFrames = true;
   }
   else
   {
      DoFrames = false;
      NumFrames = 1;
   }

   /* Exact same code as for frames, but changed to slices */

   if (NumSlices > 0)
   {
      Count [Image->SliceDim] = 1;
      DoSlices = true;
   }
   else
   {
      DoSlices = false;
      NumSlices = 1;
   }

#ifdef DEBUG
   printf ("Ready to start writing.\n");
   printf ("NumFrames = %ld, DoFrames = %d\n", NumFrames, (int) DoFrames);
   printf ("NumSlices = %ld, DoSlices = %d\n", NumSlices, (int) DoSlices);
#endif

  
   for (slice = 0; slice < NumSlices; slice++)
   {
      if (DoSlices)
      {
         Start [Image->SliceDim] = Slices [slice];
      }

      /* 
       * Loop through all frames, reading/writing one image each time.
       * Note that NumFrames will be one even if DoFrames is false; 
       * so this loop WILL always execute, but it and the functions it calls
       * (particularly PutMaxMin) act slightly differently depending on
       * the value of DoFrames,
       */

      for (frame = 0; frame < NumFrames; frame++)
      {
         Success = ReadNextImage (Buffer, Image->ImageSize, TempFile);
         if (!Success)
         {
            sprintf (ErrMsg, 
                    "Error reading from temporary file at slice %d, frame %d", 
                    slice, frame);
            return (ERR_IN_TEMP);
         }

         PutMaxMin (Image, Buffer,
                    Slices [slice], Frames [frame],
                    DoSlices, DoFrames);

         if (DoFrames)
         {
            Start [Image->FrameDim] = Frames [frame];
         }

         RetVal = miicv_put (Image->ICV, Start, Count, Buffer);
         if (RetVal == MI_ERROR)
         {
            sprintf (ErrMsg, "INTERNAL BUG: Fail on miicv_put: Error code %d",
                     ncerr);
            return (ERR_OUT_MINC);
         }
      }     /* for frame */
   }     /* for slice */

   /*
    * Use the MIcomplete attribute to signal that we are done writing
    */
   miattputstr (Image->CDF, Image->ID, MIcomplete, MI_TRUE);
   free (Buffer);
   return (ERR_NONE);

}     /* WriteImages */


/* ----------------------------- MNI Header -----------------------------------
@NAME       : 
@INPUT      : 
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: 
@METHOD     : 
@GLOBAL     : 
@CALLS      : 
@CREATED    : 
@MODIFIED   : 
---------------------------------------------------------------------------- */
int main (int argc, char *argv [])
{
   ImageInfoRec   ImInfo;
   long        Slice[MAX_WRITEABLE];
   long        Frame[MAX_WRITEABLE];
   long        NumSlices;
   long        NumFrames;
   FILE        *InFile;
   int         Result;
#ifdef DEBUG
   int         i;               /* loop to print out all slices and */
                                /* frames selected */
   ncopts = 0;
#endif

   ncopts = NC_VERBOSE;
   ErrMsg = (char *) malloc (256);

   strcpy (ErrMsg, "Hello! I am an error message!");

   if (argc != NUM_ARGS + 1)        /* +1 because argv[0] counts! */
   {
      ErrAbort ("Incorrect number of arguments", true, ERR_ARGS);
   }

#ifdef DEBUG
   printf ("Passing slice list %s to GetVector\n", SLICE_VECTOR);
   printf ("Passing frame list %s to GetVector\n", FRAME_VECTOR);
#endif

   /*
    * Parse the two lists of numbers first off
    */

   NumSlices = GetVector (SLICE_VECTOR, Slice, MAX_WRITEABLE);
   if (NumSlices < 0)
   {
      GVErrMsg (ErrMsg, "slices", NumSlices);
      ErrAbort (ErrMsg, true, ERR_ARGS);
   }

   NumFrames = GetVector (FRAME_VECTOR, Frame, MAX_WRITEABLE);
   if (NumFrames < 0)
   {
      GVErrMsg (ErrMsg, "frames", NumFrames);
      ErrAbort (ErrMsg, true, ERR_ARGS);
   }

#ifdef DEBUG
   printf ("Slices specified: ");
   for (i = 0; i < NumSlices; i++)
   {
      printf ("%8ld", Slice[i]);
   }
   printf ("\n");
   
   printf ("Frames specified: ");
   for (i = 0; i < NumFrames; i++)
   {
/*      printf ("i = %d, &(Frame) = %p, &(Frame[d]) = %p\n",
	      i, &Frame, &(Frame[i]));  */
      printf ("%8ld", Frame[i]); 
   }
   printf ("\n");
#endif

   if ((NumSlices > 1) && (NumFrames > 1))
   {
      ErrAbort ("Cannot specify both multiple frames and multiple slices", 
                true, ERR_ARGS);
   }

puts("Opening");
   Result = OpenImage (MINC_FILE, &ImInfo, NC_WRITE);
   if (Result != ERR_NONE)
   {
      ErrAbort (ErrMsg, true, Result);
   }

puts("Checking bounds");
   if (!CheckBounds (Slice, Frame, NumSlices, NumFrames, &ImInfo))
   {
      ErrAbort (ErrMsg, true, ERR_ARGS);
   }

puts("Checking for max/min variables");
   if ((ImInfo.MaxID == MI_ERROR) || (ImInfo.MinID == MI_ERROR))
   {
      sprintf (ErrMsg, "Missing image-max or image-min variable in file %s", 
               MINC_FILE);
      ErrAbort (ErrMsg, true, ERR_IN_MINC);
   }

puts("Opening temp file");
   InFile = OpenTempFile (TEMP_FILE);
   if (InFile == NULL)
   {
      ErrAbort (ErrMsg, true, ERR_IN_TEMP);
   }

   Result = WriteImages (InFile, &ImInfo, Slice, Frame, NumSlices, NumFrames);
   if (Result != ERR_NONE)
   {
      ErrAbort (ErrMsg, true, Result);
   }

   fclose (InFile);

   miicv_free (ImInfo.ICV);
   ncclose (ImInfo.CDF);
   return (ERR_NONE);

}     /* main */
