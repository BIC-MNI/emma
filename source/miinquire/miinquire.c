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

/* Borrowed from Peter's mincinfo */

char *type_names[] = {
   NULL, "byte", "char", "short", "long", "float", "double"
};


/* General input arguments */

#define MINC_FILE inargs [0]       /* input arguments */
#define OPTION    inargs [1]       /* string like dimlength, dimnames, etc. */
#define ITEM      inargs [2]       /* dimension or variable name (not always used) */
#define ATTNAME   inargs [3]       /* attribute name (only with the att* options) */

 /* Output arguments for 'dimlength' option */
#define DIMLENGTH outargs [0]

/* Output arguments for 'dimnames' option */
#define DIMNAMES  outargs [0]

/* Output arguments for 'imagesize' option */
#define IMAGESIZE outargs [0]


#define NUM_DIMS  outargs [0]       /* output arguments */
#define NUM_VARS  outargs [1]
#define NUM_GATTS outargs [2]

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

      printf ("Type \"help miinquire\" for the list of valid options\n\n");
   }
   (void) mexErrMsgTxt (msg);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : GeneralInfo
@INPUT      : CDF - handle to open NetCDF file
@OUTPUT     : NumDims, NumVars, NumGAtts - 1x1 Matrix objects containing
              (respectively) the number of dimensions, variables, and 
              global attributes.  These are meant to be returned as the
              elements of outargs[] by mexFunction.
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
              nargin, InArgs, nargout, OutArgs - input/output argument lists
	         and counts, exactly as passed to mexFunction
	      CurInArg - index into InArgs: points to the option string that
	         initiated this call to GetDimLength (presumably "dimlength")
		 and will be incremented to point to the element immediately
		 following the last item processed by GetDimLength (ie.,
		 either to the next option string or past the end of InArgs)
	      CurOutArg - similar index into OutArgs: points to the element
	         of OutArgs at which GetDimLength can start depositing its
		 output; will be incremented to point to the next element
		 of OutArgs after GetDimLength is done
@OUTPUT     : CurInArg, CurOutArg - incremented as described under INPUT
              OutArgs[] - various members (from OutArgs[CurOutArg]) will point
	         to newly created matrices holding the output from GetDimLength
@RETURNS    : ERR_NONE if all went well.
              ERR_ARGS (with ErrMsg set) if not input or output arguments given
@DESCRIPTION: Get the length of a NetCDF dimension, and return it as a
              MATLAB Matrix.  If the dimension does not exist, the "size"
	      will be returned as an empty matrix.  The only error condition
	      is if not enough input or output arguments are given (ie. no 
	      dimension name is supplied, or no output argument was given
	      corresponding to that dimension).
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 93/8/3 Greg Ward
@MODIFIED   : 93/8/19 Greg Ward:
---------------------------------------------------------------------------- */
int GetDimLength (int CDF, int nargin, Matrix *InArgs[], int *CurInArg,
		           int nargout, Matrix *OutArgs[], int *CurOutArg)
{
   Matrix  *mDimName;		/* the input and output as MATLAB Matrices */
   Matrix  *mDimLength; 

   char    *DimName;		/* parsed from InArgs[] */
   int      DimID;		/* returned by ncdimid */
   long     DimLength;		/* returned by ncdiminq and put into OutArgs[] */

   (*CurInArg)++;		/* point to the dimension name */

   /* 
    * *CurInArg now points to the dimension name in InArgs[]; check to make 
    * sure that we haven't overstepped the bounds defined by nargin before
    * trying to process the argument.
    */

   if (*CurInArg >= nargin)
   {
      sprintf (ErrMsg, "dimlength: not enough input arguments "
	       "(no dimension name found)");
      return (ERR_ARGS);
   }
   mDimName = InArgs [*CurInArg];

   if (ParseStringArg (mDimName, &DimName) == NULL)
   {
      sprintf (ErrMsg, "dimlength: Dimension name must be a character string");
      return (ERR_ARGS);
   }

   /* Get the dimension ID and length */

   DimID = ncdimid (CDF, DimName);
   if (DimID == MI_ERROR)               /* not found? then return an */
   {                                    /* empty Matrix */
      mDimLength = mxCreateFull (0, 0, REAL);
   }
   else                                 /* dimension was there, so get */
   {                                    /* the length and return it */
      ncdiminq (CDF, DimID, NULL, &DimLength);
      mDimLength = mxCreateFull (1, 1, REAL);
      *(mxGetPr(mDimLength)) = (double) DimLength;
   }

   /* Now make sure thet *CurOutArg points within the bounds of OutArgs[] */

   if (*CurOutArg >= nargout)
   {
      printf ("*CurOutArg too big, writing error message");
      sprintf (ErrMsg, "dimlength: not enough output arguments "
	       "(no room for length of dimension %s)", DimName);
      return (ERR_ARGS);
   }

   OutArgs [*CurOutArg] = mDimLength;   /* return dim length to caller */
   (*CurOutArg)++;		        /* point to the next input */
   (*CurInArg)++;                       /* and output arguments */

   return (ERR_NONE);

}     /* GetDimLength */



