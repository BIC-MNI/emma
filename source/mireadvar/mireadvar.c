/* ----------------------------- MNI Header -----------------------------------
@NAME       : mireadvar.c (CMEX)
@INPUT      : MATLAB input arguments: MINC filename, variable name, 
                vector of starting positions and vector of edge lengths
@OUTPUT     : hyperslab from the specified variable, jammed into a one-
                dimensional MATLAB matrix with the last dimension of 
                the variable varying fastest.
@RETURNS    : (void)
              ABORTS via mexErrMsgTxt in case of error
@DESCRIPTION: Read a hyperslab of values from a MINC variable into a 
              one-dimensional MATLAB Matrix.
@METHOD     : 
@GLOBALS    : 
@CALLS      : NetCDF, MINC, and mex functions.
@CREATED    : 93/5/31 - 93/6/2, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "def_mni.h"
#include "minc.h"
#include "mex.h"

#define calloc mxCalloc
#define PROGNAME "mireadvars"

/*
 * handy macros for accessing and checking the arguments to/from MATLAB
 */
#define MIN_IN_ARGS     2
#define MAX_IN_ARGS     5

#define FILENAME_POS    1        /* 1-based locations of the arguments */
#define VARNAME_POS     2        /* wrt the array of arguments passed from */
#define START_POS       3        /* MATLAB */
#define COUNT_POS       4
#define OPTIONS_POS     5

#define FILENAME        prhs [FILENAME_POS - 1]
#define VARNAME         prhs [VARNAME_POS - 1]
#define START           prhs [START_POS - 1]
#define COUNT           prhs [COUNT_POS - 1]
#define OPTIONS         prhs [OPTIONS_POS - 1]

#define RET_VECTOR      plhs [0]

#define MAX_OPTIONS     1

typedef int Boolean;

typedef struct
{
   int      CDF;                 /* ID for the CDF file of the variable */
   int      ID;                  /* for the dimension itself */
   char     Name[MAX_NC_NAME];   /* name of the dimension */
   long     Size;                /* number of data values in the dimension */
} DimInfoRec;

typedef struct
{
   int         CDF;           /* ID for the CDF file of the variable */
   int         ID;            /* ID for the variable itself */
   char        *Name;         /* the variable's name */
   nc_type     DataType;
   int         NumDims;       /* number of dimensions */
   DimInfoRec  *Dims;         /* info about every dimension associated */
                              /* with the variable */
   int         NumAtts;       /* number of attributes */ 
} VarInfoRec;


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
@MODIFIED   : 
---------------------------------------------------------------------------- */
void ErrAbort (char *msg)
{
   (void) mexPrintf ("Usage: %s ('MINC_file', 'var_name', ", PROGNAME);
   (void) mexPrintf ("[, start, count[, options]])\n");
   (void) mexPrintf ("where start and count are MATLAB vectors containing the starting index and\n");
   (void) mexPrintf ("number of elements to read for each dimension of variable var_name.\n\n");
   (void) mexErrMsgTxt (msg);
}



/* ----------------------------- MNI Header -----------------------------------
@NAME       : ParseOptions
@INPUT      : OptVector (a MATLAB Matrix)
@OUTPUT     : debug (and any other options I might want in future...)
@RETURNS    : 
@DESCRIPTION: Parses a "boolean vector" from MATLAB, assuming a correspondence
              between the elements of the vector and the Boolean arguments
              to this function.  Prints an error message and terminates 
              via ErrAbort if the options vector is incorrectly specified.
@METHOD     : 
@GLOBALS    : none
@CALLS      : standard mex functions
@CREATED    : 93-5-26, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
void ParseOptions (Matrix *OptVector, Boolean *debug)
{
   int    m, n;                        /* dimensions of options vector */
   char   MsgBuffer [80];

/* N.B. m = # rows, n = # cols; we want a row vector so require that m == 1 */

   m = mxGetM (OptVector);
   n = mxGetN (OptVector);

   if ((m != 1) || (n > MAX_OPTIONS) ||
       (!mxIsNumeric (OptVector)) || (mxIsComplex (OptVector)) ||
       (!mxIsFull (OptVector)))
   {
      sprintf (MsgBuffer,
        "Options must be a scalar or row vector with no more than %d element(s)",
               MAX_OPTIONS);
      ErrAbort (MsgBuffer);
   }

/*
 * OptVector is valid, so now we parse it -- right now, all we're 
 * interested in is seeing if the first element is 1, i.e. debug 
 * is true. 
 */

   *debug = *(mxGetPr (OptVector)) != 0;
}     /* ParseOptions () */



