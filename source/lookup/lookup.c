/* ----------------------------- MNI Header -----------------------------------
@NAME       : lookup.c (CMEX)
@INPUT      : oldx, oldy - lookup table
              newx - vector of values to lookup
@OUTPUT     : newy - vector of values looked up in oldx/oldy table 
                     using newx
@RETURNS    : 
@DESCRIPTION: Do a linear interpolation / table lookup from MATLAB.
              Works the same as MATLAB's own interp1 or Gabe Leger's
              inter, but is about 400 times faster than interp1 and
              200 times faster than inter.  Note that nothing clever
              is done when an element of newx does not "fit" into the
              range of oldx; the corresponding member of newy is just
              set to NaN.  Also, this program *does* check for
              monotonicity, and correctly handles both increasing and
              decreasing oldx.  Makes no assumptions about the order
              of oldy or newx.
@METHOD     : 
@GLOBALS    : NaN - generated by calling mxCallMATLAB()
@CALLS      : Monotonic() in monotonic.o
              Lookup1(), Lookup2() in lookup12.o
@CREATED    : 93-6-27, Mark Wolforth & Greg Ward
@MODIFIED   : 93-6-28/29, Greg Ward: broke mexFunction() up, added 
                          monotonicity checking, and Lookup2 for 
                          decreasing oldx.
              93-7-21, added NaN stuff.
---------------------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gpw.h"
#include "mex.h"

/* 
 * External functions.  MUST link with monotonic.o, lookup12.o
 * in order to find these three functions.
 */

extern int Monotonic (double *oldX, int TableRows);
extern void Lookup1 (double *oldX, double *oldY,
		     double *newX, double *newY,
		     int TableRows, int OutputRows);
extern void Lookup2 (double *oldX, double *oldY,
		     double *newX, double *newY,
		     int TableRows, int OutputRows);


#define PROGNAME "lookup"
/* #define DEBUG */

#define OLDX    prhs[0]
#define OLDY    prhs[1]
#define NEWX    prhs[2]
#define NEWY    plhs[0]


Matrix	*mNaN;			/* NaN as a MATLAB Matrix */
double  NaN;			/* NaN in native C format */


void usage (void)
{
    mexPrintf("\nUsage:\n");
    mexPrintf("newY = %s (oldx, oldy, newx)\n", PROGNAME);
}




/* ----------------------------- MNI Header -----------------------------------
@NAME       : CheckInputs
@INPUT      : OldX, OldY, NewX - just what is passed from MATLAB
@OUTPUT     : InputRows - length of OldX / OldY vectors (must be same size)
              OutputRows - length of NewX vector
@RETURNS    : 1 if successful (i.e. all inputs are valid)
              does not return if there is an error - calls mexErrMsgTxt
@DESCRIPTION: Make sure that OldX, OldY, and NewX are vectors, and 
              that OldX and OldY have the same number of elements.
@METHOD     : 
@GLOBALS    : 
@CALLS      : standard MEX functions
@CREATED    : 93-6-28, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
int CheckInputs (Matrix *OldX,
                 Matrix *OldY,
                 Matrix *NewX,
                 int    *InputRows,
                 int    *OutputRows)
{
    int     xrows, xcols;       /* used for both NewX and OldX */
    int     yrows, ycols;       /* used for OldY */

    /*
     * Get size of new X vector, and make sure it is a vector
     */

    xrows = mxGetM (NewX);  xcols = mxGetN (NewX);
#ifdef DEBUG
    mexPrintf ("NewX: %d x %d\n", xrows, xcols);
#endif
    if (min (xrows,xcols) != 1)
    {
        usage();
        mexErrMsgTxt("newx must be a vector");
    }
    *OutputRows = max (xrows,xcols);

    /*
     * Get sizes of old X and Y vectors and make sure they are vectors
     * of the same length.
     */

    xrows = mxGetM (OldX);  xcols = mxGetN (OldX);
    yrows = mxGetM (OldY);  ycols = mxGetN (OldY);
