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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mex.h"

#define TRUE 1
#define FALSE 0

#define min(A, B) ((A) < (B) ? (A) : (B))
#define max(A, B) ((A) > (B) ? (A) : (B))

#define PROGNAME "rescale"

/*
 * Constants to check for argument number and position
 */


#define MIN_IN_ARGS        2
#define MAX_IN_ARGS        2

/* ...POS macros: 1-based, used to determine if input args are present */

#define OLD_MATRIX         prhs[0]
#define CONSTANT           prhs[1]

typedef int Boolean;


/*
 * Global variables (with apologies).  Interesting note:  when ErrMsg is
 * declared as char [256] here, MATLAB freezes (infinite, CPU-hogging
 * loop the first time any routine tries to sprintf to it).  Dynamically
 * allocating it seems to work fine, though... go figure.
 */

char       *ErrMsg ;             /* set as close to the occurence of the
                                    error as possible; displayed by whatever
                                    code exits */



/* ----------------------------- MNI Header -----------------------------------
@NAME       : ErrAbort
@INPUT      : msg - character to string to print just before aborting
              PrintUsage - whether or not to print a usage summary before
                aborting
              ExitCode - one of the standard codes from mierrors.h -- NOTE!  
                this parameter is NOT currently used, but I've included it for
                consistency with other functions named ErrAbort in other
                programs
@OUTPUT     : none - function does not return!!!
@RETURNS    : 
@DESCRIPTION: Optionally prints a usage summary, and calls mexErrMsgTxt with
              the supplied msg, which ABORTS the mex-file!!!
@METHOD     : 
@GLOBALS    : requires PROGNAME macro
@CALLS      : standard mex functions
@CREATED    : 93-6-6, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
void ErrAbort (char msg[], Boolean PrintUsage, int ExitCode)
{
   if (PrintUsage)
   {
      (void) mexPrintf ("Usage: %s ('<matrix>' , scalar)\n", PROGNAME);
   }
   (void) mexErrMsgTxt (msg);
}



/* ----------------------------- MNI Header -----------------------------------
@NAME       : mexFunction
@INPUT      : nlhs, nrhs - number of output/input arguments (from MATLAB)
              prhs - actual input arguments 
@OUTPUT     : plhs - actual output arguments
@RETURNS    : (void)
@DESCRIPTION: 
@METHOD     : 
@GLOBALS    : ErrMsg
@CALLS      : 
@CREATED    : 
@MODIFIED   : 
---------------------------------------------------------------------------- */
void mexFunction(int    nlhs,
                 Matrix *plhs[],
                 int    nrhs,
                 Matrix *prhs[])
{

    double *base;
    double constant;
    long position;
    long size;

    position = 0;

    ErrMsg = (char *) mxCalloc (256, sizeof (char));
    
    /* First make sure a valid number of arguments was given. */
    
    if ((nrhs < MIN_IN_ARGS) || (nrhs > MAX_IN_ARGS))
    {
	strcpy (ErrMsg, "Incorrect number of arguments.");
	ErrAbort (ErrMsg, TRUE, -1);
    }
    
    if ((mxGetM(CONSTANT) != 1) || (mxGetN(CONSTANT) != 1))
    {
	strcpy (ErrMsg, "Argument 2 must be a scalar.\n");
	ErrAbort (ErrMsg, TRUE, -1);
    }

    constant = mxGetScalar(CONSTANT);
    
    size = ((long)mxGetM(OLD_MATRIX)) * ((long)mxGetN(OLD_MATRIX));

    base = mxGetPr(OLD_MATRIX);
    
    while (position < size)
    {
	*(base+position) = *(base+position) * constant;
	position++;
    }

}     /* mexFunction */
