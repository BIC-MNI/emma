#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>		/* for limits on integer types */
#include <float.h>		/* for limits on floating point types */
#include "ParseArgv.h"
#include "minc.h"
#include "mincutil.h"           /* for NCErrMsg () */

#define PROGNAME      "micreateimage"

#define NUM_SIZES 4             /* number of elements in Sizes array */
#define NUM_VALID 2             /* number of elements in ValidRange array */

#define DEBUG



/* MI_SIGN_STR is used for passing signed/unsigned info to the MINC 
 * library; SIGN_STR is for passing it to the user.  (Because MI_* 
 * tacks on those ugly underscores.)
 */

#define MI_SIGN_STR(sgn) ((sgn) ? (MI_SIGNED) : (MI_UNSIGNED))
#define SIGN_STR(sgn) ((sgn) ? ("signed") : ("unsigned"))

typedef enum { false, true } Boolean;


/* Global variables */

char    *ErrMsg;

/* These are needed for ParseArgv to work */

int     Sizes [NUM_SIZES] = {-1,-1,-1,-1};
char   *TypeStr = "byte";
double  ValidRange [NUM_VALID];
char   *Orientation = "transverse";

/* Type strings (from ~neelin/src/file/minc/progs/mincinfo/mincinfo.c) */

char *type_names[] = 
   { NULL, "byte", "char", "short", "long", "float", "double" };


/* Function prototypes */

Boolean GetArgs (int *pargc, char *argv[], char **MincFile,
		 long *NumFrames, long *NumSlices, long *Height, long *Width,
		 nc_type *Type, Boolean *Signed);
void usage (void);
void ErrAbort (char *msg, Boolean PrintUsage, int ExitCode);
Boolean SetTypeAndVR (char *TypeStr, nc_type *TypeEnum, Boolean *Signed, 
		      double ValidRange[]);
void GetImageSize (char num_frames[], long *frames,
                   char num_slices[], long *slices,
                   char im_height[],  long *height,
                   char im_width[],   long *width);
void CreateDims (int CDF, long Frames, long Slices, long Height, long Width,
                 char *DimOrder, int *NumDims, int DimIDs[]);



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
   fprintf (stderr, "options may come in any order; %s -help for descriptions\n", PROGNAME);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : ErrAbort
@INPUT      : msg - a nice, descriptive error message to print before bombing
              PrintUsage - flag whether to print a syntax summary
	      ExitCode - integer to return to caller [via exit()]
@OUTPUT     : (N/A)  [does NOT return!]
@RETURNS    : (void) [does NOT return!]
@DESCRIPTION: Print out a usage summary, error message, and die.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 
@MODIFIED   : 
---------------------------------------------------------------------------- */
void ErrAbort (char *msg, Boolean PrintUsage, int ExitCode)
{
   if (PrintUsage) usage ();
   fprintf (stderr, "%s\n\n", msg);
   exit (ExitCode);
}


/*
 * GetArgs and SetTypeAndVR - two functions for parsing and making sense
 * out of the command line. 
 */



