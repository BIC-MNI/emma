#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minc.h"


#define PROGNAME      "micreateimage"
#define MINC_FILE     argv[1]
#define SIZE          argv[2]
#define NUM_SLICES    argv[3]
#define NUM_FRAMES    argv[4]

typedef int Boolean;


/* ----------------------------- MNI Header -----------------------------------
@NAME       : usage
@INPUT      : void
@OUTPUT     : void
@RETURNS    : void
@DESCRIPTION: Prints usage information for micreateimage
@METHOD     : none
@GLOBALS    : none
@CALLS      : none
@CREATED    : June 2, 1993 by MW
@MODIFIED   :
---------------------------------------------------------------------------- */

void usage (void) 
{
    printf ("\nUsage:\n");
    printf ("%s <MINC file> <size> <number of slices> ", PROGNAME);
    printf ("[<number of frames>]\n\n");
}


/* ----------------------------- MNI Header -----------------------------------
@NAME       : GetSlicesAndFrames
@INPUT      : num_slices -> A character string containing a number representing
                            the number of slices.
              num_frames -> A character string containing a number representing
                            the number of frames.  Can also be left NULL.
	      frames_exist -> If false, num_frames should be ignored.
@OUTPUT     : slices -> An integer representation of the number of slices in
                        the image.
              frames -> An integer representation of the number of frames in
	                the image.
@RETURNS    : void
@DESCRIPTION: Gets the number of slices and number of frames from character
              strings.  The frame information can be ignored by setting
	      frames_exist to false.
@METHOD     : none
@GLOBALS    : none
@CALLS      : none
@CREATED    : June 2, 1993 by MW
@MODIFIED   : 
---------------------------------------------------------------------------- */
void GetSlicesAndFrames (char num_slices[], int *slices,
                         char num_frames[], int *frames,
                         Boolean frames_exist)
{
    
    if (!isdigit(num_slices[0]))
    {
	fprintf (stderr, "The number of slices must be a digit.\n");
	exit (-1);
    }
    *slices = atoi (num_slices);

    if (frames_exist)
    {
	if (!isdigit(num_frames[0]))
	{
	    fprintf (stderr, "The number of frames must be a digit.\n");
	    exit (-1);
	}
	*frames = atoi (num_frames);
    }
}
    

/* ----------------------------- MNI Header -----------------------------------
@NAME       : main
@INPUT      : see usage
@OUTPUT     : none
@RETURNS    : none
@DESCRIPTION: Sets up a new MINC file so that it can contain image data.
              Creates the dimensions, and the image, time, time-width,
              image-max, and image-min variables.
@METHOD     : none
@GLOBALS    : ncopts
@CALLS      : usage
              GetSlicesAndFrames
              MINC library
              NetCDF library
@CREATED    : June 3, 1993 by MW
@MODIFIED   : 
---------------------------------------------------------------------------- */

void main (int argc, char *argv[])
{
    int file_CDF;
    int dim[4];
    double vrange[2];
    int num_dimensions;
    int slices;
    int frames;
    int image_id, max_id, min_id;
    int time_id, time_width_id;
    Boolean frames_exist;
    
    ncopts = 0;

    if (argc <= 3)
    {
	usage();
	exit(0);
    }

    frames_exist = (Boolean)(argc-4);
    GetSlicesAndFrames (NUM_SLICES, &slices, NUM_FRAMES, &frames, frames_exist);

    file_CDF = ncopen (MINC_FILE, NC_WRITE);
    if (file_CDF == MI_ERROR)
    {
	fprintf (stderr, "Could not open the MINC file : %s\n", MINC_FILE);
	exit (-1);
    }
    ncredef(file_CDF);

    if (frames_exist)
    {
	dim[0] = ncdimdef (file_CDF, MItime, frames);
	dim[1] = ncdimdef (file_CDF, MIzspace, slices);
	dim[2] = ncdimdef (file_CDF, MIyspace, atoi(SIZE));
	dim[3] = ncdimdef (file_CDF, MIxspace, atoi(SIZE));
	num_dimensions = 4;
	max_id = micreate_std_variable (file_CDF, MIimagemax, NC_DOUBLE, 2, dim);
	min_id = micreate_std_variable (file_CDF, MIimagemin, NC_DOUBLE, 2, dim);
	time_id = micreate_std_variable (file_CDF, MItime, NC_DOUBLE, 1, dim);
	time_width_id = micreate_std_variable (file_CDF, MItime_width, NC_DOUBLE, 1, dim);
    }
    else 
    {
	dim[0] = ncdimdef (file_CDF, MIzspace, slices);
	dim[1] = ncdimdef (file_CDF, MIyspace, atoi(SIZE));
	dim[2] = ncdimdef (file_CDF, MIxspace, atoi(SIZE));
	num_dimensions = 3;
	max_id = micreate_std_variable (file_CDF, MIimagemax, NC_DOUBLE, 1, dim);
	min_id = micreate_std_variable (file_CDF, MIimagemin, NC_DOUBLE, 1, dim);
    }
        
    image_id = micreate_std_variable (file_CDF, MIimage, NC_BYTE,
				      num_dimensions, dim);
    (void) miattputstr (file_CDF, image_id, MIsigntype, MI_UNSIGNED);
    (void) miattputstr (file_CDF, image_id, MIcomplete, MI_FALSE);
    
    vrange[0] = 0;
    vrange[1] = 255;
    (void) ncattput (file_CDF, image_id, MIvalid_range, NC_DOUBLE, 2, vrange);

    ncclose (file_CDF);
}

