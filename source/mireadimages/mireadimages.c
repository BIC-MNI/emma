/* mincread.c -- MATLAB interface to read a MINC file.  

   Originally by Gabe Leger

   modified 93/5/21 GPW to return frame length as well as mid-frame times.
   modified 93/5/25 GPW to return time data just as it is stored in
      the MINC file: frame start time, and frame length.  Idea being
      that it's fairly trivial to get MATLAB to calculate the mid-frame
      times if they're needed.  Also took out the conversion from seconds
      to minutes on the same grounds.
   modified 93/5/31 GPW to not return frame times at all.  This will now
      be done by the more general purpose routine mireadvar, which should
      be used to get a hyperslab from ANY non-image variable.
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "minc.h"
#include "mex.h"

/*
 * define a few useful constants and macros
 */

typedef int Boolean;
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

#define NORMAL_STATUS 0
#define ERROR_STATUS 1

#define min(A, B) ((A) < (B) ? (A) : (B))
#define max(A, B) ((A) > (B) ? (A) : (B))

/*
 * Constants to check for argument number and position
 */

#define MAX_OPTIONS          3
#define MIN_INPUT_ARGUMENTS  2
#define MAX_INPUT_ARGUMENTS  4
#define SLICES_POSITION      3			/* these are 1-based! */
#define FRAMES_POSITION      4

/*
 * Macros to access the input and output arguments from/to MATLAB
 * (N.B. these only work in mexFunction())
 */

#define MINC_FILENAME  prhs[0]
#define OPTIONS        prhs[1]
#define SLICES         prhs[2]       /* slices to read - vector */
#define FRAMES         prhs[3]       /* ditto for frames */
#define VECTOR_IMAGES  plhs[0]       /* array of images: one per columns */

#if 0
#define TIME_VECTOR    plhs[1]       /* frame start times */
#define TIME_WIDTH     plhs[2]       /* time widths = length of each frame */
#endif

/*
 * define MINC specific stuff
 */

typedef struct {
   char *Name;
   int CdfId;
   int ImgId;
   int TimeId;
   int TimeWidthId;
   int Icv;
   int Slices;
   int Frames;
   int Height;
   int Width;
   int ImageSize;
   int nDim;
   int WidthImageDim;
   int HeightImageDim;
   int SliceDim;
   int TimeDim;
} MincInfo;