/* ----------------------------- MNI Header -----------------------------------
@NAME       : ParseStringArg
@INPUT      : Mstr - pointer to MATLAB Matrix; must be a string row vector
              debug - Boolean to print debug info
@OUTPUT     : *Cstr - pointer to newly allocated char array
              containing the string from Mstr
@RETURNS    : nothing, but aborts via ErrAbort in case of error!
@DESCRIPTION: Turn a valid MATLAB string into a C string
@METHOD     : Checks Mstr validity, allocates space for *Cstr, and copies
@GLOBALS    : 
@CALLS      : standard mex functions
@CREATED    : 93-5-31, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
void ParseStringArg (Matrix *Mstr, char *Cstr [], Boolean debug)
{
   int   m, n;                /* require m == 1, so n will be length of str */

   m = mxGetM (Mstr);    n = mxGetN (Mstr);

   if (debug)
   {
      mexPrintf ("Mstr is length %d\n", n);
   }

/* Require that Mstr is a "row strings" */
   if (!mxIsString (Mstr) || (m != 1))
      ErrAbort ("Invalid string argument supplied.");

/* All is well, so allocate space for the strings and copy them */
   *Cstr = (char *) mxCalloc (n+1, sizeof (char));
   m = mxGetString (Mstr, *Cstr, n+1);

   if (debug)
   {
      mexPrintf ("mxGetString (Mstr) returned %d\n", m);
      mexPrintf ("parsed string \`%s\'\n", *Cstr);
   }
}     /* ParseStringArg */



/* ----------------------------- MNI Header -----------------------------------
@NAME       : OpenFile
@INPUT      : Filename - name of the NetCDF/MINC file to open
              debug - as usual
@OUTPUT     : *CDF - handle of the opened file
@RETURNS    : (void)
              ABORTS via ErrAbort on error
@DESCRIPTION: Opens a NetCDF/MINC file using ncopen.
@METHOD     : 
@GLOBALS    : none
@CALLS      : standard NetCDF, mex functions.
@CREATED    : 93-5-31, adapted from code in micopyvardefs.c, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
void OpenFile (char *Filename, int *CDF, Boolean debug)
{
   char   MsgBuffer [256];

   ncopts = 0;         /* don't abort or print messages */

   if (debug)
   {
      (void) mexPrintf ("Opening %s for reading\n", Filename);
   }

   *CDF = ncopen (Filename, NC_NOWRITE);
   if (debug)
   {
      mexPrintf ("Immediately after ncopen, CDF = %d\n", *CDF);
   }     /* if debug */

   if (*CDF == MI_ERROR)
   {
      sprintf (MsgBuffer, "Error opening input file %s", Filename);
      ErrAbort (MsgBuffer);
   }
}     /* OpenFile */


/* ----------------------------- MNI Header -----------------------------------
@NAME       : GetVarInfo
@INPUT      : CDF - handle for a NetCDF file
              vName - string containing the name of the variable in question
              debug - as usual
@OUTPUT     : *vInfo - a struct which contains the CDF and variable id's,
                number of dimensions and attributes, and an array of 
                DimInfoRec's which tells everything about the various
                dimensions associated with the variable.         
@RETURNS    : (void)
              ABORTS via ErrAbort on error
@DESCRIPTION: Gets gobs of information about a NetCDF variable and its 
                associate dimensions.
@METHOD     : 
@GLOBALS    : 
@CALLS      : standard NetCDF, mex functions
@CREATED    : 93-5-31, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
void GetVarInfo (int CDF, char vName[], VarInfoRec *vInfo, Boolean debug)
{
   char        MsgBuff [256];
   int         DimIDs [MAX_NC_DIMS];
   int         dim;
   DimInfoRec  *Dims;      /* purely for convenience! */

   vInfo->CDF = CDF;
   vInfo->Name = vName;
   vInfo->ID = ncvarid (CDF, vName);

   /* 
    * Abort if there was an error finding the variable
    */
       
   if (vInfo->ID == MI_ERROR)
   {
      sprintf (MsgBuff, "Unknown variable: %s", vName);
      ErrAbort (MsgBuff);
   }     /* if ID == MI_ERROR */

   /*
    * Get most of the info about the variable...
    */

   ncvarinq (CDF, vInfo->ID, NULL, &vInfo->DataType, 
             &vInfo->NumDims, DimIDs, &vInfo->NumAtts);

   if (debug)
   {
      mexPrintf ("Variable %s has %d dimensions, %d attributes\n",
                 vInfo->Name, vInfo->NumDims, vInfo->NumAtts);
   }

   /*
    * Now loop through all the dimensions, getting info about them
    */

   Dims = (DimInfoRec *) mxCalloc (vInfo->NumDims, sizeof (DimInfoRec));
   for (dim = 0; dim < vInfo->NumDims; dim++)
   {
      ncdiminq (CDF, DimIDs [dim], Dims [dim].Name, &(Dims [dim].Size));
      if (debug)
      {
         mexPrintf ("  Dim %d: %s, size %d\n", 
                    dim, Dims[dim].Name, Dims [dim].Size);
      }
   }     /* for dim */  
   vInfo->Dims = Dims;
}     /* GetVarInfo */



