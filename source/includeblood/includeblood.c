#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <minc.h>
#include "ncblood.h"



#define PROGNAME "includeblood"



/* ----------------------------- MNI Header -----------------------------------
@NAME       : usage
@INPUT      : void
@OUTPUT     : none
@RETURNS    : void
@DESCRIPTION: Prints the usage information for includeblood
@GLOBALS    : none
@CALLS      : printf
@CREATED    : May 30, 1994 by MW
@MODIFIED   : 
---------------------------------------------------------------------------- */
void usage (void)
{
    printf ("\nUsage:\n");
    printf ("%s <MNC file> <BNC file>\n\n", PROGNAME);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : CreateBloodStructures
@INPUT      : mincHandle  -> a handle for an open MINC file.  This file should
                             be open for writing, but not in redefinition
			     mode.
              bloodHandle -> a handle for an open BNC file.  This file should
                             be open for reading.
@OUTPUT     : none
@RETURNS    : void
@DESCRIPTION: Copies all variable definitions (with attributes) from the BNC
              file to the MINC file.  The appropriate dimensions are also
              copied.
@METHOD     : none.  Just muddled through.
@GLOBALS    : none
@CALLS      : micopy_all_var_defs (MINC library)
              miadd_child (MINC library)
@CREATED    : May 30, 1994 by MW
@MODIFIED   : 
---------------------------------------------------------------------------- */
void CreateBloodStructures (int mincHandle, int bloodHandle)
{
    int mincRoot;
    int bloodRoot;

    /*
     * Copy all the variables with their attributes.
     */

    micopy_all_var_defs (bloodHandle, mincHandle, 0, NULL);

    /*
     * Make the blood analysis root variable a child of
     * the MINC root variable.
     */

    mincRoot = ncvarid (mincHandle, MIrootvariable);
    bloodRoot = ncvarid (mincHandle, MIbloodroot);
    miadd_child (mincHandle, mincRoot, bloodRoot);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : FillBloodStructures
@INPUT      : mincHandle  -> a handle for an open MINC file.  This file should
                             be open for writing, but not in redefinition
			     mode.
              bloodHandle -> a handle for an open BNC file.  This file should
                             be open for reading.
@OUTPUT     : none
@RETURNS    : void
@DESCRIPTION: Copies all variable values from the BNC file to the MINC file.
              The variable themselves should already exist in the MINC file
              (see CreateBloodStructures).
@METHOD     : none.  Just muddled through.
@GLOBALS    : none
@CALLS      : micopy_all_var_values (MINC library)
@CREATED    : May 30, 1994 by MW
@MODIFIED   : 
---------------------------------------------------------------------------- */
void FillBloodStructures (int mincHandle, int bloodHandle)
{
    micopy_all_var_values (bloodHandle, mincHandle, 0, NULL);
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : includeblood
@INPUT      : 
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: 
@METHOD     : 
@GLOBALS    : 
@CALLS      : ncopen (netCDF library)
              ncredef (netCDF library)
	      ncendef (netCDF library)
	      ncclose (netCDF library)
	      CreateBloodStructures
	      FillBloodStructures
@CREATED    : May 30, 1994 by MW
@MODIFIED   : 
---------------------------------------------------------------------------- */
int main (int argc, char *argv[])
{
    int mincHandle, bloodHandle;

    if (argc != 3)
    {
	usage();
    }
    
    mincHandle = ncopen (argv[1], NC_WRITE);
    if (mincHandle == MI_ERROR)
    {
	fprintf (stderr, "Could not open: %s\n", argv[1]);
	return (-1);
    }
    bloodHandle = ncopen (argv[2], NC_NOWRITE);
    if (bloodHandle == MI_ERROR)
    {
	fprintf (stderr, "Could not open: %s\n", argv[2]);
	return (-2);
    }
    ncredef (mincHandle);
    
    CreateBloodStructures (mincHandle, bloodHandle);

    ncendef (mincHandle);

    FillBloodStructures (mincHandle, bloodHandle);

    ncclose (mincHandle);
    ncclose