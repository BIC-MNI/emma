/* ----------------------------- MNI Header -----------------------------------
@NAME       : miinquire.c  (CMEX)
@INPUT      : MINC_FILE 
@OUTPUT     : NUM_DIMS, NUM_VARS, NUM_GATTS
@RETURNS    : 
@DESCRIPTION: CMEX routine to mirror the functionality of mincinfo.  Currently
              only provides general image info and dimension length.
@METHOD     : 
@GLOBALS    : 
@CALLS      : standard mex, NetCDF functions
@CREATED    : 93-6-8, Greg Ward
@MODIFIED   : 93-7-26 to 93-7-29, greatly expanded to allow for general 
              options, and added code for 'dimlength' option.
---------------------------------------------------------------------------- */


/* general: miinquire (<filename>, 'option' [, 'item'])
   specific:  len = miinquire (<filename>, 'dimlength', 'time'
              names = miinquire (<filename>, 'dimnames')
     etc.
*/
   

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mex.h"
#include "minc.h"
#include "gpw.h"
#include "mierrors.h"
#include "mincutil.h"
#include "mexutils.h"

#define PROGNAME "miinquire"

/* General input arguments */

#define MINC_FILE prhs [0]       /* input arguments */
#define OPTION    prhs [1]       /* string like dimlength, dimnames, etc. */
#define ITEM      prhs [2]       /* dimension or variable name (not always used) */
#define ATTNAME   prhs [3]       /* attribute name (only with the att* options) */

 /* Output arguments for 'dimlength' option */
#define DIMLENGTH plhs [0]

/* Output arguments for 'dimnames' option */
#define DIMNAMES  plhs [0]

/* Output arguments for 'imagesize' option */
#define IMAGESIZE plhs [0]


#define NUM_DIMS  plhs [0]       /* output arguments */
#define NUM_VARS  plhs [1]
#define NUM_GATTS plhs [2]

/* Global variables */

Boolean  debug;
char    *ErrMsg;