/* ----------------------------- MNI Header -----------------------------------
@NAME       : ParseIntArg
@INPUT      : Mvector - pointer to a MATLAB matrix (of doubles)
              debug - as usual
@OUTPUT     : Cvector - 1-d array of longs, copied and converted from Mvector
                (N.B. must be allocated by caller!)
              VecSize - number of elements of Cvector used
@RETURNS    : (void)
              ABORTS via ErrAbort on error
@DESCRIPTION: Given a MATLAB vector (i.e. a Matrix where either m or n is 1)
                fills in a one-dimensional C array with long int's corres-
                ponding to the elements of the MATLAB vector.               
@METHOD     : Ensures that Mvector is a valid MATLAB object (one-dimensional,
                numeric, and real).  Gets the length of it.  Copies each
                element to Cvector, casting to long as it goes.
@GLOBALS    : 
@CALLS      : standard mex functions
@CREATED    : 93-6-1, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
void ParseIntArg (Matrix *Mvector, long Cvector[], int *VecSize, Boolean debug)
{
   int      m, n, i;
   double   *TmpVector;

   m = mxGetM (Mvector);   n = mxGetN (Mvector);
   if (((m != 1) && (n != 1)) || 
      (!mxIsNumeric (Mvector)) || 
      (mxIsComplex (Mvector)))
   {
      ErrAbort ("Invalid vector argument");
   }

   *VecSize = MAX (m, n);
   if (debug)
   {
      mexPrintf ("Input vector has %d elements\n", *VecSize);
   }

   TmpVector = mxGetPr (Mvector);

   for (i = 0; i < *VecSize; i++)
   {
      Cvector [i] = (long) TmpVector [i];

      if (debug)
      {
         mexPrintf ("  Tmpvector[i] = %g\t", TmpVector [i]);
         mexPrintf ("    Cvector[i] = %d\n", (int) Cvector [i]);
      }
   }     /* for i */

}     /* ParseIntArg */



/* ----------------------------- MNI Header -----------------------------------
@NAME       : VerifyVectors
@INPUT      : *vInfo - struct describing the variable and file in question
              Start[], Count[] - the vectors (start corner and edge lengths)
                to be checked for consistency with the variable described
                by *vInfo
              StartSize, CountSize - the number of elements of Start[]
                and Count[] that are actually used
              debug - same as usual
@OUTPUT     : (none)
@RETURNS    : (void)
              ABORTS via ErrAbort on error
@DESCRIPTION: Checks the Start[] and Count[] hyperslab specification vectors
                to ensure that they are consistent with the variable whose
                hyperslab they are meant to specify.  That is, all the Start
                positions must be within the variable's bounds (ie. the size
                of each dimension), and similarly Count cannot specify a
                value outside of those bounds.  Also requires that StartSize
                == CountSize, i.e. the two vectors describe the same number
                of dimensions.
@METHOD     :                
@GLOBALS    : (none)
@CALLS      : standard mex functions
@CREATED    : 93-6-1, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
void VerifyVectors (VarInfoRec *vInfo, 
                    long   Start[],    long  Count[],
                    int    StartSize,  int   CountSize,
                    Boolean debug)
{
   char  Msg [256];
   int   DimSize;       /* size of current dim - copied from vInfo->Dims */
   int   i;

   /*
    * Make sure that Start[] and Count[] have the same number of elements 
    */

   if (StartSize != CountSize)
   {
      ErrAbort ("Start and Count vectors must have same number of elements");
   }

   /*
    * And make sure that there is one element for every dimension 
    * associated with the variable described by vInfo.
    */

   if (StartSize != vInfo->NumDims)
   {
      ErrAbort ("Start and count vectors must have one element for every dimension");
   }

   /* 
    * Finally make sure that every start index is within the size of the
    * dimension, and that start+count is also.
    */

   for (i = 0; i < vInfo->NumDims; i++)
   {
      DimSize = vInfo->Dims[i].Size;   /* just save a little typing */
      if (debug)
      {
         mexPrintf ("Dimension %d (%s) has %d values.\n",
                    i, vInfo->Dims[i].Name, (int) DimSize);
         mexPrintf ("Desired values: %d through %d\n",
                    Start [i], Start [i] + Count [i] - 1);
      }
      if (Start [i] >= DimSize)
      {
         sprintf (Msg, "Start value for dimension %d is out of range (max %d)",
                  i, DimSize-1);
         ErrAbort (Msg);
      }     /* if start too large */
      if (Start [i] + Count [i] > DimSize)
      {
         sprintf (Msg,
"Attempt to read too many values from dimension %d (total dimension size %d)",
                  i, DimSize);
         ErrAbort (Msg);
      }     /* if start+count too large */
   }     /* for i */
}     /* VerifyVectors */



