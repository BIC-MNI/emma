/* ----------------------------- MNI Header -----------------------------------
@NAME       : frameint.c (CMEX)
@INPUT      : 
              
@OUTPUT     : 
              
@RETURNS    : 
@DESCRIPTION: 

@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : August 9, 1993
@MODIFIED   : 
---------------------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mex.h"


typedef int Boolean;


#define PROGNAME "frameint"



#define TIMES   prhs[0]
#define VALUES  prhs[1]
#define START   prhs[2]
#define LENGTHS prhs[3]
#define INTS    plhs[0]


#define TRUE    1
#define FALSE   0

#define min(A, B) ((A) < (B) ? (A) : (B))
#define max(A, B) ((A) > (B) ? (A) : (B))


Matrix  *mNaN;                  /* NaN as a MATLAB Matrix */
double  NaN;                    /* NaN in native C format */


void usage (void)
{
    mexPrintf("\nUsage:\n");
    mexPrintf("integrals = %s (ts, y, fstart, flengths)\n", PROGNAME);
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
Boolean CheckInputs (Matrix *TS, Matrix *Y, Matrix *FStart, Matrix *FLengths,
                     int *NumFrames, int *TableSize)
{
    int tsrows, tscols;                      /* used for TS */
    int yrows, ycols;                        /* used for Y */
    int fstart_rows, fstart_cols;
    int flengths_rows, flengths_cols;


    /*
     * Make sure that TS and Y are both column vectors of the same size.
     */

    tsrows = mxGetM (TS);
    tscols = mxGetN (TS);
    yrows = mxGetM (Y);
    ycols = mxGetN (Y);

    if (tscols != 1)
    {
        usage();
        mexErrMsgTxt("TS must be a column vector.");
    }
    if (ycols != 1)
    {
        usage();
        mexErrMsgTxt("Y must be a column vector.");
    }
    if (tsrows != yrows)
    {
        usage();
        mexErrMsgTxt("TS and Y must have the same number of rows.");
    }

    *TableSize = tsrows;

    /*
     * Make sure that FStart and FLengths are the same size.
     */

    fstart_rows = mxGetM(FStart);
    fstart_cols = mxGetN(FStart);
    flengths_rows = mxGetM(FLengths);
    flengths_cols = mxGetN(FLengths);

    if (min(fstart_rows, fstart_cols) != 1)
    {
        usage();
        mexErrMsgTxt("fstart must be a vector.");
    }
    if (min(flengths_rows, flengths_cols) != 1)
    {
        usage();
        mexErrMsgTxt("flengths must be a vector.");
    }
    if (max(flengths_rows, flengths_cols) != max(fstart_rows, fstart_cols))
    {
        usage();
        mexErrMsgTxt("fstart and flengths must be the same size.");
    }
    
    *NumFrames = max(fstart_rows, fstart_cols);

    return (TRUE);      /* indicate success -- we will have aborted if */
                        /* there was actually any error */
}       /* end CheckInputs */


/* ----------------------------- MNI Header -----------------------------------
@NAME       : C_trapz
@INPUT      : 
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Performs a trapezoid integration.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 
@MODIFIED   : 
---------------------------------------------------------------------------- */
void C_trapz (int num_bins, double *times, double *values,
              double *bin_lengths, double *area)
{
    int current_bin;

    *area = 0;

    for (current_bin=0; current_bin<(num_bins-1); current_bin++)
    {
        *area = *area + ((values[current_bin]+values[current_bin+1])/2*
                         (times[current_bin+1]-times[current_bin]));
    }
}



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
            newY [i] = NaN;
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
    }
}


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
    double *FStart;
    double *FLength;
    double *Int;
    double x_values[256];
    double y_values[256];
    double FStop;
    int numBins;
    int NumFrames;
    int TableSize;
    int CurFrame;
    int i;
    int LowIndex, HighIndex;

    if (nrhs != 4)
    {
        usage();
        mexErrMsgTxt("Incorrect number of input arguments!");
    }
    
    CheckInputs (TIMES, VALUES, START, LENGTHS, &NumFrames, &TableSize);

#ifdef DEBUG
    printf("Number of frames: %d\n", NumFrames);
#endif

    /*
     * Create the NaN variable
     */

    mexCallMATLAB (1, &mNaN, 0, NULL, "NaN");
    NaN = *(mxGetPr(mNaN));


    /*
     * Get pointers to the actual matrix data of the input arguments
     */

    X = mxGetPr (TIMES);
    Y = mxGetPr (VALUES);
    FStart = mxGetPr (START);
    FLength = mxGetPr (LENGTHS);
    
    INTS = mxCreateFull (NumFrames, 1, REAL);
    Int = mxGetPr (INTS);

    LowIndex = 0;

    for (CurFrame=0; CurFrame<NumFrames; CurFrame++)
    {
        FStop = FStart[CurFrame]+FLength[CurFrame];

        while (X[LowIndex] < FStart[CurFrame])
        {
            LowIndex++;
        }
        HighIndex = LowIndex;
        while (X[HighIndex] < FStop)
        {
            HighIndex++;
        }
        HighIndex--;                         /* Back up one point */

        /*
         * Create the x_values array
         */

        x_values[0] = FStart[CurFrame];
        for (i=1; i<(HighIndex-LowIndex+2); i++)
        {
            x_values[i] = X[LowIndex+i-1];
            y_values[i] = Y[LowIndex+i-1];
        }
        
        numBins = i+1;
        x_values[numBins-1] = FStop;
        
        /*
         * Lookup the limits of the y_values array
         */

        Lookup1 (X, Y, &(x_values[0]), &(y_values[0]), TableSize, 1);
        Lookup1 (X, Y, &(x_values[numBins-1]), &(y_values[numBins-1]), TableSize, 1);

	if (y_values[numBins-1] == NaN)
	{
	    numBins--;
	}
	if (y_values[0] == NaN)
	{
	    C_trapz((numBins-1), (x_values+1), (y_values+1), NULL, &(Int[CurFrame]));
	}
	else 
	{
	    C_trapz ((numBins), x_values, y_values, NULL, &(Int[CurFrame]));
	}
	

#ifdef DEBUG
        printf ("Current frame value (not normalised): %lg\n", Int[CurFrame]);
        printf ("------------- End of Current Frame ------------\n");
#endif

        Int[CurFrame] = Int[CurFrame] / (FLength[CurFrame]);
    }
}