/* ----------------------------- MNI Header -----------------------------------
@NAME       : ErrAbort
@INPUT      : msg - character to string to print just before aborting
@OUTPUT     : none - function does not return!!!
@RETURNS    : 
@DESCRIPTION: Prints a usage summary, and calls mexErrMsgTxt with the 
              supplied msg, which ABORTS the mex-file!!!
@METHOD     : 
@GLOBALS    : requires PROGNAME macro
@CALLS      : standard mex functions
@CREATED    : 93-5-27, Greg Ward
@MODIFIED   : 93-6-16, added PrintUsage and ExitCode parameters to harmonize
              with mireadimages, etc.
@COMMENTS   : Copied to miinquire from mireadvar.
---------------------------------------------------------------------------- */
void ErrAbort (char msg[], Boolean PrintUsage, int ExitCode)
{
   if (PrintUsage)
   {
      printf ("Usage: %s (<filename> [, option [, item]]\n\n", PROGNAME);
      printf ("where option is a character string from the list \n");
      printf ("     dimnames - return list of dimension names\n");
      printf ("     dimlengths - return size of a dimension\n");
   }
   (void) mexErrMsgTxt (msg);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : GeneralInfo
@INPUT      : CDF - handle to open NetCDF file
@OUTPUT     : NumDims, NumVars, NumGAtts - 1x1 Matrix objects containing
              (respectively) the number of dimensions, variables, and 
              global attributes.  These are meant to be returned as the
              elements of plhs[] by mexFunction.
@RETURNS    : ERR_NONE if no errors detected.
              ERR_IN_MINC if there was an error calling ncinquire; global
              variable ErrMsg is set to an appropriately descriptive
              string in this case.
@DESCRIPTION: Get the numbers of dimensions, variables, and global 
              attributes from a NetCDF file.  These are returned as 
              Matrix objects to be returned to MATLAB.
@METHOD     : 
@GLOBALS    : ErrMsg
@CALLS      : standard NetCDF, mex functions
@CREATED    : 93-7-26, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
int GeneralInfo (int CDF, Matrix **NumDims, Matrix **NumGAtts, Matrix **NumVars)
{
   int  nDims, nVars, nGAtts;

   /* Read in # of dimensions, variables, and global attributes */

   if (ncinquire (CDF, &nDims, &nVars, &nGAtts, NULL) == MI_ERROR)
   {
      sprintf (ErrMsg, "Error reading from NetCDF file: %s", NCErrMsg (ncerr));
      return (ERR_IN_MINC);
   }

   /* Create the MATLAB Matrices for returning to caller */

   *NumDims = mxCreateFull (1, 1, REAL);
   *NumGAtts = mxCreateFull (1, 1, REAL);
   *NumVars = mxCreateFull (1, 1, REAL);

   /* Copy the ncinquire() results into MATLAB Matrices */

   *(mxGetPr (*NumDims)) = (double) nDims;
   *(mxGetPr (*NumGAtts)) = (double) nGAtts;
   *(mxGetPr (*NumVars)) = (double) nVars;

   return (ERR_NONE);
}       /* GeneralInfo */


/* ----------------------------- MNI Header -----------------------------------
@NAME       : GetDimLength
@INPUT      : CDF - handle to open NetCDF file
              mDimName - name of dimension, as a MATLAB character matrix
@OUTPUT     : mDimLength - length of dimension, as a 1x1 MATLAB matrix
                           (or empty Matrix if dimension not found)
@RETURNS    : ERR_NONE if all went well, EVEN if the dimension was not found
              -- in this case, the returned mDimLength will be empty
              ERR_ARGS if mDimName is bad
@DESCRIPTION: Get the length of a NetCDF dimension, and return it as a
              MATLAB Matrix.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 
@MODIFIED   : 
---------------------------------------------------------------------------- */
int GetDimLength (int CDF, Matrix *mDimName, Matrix **mDimLength)
{
   char    *DimName;
   int      DimID;
   long     DimLength;
   

   if (debug) printf ("Getting length of dimension ");
   if (ParseStringArg (mDimName, &DimName) == NULL)
   {
      sprintf (ErrMsg, "Item (dimension name) must be a character string");
      return (ERR_ARGS);
   }
   if (debug) printf ("%s\n", DimName);

   /* Get the dimension ID and length */

   DimID = ncdimid (CDF, DimName);
   if (DimID == MI_ERROR)               /* not found? then return an */
   {                                    /* empty Matrix */
      *mDimLength = mxCreateFull (0, 0, REAL);
   }
   else                                 /* dimension was there, so get */
   {                                    /* the length and return it */
      ncdiminq (CDF, DimID, NULL, &DimLength);
      *mDimLength = mxCreateFull (1, 1, REAL);
      *(mxGetPr(*mDimLength)) = (double) DimLength;
   }
   return (ERR_NONE);
}     /* GetDimLength */



int GetImageSize (int CDF, Matrix **Sizes)
{
   ImageInfoRec	Image;
   int		Result;
   double	*dSizes;                /* pointer to real part of *Sizes */


   Result = GetImageInfo (CDF, &Image);

   if (Result != ERR_NONE)
   {
      return (Result);
   }

   /* Count up the number of dimensions that GetImageInfo found in the image */

/*
   if (Image.FrameDim != -1) NumDims++;
   if (Image.SliceDim != -1) NumDims++;
   if (Image.HeightDim!= -1) NumDims++;
   if (Image.WidthDim != -1) NumDims++;
*/
   /* Create the MATLAB Matrix (really a vector) to hold the image sizes */

   *Sizes = mxCreateFull (4, 1, REAL);
   dSizes = mxGetPr (*Sizes);
   dSizes [0] = Image.Frames;
   dSizes [1] = Image.Slices;
   dSizes [2] = Image.Height;
   dSizes [3] = Image.Width;

   return (ERR_NONE);
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
void mexFunction (int nlhs, Matrix *plhs [],       /* output args */
                  int nrhs, Matrix *prhs [])       /* input args */
{
   char     *Filename;
   char     *Option;
   int      CDF;
   int      Result;

   ncopts = 0;
   ErrMsg = (char *) mxCalloc (256, sizeof(char));
   debug = FALSE;
   if (nrhs == 0) ErrAbort ("Not enough arguments", TRUE, ERR_ARGS);

   /* Parse filename and open MINC file */

   if (ParseStringArg (MINC_FILE, &Filename) == NULL)
   {
      ErrAbort ("Filename argument must be a character string", TRUE, ERR_ARGS);
   }
   if (debug) printf ("Filename: %s\n", Filename);

   OpenFile (Filename, &CDF, NC_NOWRITE);
   if (CDF == MI_ERROR)
   {
      ErrAbort (ErrMsg, TRUE, ERR_IN_MINC );
   }
   if (debug) printf ("CDF ID for file: %d\n", CDF);

   /* If only one input argument (filename) given, return general info */

   if (nrhs == 1) 
   {
      if (debug) printf ("Getting general info for MINC file\n");
      Result = GeneralInfo (CDF, &NUM_DIMS, &NUM_GATTS, &NUM_VARS);
      if (Result < 0)
      {
	 ncclose (CDF);
         ErrAbort (ErrMsg, TRUE, Result);
      }
      return;
   }


   /* More than one input argument, so process the second one (OPTION) */

   if (debug) printf ("More than one input arg - processing second...");
   if (ParseStringArg (OPTION, &Option) == NULL)
   {
      ncclose (CDF);
      ErrAbort ("Option argument must be a string", TRUE, ERR_ARGS);
   }
   if (debug) printf ("it's %s\n", Option);


   /* Now take action based on value of string Option */

   if (strcasecmp (Option, "dimlength") == 0)
   {
      if (nrhs < 3)
      { 
         ErrAbort ("Must supply a dimension name for option dimlength", 
                   TRUE, ERR_ARGS);
      }
      Result = GetDimLength (CDF, ITEM, &DIMLENGTH);
   } 
   else if (strcasecmp (Option, "imagesize") == 0)
   {
      if (nrhs > 2)
      {
	 ErrAbort ("Cannot supply any other options or items when imagesize is requested", TRUE, ERR_ARGS);
      }
      Result = GetImageSize (CDF, &IMAGESIZE);
   }
   else if ((strcasecmp (Option, "dimnames") == 0)
            ||(strcasecmp (Option, "varnames") == 0)
            ||(strcasecmp (Option, "vartype") == 0)
            ||(strcasecmp (Option, "vardims") == 0)
            ||(strcasecmp (Option, "varatts") == 0)
            ||(strcasecmp (Option, "varvalues") == 0)
            ||(strcasecmp (Option, "atttype") == 0)
            ||(strcasecmp (Option, "attvalue") == 0))
   { 
      printf ("Sorry, option %s not yet supported.\n", Option);
   }
   else
   {
      sprintf (ErrMsg, "Unknown option: %s", Option);
      ErrAbort (ErrMsg, TRUE, ERR_ARGS);
   }

   ncclose (CDF);

   /* If ANY of the option-based calls above resulted in an error, BOMB! */
   if (Result != ERR_NONE)
   {
      ErrAbort (ErrMsg, TRUE, Result);
   }

}