/* ----------------------------- MNI Header -----------------------------------
@NAME       : MakeDefaultVectors
@INPUT      : *vInfo - struct, tells which variable and file we are concerned with
              debug - as usual
@OUTPUT     : Start[], Count[] - NetCDF-style start/count vectors; these
                are filled in based on the size of each dimension
              *StartSize, *CountSize - the number of actual elements used
                in Start[] and Count[].  (Kind of redundant; just provided to
                be consistent with VerifyVectors.  Currently, these two
                just contain the number of dimensions in the variable.)
@RETURNS    : (void)
@DESCRIPTION: Sets up Start[] and Count[] vectors (as per the NetCDF standard,
                to be passed to ncvarget or mivarget) to read ALL values
                of the variable specified by *vInfo.
@METHOD     : Loops through however many dimensions the variable has, setting
                each element of Start[] to 0 and each element of Count[]
                to the length of that particular dimension.
@GLOBALS    : (none)
@CALLS      : standard mex functions
@CREATED    : 93-6-1, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
void MakeDefaultVectors (VarInfoRec *vInfo, 
                         long Start[],    long  Count[],
                         int  *StartSize, int   *CountSize,
                         Boolean debug)
{
   int   i;

   if (debug)
   {  
      mexPrintf ("Generating default start/count vectors...\n");
   }

   for (i = 0; i < vInfo->NumDims; i++)
   {
      Start [i] = 0;
      Count [i] = vInfo->Dims[i].Size;
      if (debug)
      {
         mexPrintf ("  dimension %d: start = %d, count = %d\n", 
                    i, (int) Start [i], (int) Count [i]);
      }
   }     /* for i */
  
   *StartSize = *CountSize = vInfo->NumDims;

}     /* MakeDefaultVectors */



/* ----------------------------- MNI Header -----------------------------------
@NAME       : ReadValues
@INPUT      : *vInfo - struct, tells which variable and file to read
                Start[], Count[] - starting corner and edge lengths of 
                hyperslab to read (just like ncvarget, etc.)
              debug - obligatory boolean flag
@OUTPUT     : **Dest - a MATLAB Matrix.  Note that *Dest is modified by
                this routine, as the Matrix is allocated here
@RETURNS    : (void)
              ABORTS via ErrAbort on error
@DESCRIPTION: Read a hyperslab of any valid NC type from a MINC file into 
              a one-dimensional MATLAB Matrix.  If more than one dimension
              is read from the MINC file, the Matrix will contain the values
              in the same order as mivarget() puts them there, i.e. with
              the last dimension of the MINC file varying fastest.
@METHOD     : Finds total number of elements, and allocates a 1-D MATLAB
              Matrix to hold them all.  Calls mivarget() to read all the
              values in, converting them to the NC_DOUBLE type, which
              is what MATLAB requires.  Does no scaling or shifting -- 
              see mireadimages if you need to read in image data.
@GLOBALS    : (none)
@CALLS      : standard NetCDF, mex functions
@CREATED    : 93-6-2, Greg Ward.
@MODIFIED   : 
---------------------------------------------------------------------------- */
void ReadValues (VarInfoRec *vInfo, 
                 long Start [], long Count [],
                 Matrix **Dest,
                 Boolean debug)
{  
   double      *TmpDest;
   long        TotSize;
   int         vgRet, i;

   if (debug)
   {
      mexPrintf ("Reading data...\n");
   }
   TotSize = 1;
   for (i = 0; i < vInfo->NumDims; i++)
   {
      TotSize *= Count [i];
      if (debug)
      {
         mexPrintf ("  dimension %d: start = %d, count = %d, TotSize = %d\n", 
                    i, (int) Start [i], (int) Count [i], (int) TotSize);
      }
   }

   if (TotSize == 0)
   {
      ErrAbort ("No values to read");
   }

   *Dest = mxCreateFull (TotSize, 1, REAL);
   TmpDest = mxGetPr (*Dest);
   vgRet = mivarget (vInfo->CDF, vInfo->ID, 
                     Start, Count, 
                     NC_DOUBLE, MI_SIGNED, TmpDest);

   if (vgRet == MI_ERROR)
   {
      ErrAbort ("Error reading from file!  (This is almost certainly a bug in mireadvar.c.)\n");
   }

   if (debug)
   {
      mexPrintf ("Read %d values:\n", TotSize);
      for (i = 0; i < TotSize; i++)
      {
         mexPrintf ("  %g\n", TmpDest [i]);
      }
   }
}     /* ReadValues */



