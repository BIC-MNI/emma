/* ----------------------------- MNI Header -----------------------------------
@NAME       : miinquire.c  (CMEX)
@INPUT      : MINC_FILE 
@OUTPUT     : NUM_DIMS, NUM_VARS, NUM_GATTS
@RETURNS    : 
@DESCRIPTION: CMEX routine to get number of dimensions, variables, and global
              attributes for a MINC/NetCDF file via ncinquire.
@METHOD     : 
@GLOBALS    : 
@CALLS      : standard mex, NetCDF functions
@CREATED    : 93-6-8, Greg Ward
@MODIFIED   : 
@COMMENTS   : possibly modify to return names of dimensions, variables,
              and global attributes... trouble with this is how to make
				  a MATLAB character matrix whose members can easily be compared
				  to strings like 'time' or 'image' (problem is with the
				  space-padding necessary in character matrices).
---------------------------------------------------------------------------- */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "def_mni.h"
#include "mex.h"
#include "minc.h"
#include "myminc.h"
#include "mexutils.h"

#define MINC_FILE	prhs [0]			/* input arguments */

#define NUM_DIMS	plhs [0]			/* output arguments */
#define NUM_VARS	plhs [1]
#define NUM_GATTS	plhs [2]


void mexFunction (int nlhs, Matrix *plhs [],       /* output args */
                  int nrhs, Matrix *prhs [])       /* input args */
{
	char		*Filename;
	int		CDF;
	int		nDims, nVars, nGAtts;		/* returned by ncinq */

	ParseStringArg (MINC_FILE, &Filename);

	CDF = ncopen (Filename, NC_NOWRITE);
	if (CDF == MI_ERROR)
	{
		mexErrMsgTxt ("File not found");
	}

	if (ncinquire (CDF, &nDims, &nVars, &nGAtts, NULL) == MI_ERROR)
	{
		mexErrMsgTxt ("Error reading file");
	}

	NUM_DIMS = mxCreateFull (1, 1, REAL);
	NUM_GATTS = mxCreateFull (1, 1, REAL);
	NUM_VARS = mxCreateFull (1, 1, REAL);

	*(mxGetPr (NUM_DIMS)) = (double) nDims;
	*(mxGetPr (NUM_GATTS)) = (double) nGAtts;
	*(mxGetPr (NUM_VARS)) = (double) nVars;

}
