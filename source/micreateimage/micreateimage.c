#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ParseArgv.h"
#include "minc.h"
#include "mincutil.h"           /* for NCErrMsg () */

#define PROGNAME      "micreateimage"

#define MINC_FILE     argv[1]
#define NUM_FRAMES    argv[2]   /* the image size parameters as strings */
#define NUM_SLICES    argv[3]   /* to be parsed by GetImageSize */
#define HEIGHT        argv[4]   
#define WIDTH         argv[5]
#define DIM_ORDER     argv[6]   /* "transverse", "coronal", or "sagittal" */

typedef int Boolean;

char    *ErrMsg;                /* just to keep mincutil happy */
Boolean  debug;

#define NUM_SIZES 4             /* number of elements in Sizes array */
#define NUM_VALID 2             /* number of elements in ValidRange array */


int     Sizes [NUM_SIZES];      /* # frames, # slices, height, width */
char   *Type;                   /* byte/short/long/float/double */
double  ValidRange [NUM_VALID]; /* low, high */
char   *Orientation;            /* transverse/coronal/sagittal */

ArgvInfo ArgTable [] = 
{
   {"-size", ARGV_INT, (char *) NUM_SIZES, (char *) Sizes, 
    "lengths of the image dimensions: <# frames> <# slices> <height> <width>"},
   {"-type", ARGV_STRING, NULL, (char *) &Type,
    "type of the image variable: one of byte, short, long, float, or double"},
   {"-valid_range", ARGV_FLOAT, (char *) NUM_VALID, (char *) ValidRange,
    "valid range of image data to be stored in the MINC file"},
   {"-orientation", ARGV_STRING, NULL, (char *) &Orientation,
    "orientation of the image dimensions: transverse, coronal, or sagittal"},
   {"-help", ARGV_HELP, NULL, NULL, NULL},
   {NULL, ARGV_END, NULL, NULL, NULL}
};


/* ----------------------------- MNI Header -----------------------------------
@NAME       : usage
@INPUT      : void
@OUTPUT     : void
@RETURNS    : void
@DESCRIPTION: Prints usage information for micreateimage
@METHOD     : none
@GLOBALS    : none
@CALLS      : none
@CREATED    : June 2, 1993 by MW
@MODIFIED   :
---------------------------------------------------------------------------- */
void usage (void) 
{
   fprintf (stderr, "\nUsage:\n");
   fprintf (stderr, "%s <MINC file> [option] [option] ...\n\n");
   fprintf (stderr, "options may come in any order; %s -help for descriptions\n", argv [0]);
}