/* ----------------------------- MNI Header -----------------------------------
@NAME       : mexFunction
@INPUT      : nlhs, nrhs - number of output, input arguments supplied by
                MATLAB caller
              prhs[] - array of pointers to the input arguments
@OUTPUT     : plhs[] - array of pointers to the output arguments
@RETURNS    : (void)
@DESCRIPTION: Given the name of a MINC file, a variable in it, and
                optional vectors of starting points and counts (as per
                ncvarget and mivarget), reads a hyperslab of values from
                the MINC variable into a one-dimensional MATLAB Matrix.
@METHOD     : Parses the filename and variable name from the MATLAB input
                arguments.  Opens the MINC file, reads information about
                the variable and its dimensions.  Parses the start/count
                vectors if they are given and checks them for validity;
                if not given, sets up defaults to read the entire variable.
                Reads the hyperslab.
@GLOBALS    : (none)
@CALLS      : standard mex, library functions; ErrAbort, ParseOptions,
                ParseStringArg, OpenFile, GetVarInfo, ParseIntArg,
                VerifyVectors, MakeDefaultVectors, ReadValues.
@CREATED    : 93-5-31, Greg Ward.
@MODIFIED   : 
---------------------------------------------------------------------------- */
void mexFunction (int nlhs, Matrix *plhs [],
                  int nrhs, Matrix *prhs [])
{
   char     Msg [256];        /* for building error messages */
   Boolean  debug;
   char     *Filename;
   char     *Varname;
   int      CDFid;
   VarInfoRec  VarInfo;       /* a nice handy structure */
   long     Start [MAX_NC_DIMS];
   long     Count [MAX_NC_DIMS];
   int      NumStart;      /* number of elements in Start[] and Count[] */
   int      NumCount;
   
   debug = FALSE;             /* default: can be overridden by caller */

   /*
    * Ensure that caller supplied correct number of input arguments
    */
   if ((nrhs < MIN_IN_ARGS) || (nrhs > MAX_IN_ARGS))
   {
      sprintf (Msg, "Incorrect number of arguments (%d): should be between %d and %d",
               nrhs, MIN_IN_ARGS, MAX_IN_ARGS);
      ErrAbort (Msg);
   }

   /*
    * If anything was given as an options vector, parse it.
    */
   if (nrhs >= OPTIONS_POS)         /* options given? then parse them */
   {
      ParseOptions (OPTIONS, &debug);
   }

   /*
    * Parse the two string options -- these are required
    */

   ParseStringArg (FILENAME, &Filename, debug);
   ParseStringArg (VARNAME, &Varname, debug);

   /*
    * Open the file and get info about the variable and its dimensions
    */

   OpenFile (Filename, &CDFid, debug); 
   GetVarInfo (CDFid, Varname, &VarInfo, debug);

   /*
    * If the start and count vectors are given (and they must BOTH be
    * given if either one is), parse them and verify their validity.
    * Otherwise call MakeDefaultVectors to set things up to read the
    * entire variable.
    */

   if (nrhs >= START_POS)           /* parse the start and count vectors */
   {
      if (nrhs < COUNT_POS)         /* can't have one without the other! */
      {
         ErrAbort ("Cannot supply just one of start and count vectors");
      }
      memset (Start, 0, MAX_NC_DIMS * sizeof (*Start));
      memset (Count, 0, MAX_NC_DIMS * sizeof (*Count));
      ParseIntArg (START, Start, &NumStart, debug);
      ParseIntArg (COUNT, Count, &NumCount, debug);

      VerifyVectors (&VarInfo, Start, Count, NumStart, NumCount, debug);

   }     /* if start and count vectors given */
   else
   {
      MakeDefaultVectors (&VarInfo, Start, Count, &NumStart, &NumCount, debug);
   }

   ReadValues (&VarInfo, Start, Count, &RET_VECTOR, debug);
}     /* mexFunction */
