#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minc.h"

#define PROGNAME "micreatedim"
#define MINC_FILE argv[1]
#define DIM_NAME argv[2]
#define LENGTH argv[3]


/* ----------------------------- MNI Header -----------------------------------
@NAME       : usage
@INPUT      : void
@OUTPUT     : void
@RETURNS    : void
@DESCRIPTION: Prints usage information for micreatedim
@METHOD     : none
@GLOBALS    : none
@CALLS      : none
@CREATED    : June 1, 1993 by MW
@MODIFIED   : 
---------------------------------------------------------------------------- */

void usage (void)
{
    
    (void) printf ("\nUsage: \n");
    (void) printf ("%s <file name> <dim name> <length>\n\n",PROGNAME);
    
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : main
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

void main (int argc, char *argv[])
{
    int file_CDF;
    long length;
    int status;


    ncopts = 0;
    
    
    if (argc <= 3)
    {
	usage();
	exit(0);
    }
    
    file_CDF = ncopen (MINC_FILE, NC_WRITE);
    if (file_CDF == MI_ERROR)
    {
	fprintf (stderr, "Could not open MINC file.\n");
	exit (-1);
    }

    status = ncredef (file_CDF);
    if (status == MI_ERROR)
    {
	fprintf (stderr, "Could not redefine MINC file.\n");
	ncclose (file_CDF);
	exit (-1);
    }
    
    if (strcmp(LENGTH,"NC_UNLIMITED") == 0)
    {
	length = NC_UNLIMITED;
    }
    else 
    {
	length = atoi(LENGTH);
    }
    
    status = ncdimdef (file_CDF, DIM_NAME, length);
    if (status == MI_ERROR)
    {
	fprintf (stderr, "Could not create dimension : %s\n", DIM_NAME);
	ncclose (file_CDF);
	exit (-1);
    }
    
    ncclose (file_CDF);
    exit(0);
}