void ErrAbort (char *msg, boolean PrintUsage, int ExitCode)
{
   if (PrintUsage) usage ();
   fprintf (stderr, "%s\n\n", msg);
   exit (ExitCode);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : GetImageSize
@INPUT      : num_frames -> A character string containing a number representing
                            the number of frames.  If NULL or contains only
                            "-", number of slices will be zero.
              num_slices -> A character string containing a number representing
                            the number of slices.  If NULL or contains only
                            "-", number of slices will be zero.
              height     -> the image height in pixels; must be supplied and
                            be greater than zero
              width      -> the image width in pixels; must be supplied and
                            be greater than zero
@OUTPUT     : frames -> An integer representation of the number of frames in
                        the image.
              slices -> An integer representation of the number of slices in
                        the image.
              height -> Height of the image in pixels (as long int).
              width  -> Width of the image in pixels (as long int).
@RETURNS    : void
@DESCRIPTION: Gets the number of frames, number of slices, image height,
              and image width from character strings (presumably from
              the command line).  If either of the character string     
              representations of the number of frames or slices is      
              NULL, contains only "-", then the corresponding integer   
              representation will be zero.  (It is then up to the caller
              to be smart enough to not create the associated MINC
              dimension).  If either of height or width is NULL or
              non-numeric or zero, that is an error and the program
              will be terminated with a brief message.

@METHOD     : none
@GLOBALS    : none
@CALLS      : none
@CREATED    : June 2, 1993 by MW
@MODIFIED   : August 16, 1993, Greg Ward: changed to parse all four image
              size parameters.
---------------------------------------------------------------------------- */
void GetImageSize (char num_frames[], long *frames,
                   char num_slices[], long *slices,
                   char im_height[],  long *height,
                   char im_width[],   long *width)
{
    char   *eos;         /* end-of-string returned by strtol */

    /* 
     * If the num_frames string is NULL or "-", then assume zero frames.
     * Otherwise use strtol to parse the number out of num_frames; if
     * strtol returns anything other than the terminating NULL character
     * as the first "unrecognized" character, then that means there was
     * non-numeric junk in the string, so it's an error.
     */      

    if ((strcmp (num_frames, "-") == 0) || (num_frames == NULL))
    {
        *frames = 0;
    }
    else
    {
        *frames = strtol (num_frames, &eos, 0);
        if (*eos != (char) 0)
        {
            fprintf (stderr, "micreateimage: number of frames must be numeric or \"-\" for no frames\n");
            exit (-1);
        }
    }

    /* Now do the exact same thing for num_slices and *slices */

    if ((strcmp (num_slices, "-") == 0) || (num_slices == NULL))
    {
        *slices = 0;
    }
    else
    {
        *slices = strtol (num_slices, &eos, 0);
        if (*eos != (char) 0)
        {
            fprintf (stderr, "micreateimage: number of slices must be numeric or \"-\" for no slices\n");
            exit (-1);
        }
    }

    /*
     * Parse the image height and width now.  Both must be numeric and
     * greater than zero; anything else is an error.
     */

    *height = strtol (im_height, &eos, 0);
    if ((*eos != (char) 0) || (*height <= 0))
    {
        fprintf (stderr, "micreateimage: image height must be numeric and greater than zero\n");
        exit (-1);
    }

    *width = strtol (im_width, &eos, 0);
    if ((*eos != (char) 0) || (*width <= 0))
    {
        fprintf (stderr, "micreateimage: image width must be numeric and greater than zero\n");
        exit (-1);
    }
    
}     /* GetImageSize */


/* ----------------------------- MNI Header -----------------------------------
@NAME       : CreateDims
@INPUT      : CDF    - handle to a CDF file open and in define mode
              Frames - number of frames (possibly zero)
              Slices - number of slices (possibly zero)
              Height - image height (second-last image dimension)
              Width  - image width (last image dimension, ie. fastest varying)
              DimOrder - character string starting with either 't' 
                         (transverse), 'c' (coronal), or 's' (sagittal)
                         which determines how slices/height/width map 
                         to zspace/yspace/xspace
@OUTPUT     : NumDims - the number of image dimensions created (2, 3, or 4)
              DimIDs  - list of dimension id's created, with DimIDs[0] being
                        the slowest varying dimension (MItime if Frames>0),
                        and DimIDs[NumDims-1] the fastest varying (MIxspace
                        in the case of transverse images).
@RETURNS    : (void)
@DESCRIPTION: Create up to four image dimensions in an open MINC file.
              At least two dimensions, the "width" and "height" of the 
              image, will always be created.  Note that width and height
              here don't necessarily correspond to width and height of 
              images when we view them on the screen -- width is simply
              the fastest varying image dimension, and height is the
              second fastest.  A slice dimension will be created if
              Slices > 0, and the MItime dimension will be created if
              Frames > 0.  DimOrder determines how slices/height/width map 
              to zspace/yspace/xspace as follows:
 
                DimOrder     Slice dim    Height dim   Width dim
                 transverse   MIzspace     MIyspace     MIxspace
                 sagittal     MIxspace     MIzspace     MIyspace
                 coronal      MIyspace     MIzspace     MIxspace
@METHOD     : 
@GLOBALS    : 
@CALLS      : NetCDF library
@CREATED    : 16 August 1993, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
void CreateDims (int CDF, long Frames, long Slices, long Height, long Width,
                 char *DimOrder, int *NumDims, int DimIDs[])
{
    int    CurDim = 0;        /* index into DimIDs */
    char  *SliceDim;
    char  *HeightDim;
    char  *WidthDim;
  
    /* Calculate how many dimensions we will be creating, either 2 3 or 4. */

    *NumDims = 4;
    if (Frames == 0)
    {
        (*NumDims)--;
    }

    if (Slices == 0)
    {
        (*NumDims)--;
    }

#ifdef DEBUG
    printf ("# frames %ld, # slices %ld, height %ld, width %ld\n",
            Frames, Slices, Height, Width);
    printf ("Will create %d dimensions\n", *NumDims);
#endif    

    /* Determine the dimension names corresponding to slices, height, width */

    switch (toupper(DimOrder [0]))
    {
        case 'T':                     /* transverse */     
        {
            SliceDim = MIzspace;
            HeightDim = MIyspace;
            WidthDim = MIxspace;
            break;
        }
        case 'S':                     /* sagittal */
        {
            SliceDim = MIxspace;
            HeightDim = MIzspace;
            WidthDim = MIyspace;
            break;
        }
        case 'C':                     /* coronal */
        {
            SliceDim = MIyspace;
            HeightDim = MIzspace;
            WidthDim = MIxspace;
            break;
        }
        default:
        {
            fprintf (stderr, "micreateimage: unknown dimension ordering (must be one of transverse, coronal, or sagittal\n");
            exit (-1);
        }
    }

#ifdef DEBUG
    printf ("Slice dimension: %s\n", SliceDim);
    printf ("Height dimension: %s\n", HeightDim);
    printf ("Width dimension: %s\n", WidthDim);
#endif


    /* If applicable, create the time dimension */

    if (Frames > 0)
    {
        DimIDs[CurDim] = ncdimdef (CDF, MItime, Frames);
        CurDim++;
    }

    /* Likewise for slice dimension */

    if (Slices > 0)
    {
        DimIDs[CurDim] = ncdimdef (CDF, SliceDim, Slices);
        CurDim++;
    }

    /* Now create the two actual image dimensions - these must be created */

    DimIDs[CurDim++] = ncdimdef (CDF, HeightDim, Height);
    DimIDs[CurDim++] = ncdimdef (CDF, WidthDim, Width);

    /* Scan through the elements of DimIDs making sure there were no errors */

    for (CurDim = 0; CurDim < *NumDims; CurDim++)
    {
        if (DimIDs [CurDim] == MI_ERROR)
        {
            fprintf (stderr, "micreateimage: error creating dimensions (%s)\n",
                     NCErrMsg (ncerr));
            exit (-1);
        }
    }

#ifdef DEBUG
    printf ("Done creating %d dimensions\n", CurDim);
#endif
    
}    




