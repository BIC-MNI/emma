/* ----------------------------- MNI Header -----------------------------------
@NAME       : frameint.c (CMEX)
@INPUT      : 
   
@OUTPUT     : 
   
@RETURNS    : 
@DESCRIPTION: 
   
@METHOD     : 
@GLOBALS    : NaN (NaN in double format)
@CALLS      : 
@CREATED    : August 9, 1993
@MODIFIED   : 
---------------------------------------------------------------------------- */
#define _IEEE 1                 /* Needed for nan.h to work */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <nan.h>
#include "mex.h"


typedef int Boolean;


#define PROGNAME "frameint"
                 

#define MAX_X_LENGTH 1024       /* maximum number of elements of X that can */
                                /* be found within each frame */


#define TIMES   prhs[0]
#define VALUES  prhs[1]
#define START   prhs[2]
#define LENGTHS prhs[3]
#define INTS    plhs[0]


#define TRUE    1
#define FALSE   0

#define min(A, B) ((A) < (B) ? (A) : (B))
#define max(A, B) ((A) > (B) ? (A) : (B))


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
@NAME       : IntOneFrame
@INPUT      : X[] - array of X values (for the integrand)
              Y[] - array of Y values (for the integrand)
	      XYLength - the number of elements in the X and Y arrays
	      LowIndex - working variable that must be set to 0 for the
	                 first call to IntOneFrame, and preserved between
			 calls with the same X and successive frames
	      FrameStart - the starting point of the interval of X's we
	                   are interested in
	      FrameStop  - the ending point of that interval
	      Normalise  - if TRUE, normalise the integral of Y by dividing
	                   by the width of the interval of integration
@OUTPUT     : Integral - the integral of the function Y(X) over the interval
                         FrameStart <= X <= FrameStart+FrameLength 
			 (evaluated trapezoidally)
@RETURNS    : (void)
@DESCRIPTION: Taking Y as the values of a function, and X as the set
              of points on the x-axis corresponding to each value in
              Y, IntOneFrame determines which elements of X fall
              within the interval described by FrameStart and
              FrameStop and integrates Y with respect to X across that
              interval.  If Normalise is true, the integral is divided
	      by the width of the interval of integration.

	      If FrameStart and FrameStop each lie entirely within the
	      interval spanned by all of X, then Y is linearly interpolated
	      at FrameStart and FrameStop so as to form a closed interval
	      across which the integral is calculated.

	      If either or both of FrameStart and FrameStop is *not*
	      within X, then that endpoint is simply not included in
	      the integration, and the width of the interval is shortened
	      to only include the points of X at which Y is defined.

              Note: X ***MUST*** be monotonically increasing.  This is
              implicitly assumed throughout the routine, and NO
              checking of it is performed (for speed).

              Furthermore, if IntOneFrame is called successively, then
              the Frames must be non-overlapping and in increasing,
              and X must be the same.  This is because IntOneFrame
              uses a static variable to keep track of the start of
              each frame in X.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 13 August 1993, Greg Ward (from code originally by Mark
              Wolforth [9 August] and modified by GPW [12-13 August])
@MODIFIED   : 
---------------------------------------------------------------------------- */