/* ----------------------------- MNI Header -----------------------------------
@NAME       : GetArgs
@INPUT      : argc - pointer to the argc passed to main()
              argv - just what is passed to main()
@OUTPUT     : argc - decremented for every argument that ParseArgv handles
              NumFrames, NumSlices, Height, Width - image size parameters
	         will be parsed from the -size argument
	      MincFile - whatever is left on the command line after calling
	         ParseArgv.
	      Type, ValidRange, Orientation - other parameters; each one
	         has its own command-line argument and will be parsed
		 out by ParseArgv.
@RETURNS    : true on success
              on failure (eg. if ParseArgv returns false), calls ErrAbort - 
  	         does NOT return!
@DESCRIPTION: Use ParseArgv to parse the command-line arguments.  The table
              that drives ParseArgv lives here, so this is what needs to
	      be changed to add more options.  Also, intelligent defaults
	      should be set by whoever calls GetArgs if the argument is
	      truly optional (this is how Type, ValidRange, and Orientation
	      work); GetArgs should make sure that the value(s) set by
	      ParseArgv are NOT the same as the defaults if an option
	      is required to be set on the command line.
@METHOD     : 
@GLOBALS    : 
@CALLS      : ParseArgv, ErrAbort (on error)
@CREATED    : 93-10-16, Greg Ward
@MODIFIED   : 
@COMMENTS   : Currently no support for explicitly setting signed or
              unsigned types.
---------------------------------------------------------------------------- */
Boolean GetArgs (int *pargc, char *argv[], char **MincFile,
		 long *NumFrames, long *NumSlices, long *Height, long *Width,
		 nc_type *Type, Boolean *Signed)
{

   /*
    * Define the valid command line arguments (-size, -type, -valid_range,
    * -orientation, and -help); what type of arguments should follow them;
    * and where to put those arguments when found.
    */
   
   ArgvInfo ArgTable [] = 
   {
      {"-size", ARGV_INT, (char *) NUM_SIZES, (char *) Sizes, 
       "lengths of the four image dimensions"},
      {"-type", ARGV_STRING, NULL, (char *) &TypeStr,
       "type of the image variable: byte, short, long, float, or double"},
      {"-valid_range", ARGV_FLOAT, (char *) NUM_VALID, (char *) ValidRange,
       "valid range of image data to be stored in the MINC file"},
      {"-orientation", ARGV_STRING, NULL, (char *) &Orientation,
       "orientation of the image dimensions: transverse, coronal, or sagittal"},
      {"-help", ARGV_HELP, NULL, NULL, NULL},
      {NULL, ARGV_END, NULL, NULL, NULL}
   };

#ifdef DEBUG
   printf ("Default values:\n");
   printf ("%ld frames, %ld slices, height %ld, width %ld\n",
	   Sizes [0], Sizes [1], Sizes [2], Sizes [3]);
   printf ("valid range min = %lg, max = %lg\n", 
	   ValidRange [0], ValidRange [1]);
   printf ("Image type = %s %s, Orientation = %s\n\n",
	   SIGN_STR (*Signed), TypeStr, Orientation);
#endif


   /* Parse those command line arguments!  If any errors, die right now. */

   if (ParseArgv (pargc, argv, ArgTable, 0))
   {
      ErrAbort ("", true, 1);
   }

   /* Break-down the elements of the Sizes[] array. */
   
   *NumFrames = (long) Sizes [0];
   *NumSlices = (long) Sizes [1];
   *Height = (long) Sizes [2];
   *Width = (long) Sizes [3];

   if (!SetTypeAndVR (TypeStr, Type, Signed, ValidRange))
   {
      ErrAbort (ErrMsg, true, 1);
   }

#ifdef DEBUG
   printf ("\nValues after ParseArgv and SetTypeAndVR:\n");
   printf ("%ld frames, %ld slices, height %ld, width %ld\n",
	   Sizes [0], Sizes [1], Sizes [2], Sizes [3]);
   printf ("valid range min = %lg, max = %lg\n", 
	   ValidRange [0], ValidRange [1]);
   printf ("Image type = %s %s, Orientation = %s\n",
	   SIGN_STR (*Signed), TypeStr, Orientation);
#endif
   
   if ((*pargc < 2) || (Sizes[0] == -1))
   {
      ErrAbort ("Require at least a MINC file name and image size", true, 1);
   }
   else
   {
      *MincFile = argv [1];
   }
}     /* GetArgs () */