/* ----------------------------- MNI Header -----------------------------------
@NAME       : GetImageSize
@INPUT      : (see GetDimLength)
@OUTPUT     : (see GetDimLength)
@RETURNS    : ERR_NONE if all went well
              ERR_ARGS if not enough output arguments were supplied
@DESCRIPTION: Get the lengths of the four image dimensions (time, zspace, 
              yspace, xspace) and return them in the order in which they 
	      appear in the MINC file: frames, slices, height, width.  Any
	      missing dimensions will have 0 as their length.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 
@MODIFIED   : 
---------------------------------------------------------------------------- */
int GetImageSize (int CDF, int nargin, Matrix *InArgs[], int *CurInArg,
		           int nargout, Matrix *OutArgs[], int *CurOutArg)
{
   ImageInfoRec	Image;
   int		Result;
   Matrix	*mSizes;
   double	*Sizes;                /* pointer to real part of *mSizes */


   /* Get all the image dimension sizes */

   Result = GetImageInfo (CDF, &Image);

   if (Result != ERR_NONE)
   {
      return (Result);
   }

   /* Create the MATLAB Matrix (really a vector) to hold the image sizes */

   mSizes = mxCreateFull (4, 1, REAL);
   Sizes = mxGetPr (mSizes);
   Sizes [0] = Image.Frames;
   Sizes [1] = Image.Slices;
   Sizes [2] = Image.Height;
   Sizes [3] = Image.Width;

   if (*CurOutArg >= nargout)
   {
      sprintf (ErrMsg, "imagesize: not enough output arguments");
      return (ERR_ARGS);
   }

   OutArgs [*CurOutArg] = mSizes;
   (*CurOutArg)++;		        /* point to the next output */
   (*CurInArg)++;                       /* and input arguments */

   return (ERR_NONE);
}


int GetVarType (int CDF, int nargin, Matrix *InArgs[], int *CurInArg,
		         int nargout, Matrix *OutArgs[], int *CurOutArg)
{
   Matrix  *mVarName;
   Matrix  *mVarTypeStr;

   char    *VarName;
   int	    VarID;
   nc_type  VarType;
   char    *VarTypeStr;

   (*CurInArg)++;		/* point to the variable name */
   if (*CurInArg >= nargin)
   {
      sprintf (ErrMsg, "vartype: not enough input arguments "
	       "(no variable name found)");
      return (ERR_ARGS);
   }

   mVarName = InArgs [*CurInArg];
   if (ParseStringArg (mVarName, &VarName) == NULL)
   {
      sprintf (ErrMsg, "vartype: Dimension name must be a character string");
      return (ERR_ARGS);
   }

   VarID = ncvarid (CDF, VarName);
   if (VarID == MI_ERROR)
   {
      mVarTypeStr = mxCreateString ("");
   }
   else
   {
      ncvarinq(CDF, VarID, NULL, &VarType, NULL, NULL, NULL);
      VarTypeStr = type_names [VarType];
      mVarTypeStr = mxCreateString (VarTypeStr);
   }

   OutArgs[*CurOutArg] = mVarTypeStr;
   (*CurInArg)++;
   (*CurOutArg)++;
   
   return (ERR_NONE);
}