/* ----------------------------- MNI Header -----------------------------------
@NAME       : 
@INPUT      : 
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: 
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 
@MODIFIED   : 
---------------------------------------------------------------------------- */
MincInfo *open_minc_file(char *minc_filename, Boolean progress, Boolean debug)
{

   int
      dimension,
      dim_ids[MAX_VAR_DIMS],
      attribute,
      natts;
   
   long dim_length[MAX_VAR_DIMS];

   char
      image_name[256],
      dim_names[MAX_VAR_DIMS][256],
      attname[256];

   nc_type datatype;
   MincInfo *MincFile;
   Boolean
      frames_present = FALSE,
      slices_present = FALSE;

   if (progress && !debug){
      mexPrintf("Getting image information ... ");
      (void) fflush(stdout);
   }

   /* 
    * Open the file. On error, return without printing diagnostic messages 
    */

   MincFile = (MincInfo *)mxCalloc(1,sizeof(MincInfo));
   MincFile->Name = minc_filename;

   if (debug) {
      (void) mexPrintf("MincFile info:\n");
      (void) mexPrintf(" Minc filename: %s\n", MincFile->Name);
   }

   ncopts = 0;
   MincFile->CdfId = ncopen(MincFile->Name, NC_NOWRITE);
   if (MincFile->CdfId == MI_ERROR){
      printf("\nCould not open file %s.\n", MincFile->Name);
      return(NULL);
   }
   
   /* 
    * Get id for image variable 
    */
   
   MincFile->ImgId = ncvarid(MincFile->CdfId, MIimage);
   
   /* 
    * Get info about variable 
    */

   (void) ncvarinq(MincFile->CdfId, 
                   MincFile->ImgId, 
                   image_name, 
                   &datatype, 
                   &MincFile->nDim, 
                   dim_ids, 
                   &natts);
   

   if (debug){
      (void) mexPrintf(" Image variable name: %s, attributes: %d, dimensions: %d\n", 
                       image_name, natts, MincFile->nDim);
      (void) mexPrintf(" Attributes:\n");
      for (attribute = 0; attribute < natts; attribute++){
         (void) ncattname(MincFile->CdfId, MincFile->ImgId, attribute, attname);
         (void) mexPrintf("  %s\n", attname);
      }
      (void) mexPrintf(" Dimension variables:\n");
   }
      
   /*
    * For each dimension, inquire about name and get dimension length
    */

   for (dimension = 0; dimension < MincFile->nDim; dimension++){
      
      (void) ncdiminq(MincFile->CdfId,
                      dim_ids[dimension],
                      dim_names[dimension],
                      &dim_length[dimension]);

      if (debug){
         (void) mexPrintf("  %d\t%d\t%s\t%ld\n", 
                          dimension,
                          dim_ids[dimension],
                          dim_names[dimension],
                          dim_length[dimension]); 
      }
      
      if (strcmp(MItime,dim_names[dimension]) == 0){
         MincFile->TimeDim = dimension;
         MincFile->Frames = dim_length[dimension];
         frames_present = TRUE;
         if (dimension > MincFile->nDim-3){
            mexPrintf("Found time as an image dimension ... cannot continue\n");
            return(NULL);
         }

      }else if (dimension == MincFile->nDim-3 || dimension == MincFile->nDim-4){
         MincFile->SliceDim = dimension;
         MincFile->Slices = dim_length[dimension];
         slices_present = TRUE;

      }else if (dimension == MincFile->nDim-2){
         MincFile->HeightImageDim = dimension;
         MincFile->Height = dim_length[dimension];

      }else if (dimension == MincFile->nDim-1){
         MincFile->WidthImageDim = dimension;
         MincFile->Width = dim_length[dimension];

      }else{
         (void) mexPrintf("  Too many dimensions, skipping dimension <%s>\n",
                          dim_names[dimension]);

      }

   }

   MincFile->ImageSize = MincFile->Width * MincFile->Height;

   /*
    * Create the image icv 
    */

   MincFile->Icv = miicv_create();
   (void) miicv_setint(MincFile->Icv, MI_ICV_TYPE, NC_DOUBLE);
   (void) miicv_setint(MincFile->Icv, MI_ICV_DO_NORM, TRUE);
   (void) miicv_setint(MincFile->Icv, MI_ICV_USER_NORM, TRUE);
   
   /* 
    * Attach icv to image variable 
    */

   (void) miicv_attach(MincFile->Icv, MincFile->CdfId, MincFile->ImgId);

   /*
    * If frames are present, then get variable id for time
    */
   if (frames_present) {
      MincFile->TimeId = ncvarid(MincFile->CdfId, dim_names[MincFile->TimeDim]); 
      MincFile->TimeWidthId = ncvarid(MincFile->CdfId, MItime_width);  
   }else{
      MincFile->Frames = 0;
   }

   if (!slices_present) MincFile->Slices = 0;
   
   if (progress && !debug) mexPrintf("done\n");
   
   return(MincFile);

}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : 
@INPUT      : 
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: 
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 
@MODIFIED   : 
---------------------------------------------------------------------------- */
int read_minc_file(MincInfo *MincFile,
                   int      *slices,
                   int      slice_count,
                   int      *frames,
                   int      frame_count,
                   int      images_to_get,
                   Boolean  frames_specified,
/*                 double   *Time,
                   double   *TimeWidth,     */
                   double   *VectorImages,
                   Boolean  progress,
                   Boolean  debug)
{
   int
      frame,               /* just loop counters */
      slice;

#if 0
	double
      *frame_time,         /* data from the MINC file -- frame_time is just */
      *frame_length;       /* start of frame, frame_length is length */
#endif
                 
   long 
      coord[MAX_VAR_DIMS],
      count[MAX_VAR_DIMS];
   
   int notice_every;
   int image_no;

   progress = progress && !debug;
   
   /* 
    * Modify count and coord 
    */

   (void) miset_coords(MincFile->nDim, (long) 0, coord);
   (void) miset_coords(MincFile->nDim, (long) 1, count);
   count[MincFile->nDim-1] = MincFile->Width;
   count[MincFile->nDim-2] = MincFile->Height;
   coord[MincFile->nDim-1] = 0;
   coord[MincFile->nDim-2] = 0;
   
   if (progress){
      (void) mexPrintf("Reading minc data ");
      notice_every = images_to_get/50;
      if (notice_every == 0) notice_every = 1;
      image_no = 0;
   }
   
   /* 
    * Get the data 
    */
   
   for (slice = 0; slice < slice_count; slice++){

      if (MincFile->Slices > 0)
         coord[MincFile->SliceDim] = slices[slice]-1;
      
      for (frame = 0; frame < frame_count; frame++){

         if (MincFile->Frames > 0)
            coord[MincFile->TimeDim] = frames[frame]-1;
         
         (void) miicv_get(MincFile->Icv, coord, count, (void *) VectorImages);
         VectorImages += MincFile->ImageSize;
         
         if (debug){
            (void) mexPrintf(" Slice: %d, Frame: %d\n",
                             MincFile->Slices > 0 ? coord[MincFile->SliceDim]+1 : 1,
                             MincFile->Frames > 0 ? coord[MincFile->TimeDim]+1 : 1);
         }
         
         if (progress){
            if (image_no++ % notice_every == 0){
               (void) mexPrintf(".");
            }
         }
         
      } /* for frame */
      
   } /* for slice */

   if (progress) (void) mexPrintf(" done\n");
   
   /* 
    * If user requested frames
    */
#if 0
   if (frames_specified){

      /*
       * Allocate space for time calculations
       */
      
      frame_time = (double *)mxCalloc(MincFile->Frames, sizeof(double));
      frame_length = (double *)mxCalloc(MincFile->Frames, sizeof(double));
   
      /* 
       * Get time domain 
       */

      count[0] = MincFile->Frames;
      coord[0] = 0;
      
      (void) mivarget(MincFile->CdfId,
                      MincFile->TimeId,
                      coord,
                      count,
                      NC_DOUBLE,
                      MI_SIGNED,
                      frame_time);

      (void) mivarget(MincFile->CdfId,
                      MincFile->TimeWidthId,
                      coord,
                      count,
                      NC_DOUBLE,
                      MI_SIGNED,
                      frame_length);

      /*
       * Calculate midframe time using frame_time and frame_length (seconds)
       */
/*      
      for (frame = 0; frame < MincFile->Frames; frame++){
         frame_time[frame] = (frame_time[frame] + frame_length[frame]/2);
         if (debug) {
            (void) mexPrintf(" Frame: %2d, Time: %.2f, length: %.2f (sec)\n",
                             frame+1,
                             frame_time[frame],
                             frame_length[frame]);
         }
         
      }
*/
      /*
       * Select those time points associated with the selected frames.
       */

      for (frame = 0; frame < frame_count; frame++){
         Time[frame] = frame_time[frames[frame]-1];
         TimeWidth[frame] = frame_length[frames[frame]-1];
         if (debug) {
            (void) mexPrintf(" User frame: %2d, Study frame: %2d, ",
                             frame+1,
                             (long)frames[frame]);
            (void) mexPrintf("frame start time:  %.2f, frame length: %.2f\n",
                             Time[frame],
                             TimeWidth[frame]); 
         }		/* if debug */
      }		/* for frame */
   }		/* if frames_specified */
#endif
   
   return(NORMAL_STATUS);
   
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : 
@INPUT      : 
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: 
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 
@MODIFIED   : 
---------------------------------------------------------------------------- */
int close_minc_file(MincInfo *MincFile)
{

   /* 
    * Free the image icv 
    */

   (void) miicv_free(MincFile->Icv);
   
   /* 
    * Close the file and return
    */

   (void) ncclose(MincFile->CdfId);
   
}

void usage(char * program_name)
{
   (void) printf("usage: [i,t] = %s('filename', options [, slices [, frames]])\n",
                 program_name);
}

#if 0
void TestFunction (Matrix *Options, Boolean *debug)
{
   Matrix  *Tmp;
   Tmp = Options;
   if (mxGetM (Options) == 1) return;
}
#endif 

void mexFunction(int    nlhs,
                 Matrix *plhs[],
                 int    nrhs,
                 Matrix *prhs[]
                 )
{

   MincInfo
      *MincFile;

   int
      m, n,
      slice,
      frame,
      slice_count = 1,
      frame_count = 1,
      *slices,
      *frames,
      image_out_count = 1;

   double
      *VectorImages,
      *Time,
      *TimeWidth,
      *double_slices,
      *double_frames;

   Boolean
      progress = FALSE,
      debug = FALSE,
      dump_only = FALSE,
      slices_specified = FALSE,
      frames_specified = FALSE;

   char
      *minc_filename;

   /* 
    * Check for proper number of arguments 
    */

	if (nrhs < MIN_INPUT_ARGUMENTS || nrhs > MAX_INPUT_ARGUMENTS) {
      usage("MINCREAD");
		(void) mexErrMsgTxt("\nMINCREAD: supply at least two input arguments.");  
	}
   
   /*
    * Check for valid filename, convert to C string.
    */

   if (!mxIsString(MINC_FILENAME)){
      usage("MINCREAD");
      (void) mexErrMsgTxt("\nMINCREAD: first argument must be minc filename.");
   }

   n = mxGetN(MINC_FILENAME)+1;
   minc_filename = mxCalloc(n,sizeof(char));
   mxGetString(MINC_FILENAME,minc_filename,n);


   /*
    * Check for user selected options
    */
   m = max(mxGetM(OPTIONS),mxGetN(OPTIONS));
	n = min(mxGetM(OPTIONS),mxGetN(OPTIONS));
   
	if (!mxIsNumeric(OPTIONS) || mxIsComplex(OPTIONS) || 
       !mxIsFull(OPTIONS)  || !mxIsDouble(OPTIONS) ||
       (n != 1 && n != 0)) {
		(void) mexErrMsgTxt("\nMINCREAD: <options> must be a vector.");
      }
   
   /*
    * If options are specified, parse them
    */

   if (m != 0){

      int option;
      double *options;

      if (m > MAX_OPTIONS) {
         (void) mexErrMsgTxt("\nMINCREAD: <options> vector to large.");
      }

      options = mxGetPr(OPTIONS);

      for (option = 0; option < m; option++) {
         switch(option){
         case 0:
            if ((int)options[option] == 1) progress = TRUE;
            break;
         case 1:
            if ((int)options[option] == 1) debug = TRUE;
            break;
         case 2:
            if ((int)options[option] == 1){
               dump_only = TRUE;
               debug = TRUE;
            }
            break;
         }
      }
   }

   if (debug && !dump_only) (void) mexPrintf("MINC filename: %s\n", minc_filename);

   /* 
    * Check to see if slice and frame specifications are legal 
    * if so, dereference pointers.
    */

   if (nrhs >= SLICES_POSITION && !dump_only) {
      
      slices_specified = TRUE;
      
      m = mxGetM(SLICES);
      n = mxGetN(SLICES);
      
      if (!mxIsNumeric(SLICES) || mxIsComplex(SLICES) || 
          !mxIsFull(SLICES)  || !mxIsDouble(SLICES) ||
          (min(m,n) != 1)) {
         (void) mexErrMsgTxt("\nMINCREAD: <slices> must be a vector.");
      }
      
      slice_count = max(m,n);
      double_slices = mxGetPr(SLICES);
      slices = (int *)mxCalloc(slice_count,sizeof(int));
      for (slice = 0; slice < slice_count; slice++)
         slices[slice] = (int)double_slices[slice];
      
      if (debug){
         (void) mexPrintf("User selections:\n");
         (void) mexPrintf(" Slice count: %d\n", slice_count);
         (void) mexPrintf(" %s", slice_count > 1 ? "Slices:" : "Slice:");
         for (slice = 0; slice < slice_count; slice++){
            (void) mexPrintf(" %d", slices[slice]);
         }
         (void) mexPrintf("\n");
      }

   }else{

      slices = (int *)mxCalloc(1,sizeof(int));
      slices[0] = 1;

   }
   
   if (nrhs == FRAMES_POSITION && !dump_only) {

      frames_specified = TRUE;

      m = mxGetM(FRAMES);
      n = mxGetN(FRAMES);

      if (!mxIsNumeric(FRAMES) || mxIsComplex(FRAMES) || 
          !mxIsFull(FRAMES)  || !mxIsDouble(FRAMES) ||
          (min(m,n) != 1)) {
         (void) mexErrMsgTxt("\nMINCREAD: <frames> must be a vector.");
      }

      frame_count = max(m,n);
      double_frames = mxGetPr(FRAMES);
      frames = (int *)mxCalloc(frame_count,sizeof(int));
      for (frame = 0; frame < frame_count; frame++)
         frames[frame] = (int)double_frames[frame];

#if 0
      TIME_VECTOR = mxCreateFull(frame_count,1,REAL);
      Time = mxGetPr(TIME_VECTOR);

      TIME_WIDTH = mxCreateFull (frame_count,1,REAL);
      TimeWidth = mxGetPr(TIME_WIDTH);
#endif 
      
      if (debug){
         (void) mexPrintf(" Frame count: %d\n", frame_count);
         (void) mexPrintf(" %s", frame_count > 1 ? "Frames:" : "Frame:");
         for (frame = 0; frame < frame_count; frame++){
            (void) mexPrintf(" %d", frames[frame]);
         }
         (void) mexPrintf("\n");
      }
            
   }else{
      
      frames = (int *)mxCalloc(1,sizeof(int));
      frames[0] = 1;
      
   }

   /*
    * Allow only one of the z or t dimensions to be multiple
    */
   
   if (slice_count != 1 && frame_count != 1){
      (void) mexErrMsgTxt("\nMINCREAD: <slices> and <frames> cannot both be vectors");
   }
   
   if (!dump_only) image_out_count = slice_count * frame_count;
   
   if (debug && !dump_only) 
      (void) mexPrintf(" Total Images requested: %d\n", image_out_count);

   if ((MincFile = open_minc_file(minc_filename, progress, debug)) == NULL){
      (void) mexErrMsgTxt("\nMINCREAD: exiting");
   }

   /*
    * Check to see that file contains requested dimensions
    */

   if (slices_specified){

      if (MincFile->Slices == 0){
         (void) mexErrMsgTxt("\nMINCREAD: File has no z dimension ... exiting");
      }

      if (slice_count > MincFile->Slices){
         (void) mexPrintf("\nRequested %d slices, file has only %d\n", 
                          slice_count,
                          MincFile->Slices);
         (void) mexErrMsgTxt("\nMINCREAD: exiting");
      }

      if (slices[slice_count - 1] > MincFile->Slices){
         (void) mexPrintf("\nLast slice requested is %d, file has only %d\n", 
                          slices[slice_count - 1],
                          MincFile->Slices);
         (void) mexErrMsgTxt("\nMINCREAD: exiting");
      }

   }

   if (frames_specified){

      if (MincFile->Frames == 0){
         (void) mexErrMsgTxt("\nMINCREAD: File has no time dimension ... exiting");
      }

      if (frame_count > MincFile->Frames){
         (void) mexPrintf("\nRequested %d frames, file has only %d\n", 
                          frame_count,
                          MincFile->Frames);
         (void) mexErrMsgTxt("\nMINCREAD: exiting");
      }

      if (frames[frame_count - 1] > MincFile->Frames){
         (void) mexPrintf("\nLast frame requested is %d, file has only %d\n", 
                          frames[frame_count - 1],
                          MincFile->Frames);
         (void) mexErrMsgTxt("\nMINCREAD: exiting");
      }

   }

   /*
    * Everything seems Ok, ... (famous last words) here we go ...
    */

   if (debug) 
      (void) mexPrintf("Individual image size: %d\n", MincFile->ImageSize);
   
   if (!dump_only) {

      VECTOR_IMAGES = mxCreateFull(MincFile->ImageSize, image_out_count, REAL);
      VectorImages = mxGetPr(VECTOR_IMAGES);      
      
      (void) read_minc_file(MincFile,
                            slices,
                            slice_count,
                            frames,
                            frame_count,
                            image_out_count,                  
                            frames_specified,
                    /*      Time,
                            TimeWidth,       */
                            VectorImages,
                            progress,
                            debug);

   }

   close_minc_file(MincFile);

}
