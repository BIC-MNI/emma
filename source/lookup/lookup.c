/* ----------------------------- MNI Header -----------------------------------
@NAME       : lookup.c (CMEX)
@INPUT      : oldx, oldy - lookup table
              newx - vector of values to lookup
@OUTPUT     : newy - vector of values looked up in oldx/oldy table 
                     using newx
@RETURNS    : 
@DESCRIPTION: Do a linear interpolation / table lookup from MATLAB.  Works
              the same as MATLAB's own interp1 or Gabe Leger's inter,
              but is about 400 times faster than interp1 and 200 times
              faster than inter.  Note that nothing clever is done
              when an element of newx does not "fit" into the range
              of oldx; the corresponding member of newy is just set
              to zero (perhaps this should be NaN?).  Also, this program
              *does* check for monotonicity, and correctly handles
              both increasing and decreasing oldx.  Makes no assumptions
              about the order of oldy or newx.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 93-6-27, Mark Wolfroth & Greg Ward
@MODIFIED   : 93-6-28/29, Greg Ward: broke mexFunction() up, added 
                          monotonicity checking, and Lookup2 for 
                          decreasing oldx.
---------------------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gpw.h"
#include "mex.h"


#define PROGNAME "lookup"
/* #define DEBUG */

#define OLDX    prhs[0]
#define OLDY    prhs[1]
#define NEWX    prhs[2]
#define NEWY    plhs[0]

#define sgn(x) ((x) > 0) ? 1 : (((x) < 0) ? -1 : 0)


void usage (void)
{
    mexPrintf("\nUsage:\n");
    mexPrintf("newY = %s (oldx, oldy, newx)\n", PROGNAME);
}


#if 0
int double_cmp (const void *dbl1, const void *dbl2)
{
   double   diff;

   diff = (*((double *) dbl1)) - (*((double *) dbl2));
   return (sgn (diff));
}
#endif


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


/* CheckOldX - make sure that oldX column is monotonic, and determine
   whether it is increasing or decreasing.  Returns:
     1 if oldX is monotonic increasing
    -1 if oldX is monotonic decreasing
     0 if oldX is not monotonic (including any two elements equal)
*/
/* ----------------------------- MNI Header -----------------------------------
@NAME       : CheckOldX
@INPUT      : oldX - pointer to C array of doubles
              TableRows - number of doubles in that array
@OUTPUT     : 
@RETURNS    : 0 if the elements of OldX[] are not monotonic
              1 if they are monotonically increasing
             -1 if they are monotonically decreasing
@DESCRIPTION: 
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 93-6-28, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
int CheckOldX (double *oldX, int TableRows)
{
    int j;
    double diff;
    int sign;    /* just the return value */
    int cursign;

    if (TableRows < 2)        /* not enough elements - results meaningless */
    {
       return (0);
    }

    diff = oldX [1] - oldX [0];
    sign = sgn (diff);
    if (sign == 0) return (0);      /* choke if two elements the same */

    for (j = 2; j < TableRows; j++)
    {
        diff = oldX [j] - oldX [j-1];
        cursign = sgn (diff);
        if ((cursign == 0) || (cursign != sign))
        {                           /* choke if two elements the same */
           return (0);              /* OR if the sign of the difference */
        }                           /* has changed */
    }       /* for j */

    return (sign);
}       /* CheckOldX () */


