#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mex.h"


#define PROGNAME "lookup"

#define OLDX    prhs[0]
#define OLDY    prhs[1]
#define NEWX    prhs[2]
#define NEWY    plhs[0]


void usage (void)
{
    mexPrintf("\nUsage:\n");
    mexPrintf("Y0 = %s (X, Y, X0)\n", PROGNAME);
}


void mexFunction (int nlhs, Matrix *plhs [],
		  int nrhs, Matrix *prhs [])
{
    double *newX;
    double *newY;
    double *oldX;
    double *oldY;
    double slope;
    double temp;
    
    int output_size;
    int table_size;
    int i,j,k;
        
    if (nrhs != 3)
    {
	usage();
	mexErrMsgTxt("Incorrect number of input arguments!");
    }
    
    newX = mxGetPr (NEWX);
    oldX = mxGetPr (OLDX);
    oldY = mxGetPr (OLDY);

    if (mxGetM (NEWX) == 1)
    {
	output_size = mxGetN (NEWX);
    }
    else
    {
	output_size = mxGetM (NEWX);
    }
    if (mxGetM (OLDX) == 1)
    {
	table_size = mxGetN (OLDX);
    }
    else
    {
	table_size = mxGetM (OLDX);
    }

    NEWY = mxCreateFull (output_size, 1, REAL);
    newY = mxGetPr (NEWY);
    
    for (i=0; i<output_size; i++)
    {
	newY[i] = 0;
	j = 0;
	
	while ((oldX [j+1] < newX [i]) && (j<(table_size)))
	{
	    j++;
	}
	
	if (j == table_size)
	{
	    mexPrintf ("Reached end of table without interpolating. Uh-oh.\n");
	}
	else
	{
/*	    mexPrintf ("Bracketing y's: %g, %g\n", oldY[j], oldY[j+1]);
	    mexPrintf ("Bracketing x's: %g, %g\n", oldX[j], oldX[j+1]);
*/	    
	    slope = (oldY[j+1] - oldY[j]) / (oldX[j+1] - oldX[j]);
	    newY [i] =  oldY[j] + slope*(newX[i] - oldX[j]);
/*	    mexPrintf ("I think slope=%g, newy=%g\n", slope, temp);
*/	    
	}
		
    }
}