void IntOneFrame (double X[], double Y[], int XYLength, int *LowIndex,
		  double FrameStart, double FrameStop,
		  Boolean Normalise,
		  double *Integral)
{
   int        i;
   int        HighIndex;
   double     x_values[MAX_X_LENGTH];
   double     y_values[MAX_X_LENGTH];
   int        numBins;             /* actual number of elements of x_values */
                                   /* and y_values used for integration     */
   char       Msg [256];


#ifdef DEBUG
   printf ("Current frame: start %g, stop %g\n",
	   FrameStart, FrameStop);
#endif

   /* 
    * First make sure that at least some of the frame lies within the 
    * range of X.  If not, just return NaN.
    */

   if ((FrameStop <= X[0]) | (FrameStart >= X [XYLength-1]))
   {
#ifdef DEBUG
      printf ("Frame entirely out of bounds\n");
#endif
      *Integral = NaN;
      return;
   }
   
   /*
    * Set *LowIndex and HighIndex so that they point to (respectively)
    * the first and last data points that fall *within* the current frame.
    */

   while (X[*LowIndex] <= FrameStart)
   {
      (*LowIndex)++;
   }

#ifdef DEBUG
   printf ("LowIndex = %d, X [LowIndex] = %g\n", *LowIndex, X [*LowIndex]);
#endif

   HighIndex = *LowIndex;
   if (FrameStop > X [XYLength-1])
   {
      HighIndex = XYLength-1;
   }
   else
   {
      while (X[HighIndex] < FrameStop)
      {
	 HighIndex++;
      }
      HighIndex--;                         /* Back up one point */
   }

   if ((HighIndex - (*LowIndex) + 3) >= MAX_X_LENGTH)
   {
      printf ("Frame start = %g, frame stop = %g\n", FrameStart, FrameStop);
      printf ("Global: Lowest x = %g, highest x = %g\n", X[0], X[XYLength-1]);
      printf (" Frame: Lowest x = %g, highest x = %g\n", 
	      X[*LowIndex], X[HighIndex]);
      printf ("Number of elements = %d, max is %d\n", 
	      (HighIndex - (*LowIndex) + 3), MAX_X_LENGTH);
      mexErrMsgTxt ("Oh, NO!!!  Found too many X values within frame!");
   }
   
#ifdef DEBUG
   printf (" LowIndex = %d,  X [LowIndex] = %g\n", *LowIndex, X [*LowIndex]);
   printf ("HighIndex = %d, X [HighIndex] = %g\n", HighIndex,X [HighIndex]);
   printf ("Now filling x_values with x[LowIndex..HighIndex]\n");
#endif
   
   /*
    * Fill the x_values array
    */
   
   x_values[0] = FrameStart;
#ifdef DEBUG
   printf ("                x_values[0] = %g\n", x_values[0]);
#endif
   
   for (i=1; i<(HighIndex-(*LowIndex)+2); i++)
   {
#ifdef DEBUG2
      printf ("Copying X[%d] to x_values[%d] = %g\n",
	      (*LowIndex)+i-1, i, X[(*LowIndex)+i-1]);
#endif
      x_values[i] = X[(*LowIndex)+i-1];
      y_values[i] = Y[(*LowIndex)+i-1];
   }
   
   numBins = i+1;
   x_values[numBins-1] = FrameStop;
#ifdef DEBUG
   printf ("                x_values [%d] = %g\n", 
	   numBins-1, x_values[numBins-1]);
   printf ("numBins = %d\n", numBins);
#endif
   
   /*
    * Lookup the limits of the y_values array.  Note that either one (and
    * possibly both, though this is unlikely) of these could result
    * in a NaN; however, those are the ONLY possible NaN's.  (Unless we
    * have been passed data with NaN's, which is Somebody Else's Problem.
    */
   
#ifdef DEBUG
   printf ("Lower limit of x = %lg\n", x_values[0]);
   printf ("Upper limit of x = %lg\n", x_values[numBins-1]);
#endif
   Lookup1 (X, Y, &(x_values[0]), &(y_values[0]), XYLength, 1);
   Lookup1 (X, Y, &(x_values[numBins-1]), &(y_values[numBins-1]), XYLength, 1);
#ifdef DEBUG
   printf ("y at lower limit of x = %lg\n", y_values[0]);
   printf ("y at upper limit of x = %lg\n", y_values[numBins-1]);
#endif
   
   
#ifdef DEBUG2
   printf ("Dumping %d bins...\n", numBins);
   for (i = 0; i < numBins; i++)
   {
      printf ("x_values [%3d] = %g, y_values [%3d] = %g\n",
	      i, x_values [i], i, y_values [i]);
   }
#endif
   
   
   
   /* Note: a blunt "== NaN" comparison doesn't work.  Must use the 
    * more sophisticated macros provided in nan.h.
    */
   
   if (IsNANorINF(y_values[numBins-1]))
   {
      numBins--;
#ifdef DEBUG
      printf ("Found NaN at end of y's, decreasing numBins from %d to %d\n",
	      numBins+1, numBins);
      printf ("New lower limit of x = %lg\n", x_values[0]);
      printf ("New upper limit of x = %lg\n", x_values[numBins-1]);
      printf ("y at new lower limit of x = %lg\n", y_values[0]);
      printf ("y at new upper limit of x = %lg\n", y_values[numBins-1]);
#endif
   }
   if (IsNANorINF(y_values[0]))
   {
#ifdef DEBUG
      printf ("Found NaN at front of y's, starting integral with x=%lg\n",
	      x_values[1]);
      printf ("Integrating %d points from x=%g to %g (y=%g to %g)\n",
	      numBins-1, x_values[1], x_values[numBins-1],
	      y_values[1], y_values[numBins-1]);
#endif
      
      
      C_trapz((numBins-1), (x_values+1), (y_values+1), NULL, Integral);

#ifdef DEBUG
      printf ("Unnormalised integral = %g\n", *Integral);
      printf ("Normalising by %g\n", (x_values[numBins-1] - x_values[1]));
#endif

      *Integral = *Integral / (x_values[numBins-1] - x_values[1]);
   }
   else 
   {
#ifdef DEBUG
      printf ("No NaN at front, starting integral with x=%lg\n",
	      x_values[0]);
      printf ("Integrating %d points from x=%g to %g (y=%g to %g)\n",
	      numBins, x_values[0], x_values[numBins-1],
	      y_values[0], y_values[numBins-1]);
#endif

      C_trapz ((numBins), x_values, y_values, NULL, Integral);

#ifdef DEBUG
      printf ("Unnormalised integral = %g\n", *Integral);
      printf ("Normalising by %g\n", (x_values[numBins-1] - x_values[1]));
#endif
      *Integral = *Integral / (x_values[numBins-1] - x_values[0]);
   }

}     /* IntOneFrame () */