/* ----------------------------- MNI Header -----------------------------------
@NAME       : main
@INPUT      : argv[1] - name of MINC file to create image variable in
              argv[2] - number of frames (0 if no frames)
              argv[3] - number of slices (0 if no slices)
              argv[4] - image height (ie. yspace length if transverse)
              argv[5] - image width (ie. xspace length if transverse)
              argv[6] - (optional) one of transverse, coronal, or sagittal
                      - only the first letter is looked at
                      - defaults to transverse
@OUTPUT     : none
@RETURNS    : none
@DESCRIPTION: Sets up a new MINC file so that it can contain image data.
              Creates the dimensions, and the image, time, time-width,
              image-max, and image-min variables.
@METHOD     : none
@GLOBALS    : ncopts
@CALLS      : usage
              GetImageSize
              MINC library
              NetCDF library
@CREATED    : June 3, 1993 by MW
@MODIFIED   : 
---------------------------------------------------------------------------- */

int main (int argc, char *argv[])
{
#if 0
    long   frames;		/* lengths of the various image dimensions */
    long   slices;
    long   height;
    long   width;
    double vrange[2];		/* valid range of image data */
#endif

    char  *MincFile;		/* name of MINC file from command line */
    int    file_CDF;
    int    dim[4];		/* dimension ID's for the image */
    int    num_dimensions;	/* 2, 3, or 4 based on whether frames */ 
				/* slices, or both are zero */

    int    image_id, max_id, min_id;
    int    time_id, time_width_id;


    if (ParseArgv (&argc, argv, ArgTable, 0))
    {
       ErrAbort ("", true, 1);
    }

    printf ("%d frames, %d slices, height %d, width %d\n", 
            Sizes [0], Sizes [1], Sizes [2], Sizes [3]);
    printf ("vr min = %lf, vr max = %lf\n", ValidRange[0], ValidRange[1]);
    printf ("Image type: %s; image orientation: %s\n", Type, Orientation);

    if (argc < 2)
    {
       ErrAbort ("Must supply the name of a MINC file", true, 1);
    }
    else
    {
       MincFile = argv [1];
    }

    
    ncopts = 0;
/*
    GetImageSize (NUM_FRAMES, &frames, NUM_SLICES, &slices,
                  HEIGHT, &height, WIDTH, &width);
*/
    /* Open the NetCDF file, bomb if error */

    file_CDF = ncopen (MincFile, NC_WRITE);
    if (file_CDF == MI_ERROR)
    {
        fprintf (stderr, "micreateimage: error opening MINC file %s\n",
                 MINC_FILE);
        exit (-1);
    }

    /* Bomb if the MIimage variable is found in the NetCDF file */

    if (ncvarid (file_CDF, MIimage) != MI_ERROR)
    {
        fprintf (stderr, "micreateimage: image variable already exists "
                 "in file %s\n", MINC_FILE);
        exit (-1);
    }

    ncredef(file_CDF);

    if (argc < 7)              /* DIM_ORDER not supplied */
    {
        CreateDims (file_CDF, frames, slices, height, width, 
                    "transverse", &num_dimensions, dim);
    }
    else
    {
        CreateDims (file_CDF, frames, slices, height, width, 
                    DIM_ORDER, &num_dimensions, dim);
    }

    /* 
     * Create the image-max and image-min variables.  They should be
     * dependent on the "non-image" dimensions (ie. time and slices,
     * if they exist), so pass num_dimensions-2 as the number of
     * dimensions, and dim as the list of dimension ID's -- 
     * micreate_std_variable should then only look at the first one
     * or two dimension IDs in the list.
     */

    max_id = micreate_std_variable (file_CDF, MIimagemax, NC_DOUBLE,
                                    num_dimensions-2, dim);
    min_id = micreate_std_variable (file_CDF, MIimagemin, NC_DOUBLE,
                                    num_dimensions-2, dim);

    if ((max_id == MI_ERROR) || (min_id == MI_ERROR))
    {  
        fprintf (stderr, "Error creating image max/min variables: %s\n",
                 NCErrMsg (ncerr));
        exit (-1);
    }

    if (frames > 0)
    {
        time_id = micreate_std_variable (file_CDF, MItime, NC_DOUBLE, 1, dim);
        time_width_id = micreate_std_variable (file_CDF, MItime_width, NC_DOUBLE, 1, dim);

        if ((time_id == MI_ERROR) || (time_width_id == MI_ERROR))
        {
            fprintf (stderr, "Error creating time/time-width variables: %s\n",
                     NCErrMsg (ncerr));
            exit (-1);
        }
    }


    /* 
     * N.B. we're currently doing only unsigned byte images, valid range
     * 0 .. 255.  That should be made more flexible, through still MORE
     * arguments to micreateimage
     */

    image_id = micreate_std_variable (file_CDF, MIimage, NC_BYTE,
                                      num_dimensions, dim);
    (void) miattputstr (file_CDF, image_id, MIsigntype, MI_UNSIGNED);
    (void) miattputstr (file_CDF, image_id, MIcomplete, MI_FALSE);
    
    vrange[0] = 0;
    vrange[1] = 255;
    (void) ncattput (file_CDF, image_id, MIvalid_range, NC_DOUBLE, 2, vrange);

    ncclose (file_CDF);

    return (0);
}