#if 0
int GetAttValue (int CDF, int nargin, Matrix *InArgs[], int *CurInArg,
		         int nargout, Matrix *OutArgs[], int *CurOutArg)
{
   Matrix  *mVarName;
   Matrix  *mAttValue;

   char    *VarName;
   int	    VarID;
   char    *AttName;
   nc_type  AttType;

   /* Point *CurInArg to the varible name, and make sure enough input args given */

   (*CurInArg)++;
   if (*CurInArg < nargin-2)
   {
      sprintf (ErrMsg, "attvalue: not enough input arguments "
	       "(need both variable and attribute name)");
      return (ERR_ARGS);
   }

   mVarName = InArgs [*CurInArg];
   if (ParseStringArg (mVarName, &VarName) == NULL)
   {
      sprintf (ErrMsg, "attvalue: Dimension name must be a character string");
      return (ERR_ARGS);
   }

   VarID = ncvarid (CDF, VarName);
   if (VarID == MI_ERROR)
   {
      return emptiness  
   }







   OutArgs[*CurOutArg] = mVarTypeStr;
   (*CurInArg)++;
   (*CurOutArg)++;
   
   return (ERR_NONE);
}
#endif


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
void mexFunction (int nargout, Matrix *outargs [],      /* output args */
                  int nargin, Matrix *inargs [])        /* input args */
{
   char     *Filename;
   char     *Option;
   int      CDF;
   int      Result;		   /* return value from various functions */
   int      cur_outarg, cur_inarg; /* indeces into *outargs and *inargs arrays */

   ncopts = 0;
   ErrMsg = (char *) mxCalloc (256, sizeof(char));
   debug = FALSE;
   if (nargin == 0) ErrAbort ("Not enough arguments", TRUE, ERR_ARGS);

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

   if (nargin == 1) 
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

   /* If no explicit output arguments were given, set nargout to 1 because
    * MATLAB always supplies the "ans" output argument.
    */

   if (nargout == 0)
   {
      nargout = 1;
   }


   /* 
    * If we get here, more than one input argument was given.  Set cur_inarg 
    * to point to the first element of inargs[] after the filename, and cur_outarg
    * to point to the first element of outargs[] at all.  Then start processing
    * the input arguments from inargs [cur_inarg] on...
    */
   
   cur_inarg = 1;			/* point right after the filename */
   cur_outarg = 0;			/* point to the very first output arg */

   while (cur_inarg < nargin)
   {
      
#if 1+1==3
      if (cur_outarg >= nargout)		/* eg. if cur_outarg==0 we must have >= 1 output arg */
      {
	 ncclose (CDF);
	 ErrAbort ("Not enough output arguments", TRUE, ERR_ARGS);
      }
#endif
      
      /* Currently cur_inarg points to the next option in the argument list. */
      
      if (debug) printf ("Parsing inargs[%d]...", cur_inarg);
      if (ParseStringArg (inargs[cur_inarg], &Option) == NULL)
      {
	 ncclose (CDF);
	 ErrAbort ("Option argument must be a string", TRUE, ERR_ARGS);
      }
      if (debug) printf ("it's %s\n", Option);
      

      if (debug) printf ("Passing inargs[%d] and outargs[%d] on\n", 
			 cur_inarg, cur_outarg);
      
      /* Now take action based on value of string Option */
      
      if (strcasecmp (Option, "dimlength") == 0)
      {
	 Result = GetDimLength(CDF, nargin, inargs, &cur_inarg, 
			            nargout,outargs,&cur_outarg);
      } 
      else if (strcasecmp (Option, "imagesize") == 0)
      {
	 Result = GetImageSize(CDF, nargin, inargs, &cur_inarg, 
			            nargout,outargs,&cur_outarg);
      }
      else if (strcasecmp (Option, "vartype") == 0)
      {
	 Result = GetVarType (CDF, nargin, inargs, &cur_inarg,
			           nargout,outargs,&cur_outarg);
      }
/*
      else if (strcasecmp (Option, "attvalue") == 0)
      {
	 Result = GetAttValue (CDF, nargin, inargs, &cur_inarg,
			           nargout,outargs,&cur_outarg);
      }
*/
      else if ((strcasecmp (Option, "dimnames") == 0)
	       ||(strcasecmp (Option, "varnames") == 0)
	       ||(strcasecmp (Option, "vardims") == 0)
	       ||(strcasecmp (Option, "varatts") == 0)
	       ||(strcasecmp (Option, "varvalues") == 0)
	       ||(strcasecmp (Option, "atttype") == 0)
	       ||(strcasecmp (Option, "attvalue") == 0))
      { 
	 printf ("Sorry, option %s not yet supported.\n", Option);
	 cur_inarg++;
      }
      else
      {
	 sprintf (ErrMsg, "Unknown option: %s", Option);
	 ErrAbort (ErrMsg, TRUE, ERR_ARGS);
      }
      
      /* If ANY of the option-based calls above resulted in an error, BOMB! */
      if (Result != ERR_NONE)
      {
	 ncclose (CDF);
	 ErrAbort (ErrMsg, TRUE, Result);
      }
      
   }

   ncclose (CDF);

}
