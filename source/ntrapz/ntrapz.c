/* ----------------------------- MNI Header -----------------------------------
@NAME       : trapint.c (CMEX)
@INPUT      : 
              
@OUTPUT     : 
              
@RETURNS    : 
@DESCRIPTION: 

@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : August 6, 1993
@MODIFIED   : 
---------------------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mex.h"


typedef int Boolean;


#define PROGNAME "ntrapz"


#define TIMES   prhs[0]
#define VALUES  prhs[1]
#define LENGTHS prhs[2]
#define AREA    plhs[0]


#define TRUE    1
#define FALSE   0

#define min(A, B) ((A) < (B) ? (A) : (B))
#define max(A, B) ((A) > (B) ? (A) : (B))


extern void TrapInt (int num_bins, double *times, double *values,
		     double *bin_lengths, double *area);


void usage (void)
{
    mexPrintf("\nUsage:\n");
    mexPrintf("area = %s (x, y, [bin_lengths])\n", PROGNAME);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : CheckInputs
@INPUT      : 
@OUTPUT     : 
@RETURNS    : Returns TRUE if successful
@DESCRIPTION: 
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 
@MODIFIED   : 
---------------------------------------------------------------------------- */
Boolean CheckInputs (Matrix *X, Matrix *Y, int *InputRows, int *InputCols)
{
    int     xrows, xcols;       /* used for X */
    int     yrows, ycols;       /* used for Y */

    /*
     * Get sizes of X and Y vectors and make sure they are vectors
     * of the same length.
     */

    xrows = mxGetM (X);  xcols = mxGetN (X);
    yrows = mxGetM (Y);  ycols = mxGetN (Y);

#ifdef DEBUG
    printf ("Input X is %d x %d\n", xrows, xcols);
    printf ("Input Y is %d x %d\n", xrows, xcols);
#endif

    if ((min(xrows,xcols) == 0) || min(yrows, ycols) == 0)
    {
	*InputRows = 0;
	*InputCols = 0;
	return (TRUE);
    }

    if (min(xrows, xcols) != 1)
    {
	usage();
	mexErrMsgTxt("X must be a vector.");
    }

    if (min(yrows, ycols) != 1)
    {
	if (xcols != 1)
	{
	    usage();
	    mexErrMsgTxt("X must be a column vector if Y is a matrix.");
	}
	if (xrows != yrows)
	{
	    usage();
	    mexErrMsgTxt("X and Y must have the same number of rows.");
	}

	*InputRows = xrows;
	*InputCols = ycols;
    }
    else 
    {
	if (max(xrows, xcols) != max(yrows, ycols))
	{
	    usage();
	    mexErrMsgTxt("X and Y must be vectors of the same length.");
	}

	*InputRows = max(xrows, xcols);
	*InputCols = 1;
    }


    return (TRUE);      /* indicate success -- we will have aborted if */
                        /* there was actually any error */
}       /* end CheckInputs */




/* ----------------------------- MNI Header -----------------------------------
@NAME       : mexFunction
@INPUT      : nlhs, plhs[] - number and array of input arguments
              nrhs - number of output arguments
@OUTPUT     : prhs[0] created and points to a vector
@RETURNS    : (void)
@DESCRIPTION: 
@METHOD     : 
@GLOBALS    : 
@CALLS      : CheckInputs
@CREATED    : 
@MODIFIED   : 
---------------------------------------------------------------------------- */
void mexFunction (int nlhs, Matrix *plhs [],
                  int nrhs, Matrix *prhs [])
{
    double *X;               /* these just point to the real parts */
    double *Y;               /* of various MATLAB Matrix objects */
    double *CurColumn;
    double *Lengths;
    double *Area;
    int xrows, ycols;
    int i;


    if ((nrhs != 3) && (nrhs != 2))
    {
        usage();
        mexErrMsgTxt("Incorrect number of input arguments!");
    }
    
    CheckInputs (TIMES, VALUES, &xrows, &ycols);

    /*
     * Get pointers to the actual matrix data of the input arguments
     */

    X = mxGetPr (TIMES);
    Y = mxGetPr (VALUES);
    if (nrhs == 3) 
    {
	Lengths = mxGetPr (LENGTHS);
    }

    if (ycols > 0)
    {
	AREA = mxCreateFull (1, ycols, REAL);
	Area = mxGetPr (AREA);
    }
    else 
    {
	AREA = mxCreateFull (1,1,REAL);
	Area = mxGetPr (AREA);
	*Area = 0;
	return;
    }

    if (nrhs == 3) 
    {
	for (i=0; i<ycols; i++)
	{
	    CurColumn = Y + (i*xrows);
	    TrapInt (xrows, X, CurColumn, Lengths, &(Area[i]));
	}
    }
    else
    {
	for (i=0; i<ycols; i++)
	{
	    CurColumn = Y + (i*xrows);
	    TrapInt (xrows, X, CurColumn, NULL, &(Area[i]));
	}
    }
}


    
    