/* Lookup1 - linear interpolation with minimal checking.  Makes sure
   every element of newX is within the bounds of oldX, but thats it.
   ASSUMES that oldX is monotonically increasing.
*/
/* ----------------------------- MNI Header -----------------------------------
@NAME       : Lookup1
@INPUT      : oldX, oldY - lookup table
              newX - values to look up
              TableRows - number of elements in each of oldX, oldY
              OutputRows - number of elements in newX, newY
@OUTPUT     : newY - interpolated values corresponding to each member of newX
@RETURNS    : (void)
@DESCRIPTION: Perform a linear interpolation on every member of newX (ie.
              newX[0] .. newX[OutputRows-1]).  Each newX[i] is lookup in
              oldX[] so that the two elements of oldX[] bracketing newX[i]
              are found; newY[i] is then calculated by linearly interpolating
              between the elements of oldY[] corresponding to the bracketing
              elements of oldX[].  If any member of newX[] is NOT within
              the range of oldX[], then the corresponding newY will
              be simply zero.  NOTE: It is assumed that OldX is monotonically
              increasing; the behaviour of this function is undefined
              if that is not the case.  It may well loop infinitely
              or generate segmentation faults or other such unpleasantries.
              Use CheckOldX () before calling!!!
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 93-6-27, Mark Wolforth & Greg Ward
@MODIFIED   : 93-6-28, Greg Ward: moved to its own function, improved 
                                  checking for out-of-range newX
---------------------------------------------------------------------------- */
void Lookup1 (double *oldX, double *oldY,
              double *newX, double *newY,
              int TableRows,
              int OutputRows)
{
    int      i, j;
    double   slope;

    for (i=0; i<OutputRows; i++)
    {
        /*
         * Make sure that newX [i] is within the bounds of oldX [0..TableSize-1]
         * change this for oldX descending monotonic
         */

        if ((newX [i] < oldX [0]) || (newX [i] > oldX [TableRows-1]))
        {
            newY [i] = 0;               /* SHOULD BE MADE NaN!!!!!!!!! */
            continue;                   /* skip to next newY */
        }

        /*
         * Find the element (j+1) of oldX *just* larger than newX [i]
         * Note that we are guaranteed oldX[0] <= newX[i] <= oldX[TableRows-1]
         */
        
        j = 0;
        while (oldX [j+1] < newX [i])
        {
            j++;
        }

        /*
         * Now we have oldX [j] < newX [i] <= oldX [j+1], so interpolate
         * linearly to find newY [i]
         */

        slope = (oldY[j+1] - oldY[j]) / (oldX[j+1] - oldX[j]);
        newY [i] =  oldY[j] + slope*(newX[i] - oldX[j]);
    }       /* for i */
}       /* Lookup1 */


/* ----------------------------- MNI Header -----------------------------------
@NAME       : Lookup2
@INPUT      : (see Lookup1)
@OUTPUT     : (see Lookup1)
@RETURNS    : (see Lookup1)
@DESCRIPTION: Essentially the same as Lookup1, with a few comparisons
              changed.  This one assumes that oldX is monotonically
              *decreasing*, and again unwanted behaviour may well
              result if this is not so.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 93-6-29 Greg Ward: basically a copy of Lookup1
@MODIFIED   : 
---------------------------------------------------------------------------- */
void Lookup2 (double *oldX, double *oldY,
              double *newX, double *newY,
              int TableRows,
              int OutputRows)
{
    int      i, j;
    double   slope;

    for (i=0; i<OutputRows; i++)
    {
        /*
         * Make sure that newX[i] is within the bounds of oldX[0..TableSize-1]
         */

        if ((newX [i] > oldX [0]) || (newX [i] < oldX [TableRows-1]))
        {
            newY [i] = 0;               /* SHOULD BE MADE NaN!!!!!!!!! */
            continue;                   /* skip to next newY */
        }

        /*
         * Find the element (j+1) of oldX *just* smaller than newX [i]
         * Note that we are guaranteed oldX[0] >= newX[i] >= oldX[TableRows-1]
         */
        
        j = 0;
        while (oldX [j+1] > newX [i])
        {
            j++;
        }

        /*
         * Now we have oldX [j] > newX [i] >= oldX [j+1], so interpolate
         * linearly to find newY [i]
         */

        slope = (oldY[j+1] - oldY[j]) / (oldX[j+1] - oldX[j]);
        newY [i] =  oldY[j] + slope*(newX[i] - oldX[j]);
    }       /* for i */
}       /* Lookup2 */



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
@CALLS      : CheckInputs, CheckOldX, Lookup1 / Lookup2
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

#ifdef DEBUG
    if (nrhs == 1)
    {
        plhs[0] = mxCreateFull (1,1, REAL);
        table_size = max (mxGetM (OLDX), mxGetN(OLDX));
        *(mxGetPr (plhs[0])) = CheckOldX (mxGetPr (OLDX), table_size);
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

    Direction = CheckOldX (oldX, table_size);
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