/* ----------------------------- MNI Header -----------------------------------
@NAME       : SetTypeAndVR
@INPUT      : TypeStr - the desired image type as a character string, must
                 be one of "byte", "short", "long", "float", "double".
	      ValidRange - the (possibly not-yet-set) valid range.
@OUTPUT     : TypeEnum - the data type as one of the nc_type enumeration,
                 i.e. NC_BYTE, NC_SHORT, etc.
	      Signed - whether or not the type is signed (this is currently
	         hard-coded to set bytes unsigned, all others signed)
	      ValidRange - the (possibly unmodified) valid range.
@RETURNS    : true on success
              false if TypeStr is invalid
	      false if ValidRange is invalid for the given type
	      (all error conditions set the global variable ErrMsg)
@DESCRIPTION: Converts the character string TypeStr (from the command-line)
              to an nc_type.  If ValidRange is {0, 0} (ie. not set on the
	      command line), then it is set to the default valid range for
	      the given type, namely the maximum range of the type.  If
	      ValidRange was already set, then it is checked to make
	      sure it's within the maximum range of the type.
@METHOD     : 
@GLOBALS    : ErrMsg
@CALLS      : 
@CREATED    : 93-10-16, Greg Ward
@MODIFIED   :
@COMMENTS   : Currently no support for explicitly setting signed or
              unsigned types; byte => unsigned, all others => signed.
---------------------------------------------------------------------------- */
Boolean SetTypeAndVR (char *TypeStr, nc_type *TypeEnum, Boolean *Signed,
		      double ValidRange[])
{
   double  DefaultMin;		/* maximum range of the type specified by */
   double  DefaultMax;		/* TypeStr (used for setting/checking) */

   /* First convert the character string type to the nc_type enumeration */

   if (strcmp (TypeStr, "byte") == 0)
   {
      *TypeEnum = NC_BYTE;
      *Signed = false;
   }
   else if (strcmp (TypeStr, "short") == 0)
   {
      *TypeEnum = NC_SHORT;
      *Signed = true;
   }
   else if (strcmp (TypeStr, "long") == 0)
   {
      *TypeEnum = NC_LONG;
      *Signed = true;
   }
   else if (strcmp (TypeStr, "float") == 0)
   {
      *TypeEnum = NC_FLOAT;
      *Signed = true;
   }
   else if (strcmp (TypeStr, "double") == 0)
   {
      *TypeEnum = NC_DOUBLE;
      *Signed = true;
   }
   else if (strcmp (TypeStr, "char") == 0)
   {
      sprintf (ErrMsg, "Unsupported NetCDF type: char");
      return (false);
   }
   else
   {
      sprintf (ErrMsg, "Unknown data type: %s", TypeStr);
      return (false);
   }

#ifdef DEBUG
   printf ("Supplied type was %s, this maps to nc_type as %s, and it's %s\n",
	   TypeStr, type_names [*TypeEnum], SIGN_STR(*Signed));
#endif


   /* Now find the maximum range of the desired type; this will be 
    * used to either set the valid range (if none was set on the command
    * line) or to ensure that the given valid range is in fact valid.
    */

   switch (*TypeEnum)
   {
      case NC_BYTE:
      {
	 DefaultMin = (double) CHAR_MIN;
	 DefaultMax = (double) CHAR_MAX;
	 break;
      }
      case NC_SHORT:
      {
	 DefaultMin = (double) SHRT_MIN;
	 DefaultMax = (double) SHRT_MAX;
	 break;
      }
      case NC_LONG:
      {
	 DefaultMin = (double) LONG_MIN;
	 DefaultMax = (double) LONG_MAX;
	 break;
      }
      case NC_FLOAT:
      {
	 DefaultMin = (double) -FLT_MAX;
	 DefaultMax = (double) FLT_MAX;
	 break;
      }
      case NC_DOUBLE:
      {
	 DefaultMin = -DBL_MAX;
	 DefaultMax = DBL_MAX;
	 break;
      }
   }

#ifdef DEBUG
   printf ("Maximum range (= default valid range) for this type: [%lg %lg]\n",
	   DefaultMin, DefaultMax);
#endif


   /* If both the min and max of ValidRange are zero, then it was not
    * set on the command line by the user -- so set it to the default
    * range for the given type.  Otherwise, make sure that the given
    * range is legal.
    */

   if ((ValidRange[0] == 0) && (ValidRange[1] == 0))
   {
      ValidRange [0] = DefaultMin;
      ValidRange [1] = DefaultMax;

#ifdef DEBUG
      printf ("Valid range was all zero already, so set it to [%lg %lg]\n",
	      ValidRange[0], ValidRange[1]);
#endif

   }     /* if ValidRange not set on command line */
   else
   {
#ifdef DEBUG
      printf ("Valid range already set, making sure it's in bounds\n");
#endif

      if ((ValidRange [0] < DefaultMin) ||
	  (ValidRange [1] > DefaultMax))
      {
	 sprintf (ErrMsg, "Invalid range (%lg .. %lg) given for type %s",
		  ValidRange[0], ValidRange[1], TypeStr);
	 return (false);
      }
   }

   return (true);

}     /* SetTypeAndVR() */




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
@RETURNS    : true on success
              false if an invalid orientation was given
	      false if any errors occured while creating the dimensions
	      (ErrMsg is set on error)
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

              (Note that only the first character of DimOrder is looked at.)
@METHOD     : 
@GLOBALS    : ErrMsg
@CALLS      : NetCDF library
@CREATED    : 1993/8/16, Greg Ward
@MODIFIED   : 1993/10/26: Civilised the error handling (GPW)
---------------------------------------------------------------------------- */
Boolean CreateDims (int CDF, long Frames, long Slices, long Height, long Width,
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
            sprintf (ErrMsg, "Unknown orientation %s "
		     "(must be one of transverse, coronal, or sagittal\n",
		     DimOrder);
	    return (false);
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
            sprintf (ErrMsg, "Error creating dimensions (%s)\n",
                     NCErrMsg (ncerr));
	    return (false);
        }
    }