#ifdef DEBUG
    mexPrintf ("OldX: %d x %d\n", xrows, xcols);
    mexPrintf ("OldY: %d x %d\n", yrows, ycols);
#endif
    if ((min (xrows,xcols) != 1) || (min (yrows,ycols) != 1))
    {
        usage();
        mexErrMsgTxt("oldx and oldy must be vectors");
    }

    *InputRows = max (xrows,xcols);
    if (*InputRows != max (yrows, ycols))
    {
        usage();
        mexErrMsgTxt("oldx and oldy must be the same length");
    }

#ifdef DEBUG
    mexPrintf ("Input args OK\n");
#endif
    return (1);         /* indicate success -- we will have aborted if */
                        /* there was actually any error */
}       /* CheckInputs */





/* ----------------------------- MNI Header -----------------------------------
@NAME       : mexFunction
@INPUT      : nlhs, plhs[] - number and array of input arguments
              nrhs - number of output arguments
@OUTPUT     : prhs[0] created and points to a vector of looked up Y
              values corresponding to input argument NEWX
@RETURNS    : (void)
@DESCRIPTION: 
@METHOD     : 
@GLOBALS    : 
@CALLS      : CheckInputs, Monotonic, Lookup1 / Lookup2
@CREATED    : 93-6-27, Mark Wolforth & Greg Ward
@MODIFIED   : 93-6-28, Greg Ward: took stuff out and made into CheckInputs
                                  and Lookup1/Lookup2 functions
---------------------------------------------------------------------------- */
void mexFunction (int nlhs, Matrix *plhs [],
                  int nrhs, Matrix *prhs [])
{
    double *newX;               /* these just point to the real parts */
    double *newY;               /* of various MATLAB Matrix objects */
    double *oldX;
    double *oldY;
/*    double slope;
    double temp;   */
    
    int output_size;            /* number of elements in the output vector */
    int table_size;             /* number of elements in the input table */
    int Direction;              /* 1=increasing, -1=decreasing */

    /* Create the NaN variable */
    mexCallMATLAB (1, &mNaN, 0, NULL, "NaN");
    NaN = *(mxGetPr(mNaN));

#ifdef DEBUG
    printf ("NaN is %lf\n", NaN);
#endif

#ifdef DEBUG
    if (nrhs == 1)
    {
        plhs[0] = mxCreateFull (1,1, REAL);
        table_size = max (mxGetM (OLDX), mxGetN(OLDX));
        *(mxGetPr (plhs[0])) = Monotonic (mxGetPr (OLDX), table_size);
        return;
    }
#endif
        
    if (nrhs != 3)
    {
        usage();
        mexErrMsgTxt("Incorrect number of input arguments!");
    }
    
    CheckInputs (OLDX, OLDY, NEWX, &table_size, &output_size);
#ifdef DEBUG
    mexPrintf ("output size = %d, table size = %d\n", output_size, table_size);
#endif

    /*
     * Get pointers to the actual matrix data of the input arguments
     */

    newX = mxGetPr (NEWX);
    oldX = mxGetPr (OLDX);
    oldY = mxGetPr (OLDY);

    Direction = Monotonic (oldX, table_size);
    if (Direction == 0)
    {
       usage();
       mexErrMsgTxt ("Input argument oldx must be monotonic");
    }

#ifdef DEBUG
    mexPrintf ("newx (1) = %g\n", newX [0]);
    mexPrintf ("oldx (1) = %g\n", oldX [0]);
    mexPrintf ("oldy (1) = %g\n", oldY [0]);
#endif

    /*
     * Create the output matrix (of the same size as the input NEWX; note
     * that currently CheckInputs requires NEWX to be a vector), and get
     * a pointer to its data
     */

    NEWY = mxCreateFull (mxGetM (NEWX), mxGetN (NEWX), REAL);
    newY = mxGetPr (NEWY);

    if (Direction == 1)
    {
       Lookup1 (oldX, oldY, newX, newY, table_size, output_size);
    }
    else
    {
       Lookup2 (oldX, oldY, newX, newY, table_size, output_size);
    }
}