/* ----------------------------- MNI Header -----------------------------------
@NAME       : mexFunction
@INPUT      : nlhs, plhs[] - number and array of input arguments
              nrhs - number of output arguments
@OUTPUT     : prhs[0] created and points to a vector
@RETURNS    : (void)
@DESCRIPTION: 
@METHOD     : 
@GLOBALS    : 
@CALLS      : CheckInputs, IntOneFrame
@CREATED    : 
@MODIFIED   : 
---------------------------------------------------------------------------- */
void mexFunction (int nlhs, Matrix *plhs [],
                  int nrhs, Matrix *prhs [])
{
   Matrix  *mNaN;                  /* NaN as a MATLAB Matrix */
   double *X;               /* these just point to the real parts */
   double *Y;               /* of various MATLAB Matrix objects */
   double *FStart;
   double *FLength;
   double *Int;

   int NumFrames;           /* size of FStart and FLength */
   int CurFrame;            /* loop counter for frames */
   int TableSize;           /* size of X and Y */
   int LowIndex;            /* needed for IntOneFrame */
   
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

      IntOneFrame (X, Y, TableSize, &LowIndex,
		   FStart[CurFrame], FStart[CurFrame]+FLength[CurFrame],
		   TRUE, &(Int[CurFrame]));
      
#ifdef DEBUG
      printf ("Current frame value (normalised): %lg\n", Int[CurFrame]);
      printf ("------------- End of Frame %d/%d------------\n", 
	      CurFrame+1, NumFrames);
#endif
      
/*    Int[CurFrame] = Int[CurFrame] / (FLength[CurFrame]);     */
   }
}