#ifdef DEBUG
    printf ("Done creating %d dimensions\n", CurDim);
#endif
    
}      /* CreateDims () */    




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
@CALLS      : GetArgs
              CreateDims
              MINC library
              NetCDF library
@CREATED    : June 3, 1993 by MW
@MODIFIED   : 
---------------------------------------------------------------------------- */

int main (int argc, char *argv[])
{
   nc_type NCType;
   Boolean Signed;


   long    NumFrames;		/* lengths of the various image dimensions */
   long    NumSlices;
   long    Height;
   long    Width;
/* double  vrange[2];	    */  /* valid range of image data */

   char   *MincFile;		/* name of MINC file from command line */
   int     file_CDF;
   int     Dim[4];		/* dimension ID's for the image */
   int     NumDim;		/* 2, 3, or 4 based on whether frames */ 
				/* slices, or both are zero */

   int     image_id, max_id, min_id;
   int     time_id, time_width_id;


   ErrMsg = (char *) calloc (256, sizeof (char));

   GetArgs (&argc, argv, &MincFile, &NumFrames, &NumSlices, &Height, &Width, 
	    &NCType, &Signed);


   printf ("File is: %s\n", MincFile);
    
   ncopts = 0;
   
   /* Open the NetCDF file, bomb if error */
   
   file_CDF = ncopen (MincFile, NC_WRITE);
   if (file_CDF == MI_ERROR)
   {
      sprintf (ErrMsg, "Error opening MINC file %s: %s\n",
	       MincFile, NCErrMsg (ncerr));
      ErrAbort (ErrMsg, true, 1);
   }
   
   /* Bomb if the MIimage variable is already in the NetCDF file */
   
   if (ncvarid (file_CDF, MIimage) != MI_ERROR)
   {
      sprintf (ErrMsg, "Image variable already exists in file %s\n", MincFile);
      ErrAbort (ErrMsg, true, 1);
   }

   /* Put the CDF file back into definition mode, and create the 
    * image dimensions (either two, three, or four of them; how 
    * many are actually created will be put into NumDim, and the 
    * list of dimension ID's will be put into Dim[].
    */

   ncredef(file_CDF);
   CreateDims (file_CDF, NumFrames, NumSlices, Height, Width, 
	       Orientation, &NumDim, Dim);


   /*
    * Create the image-max and image-min variables.  They should be
    * dependent on the "non-image" dimensions (ie. time and slices,
    * if they exist), so pass NumDim-2 as the number of
    * dimensions, and Dim as the list of dimension ID's -- 
    * micreate_std_variable should then only look at the first one
    * or two dimension IDs in the list.
    */
   
   max_id = micreate_std_variable (file_CDF, MIimagemax, NC_DOUBLE,
				   NumDim-2, Dim);
   min_id = micreate_std_variable (file_CDF, MIimagemin, NC_DOUBLE,
				   NumDim-2, Dim);
   
   if ((max_id == MI_ERROR) || (min_id == MI_ERROR))
   {  
      fprintf (stderr, "Error creating image max/min variables: %s\n",
	       NCErrMsg (ncerr));
      exit (-1);
   }

   /* If there are to be any frames present in the file, create time
    * and time-width variables. */
   
   if (NumFrames > 0)
   {
      time_id = micreate_std_variable (file_CDF, MItime, NC_DOUBLE, 1, Dim);
      time_width_id = micreate_std_variable (file_CDF, MItime_width, NC_DOUBLE, 1, Dim);
      
      if ((time_id == MI_ERROR) || (time_width_id == MI_ERROR))
      {
	 fprintf (stderr, "Error creating time/time-width variables: %s\n",
		  NCErrMsg (ncerr));
	 exit (-1);
      }
   }
   

   /* Finally, we create the image variable, and put in the valid range,
    * signtype, and complete attributes.
    */
  
   image_id = micreate_std_variable (file_CDF, MIimage, NCType,
				     NumDim, Dim);
   (void) miattputstr (file_CDF, image_id, MIsigntype, SIGN_STR(Signed));
   (void) miattputstr (file_CDF, image_id, MIcomplete, MI_FALSE);
   
   (void) ncattput (file_CDF, image_id, MIvalid_range, NC_DOUBLE, 2, ValidRange);

   ncclose (file_CDF);
   
   return (0);
}

