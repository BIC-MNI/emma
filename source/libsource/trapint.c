/* ----------------------------- MNI Header -----------------------------------
@NAME       : TrapInt
@INPUT      : num_bins - the number of points in times[] and values[]
              times[]  - vector of points on the abscissa - defines the
                         domain of the function being integrated
	      values[] - vector of points on the ordinate - defines the
	                 shape of the function
	      bin_lengths[] - the width of each interval to which the 
	                 elements of times[] and values[] refer.  Normally,
			 this would be taken to be simply the difference
			 between successive elements of times[]; however, 
			 if the intervals over which the function was 
			 sampled have gaps between them, supplying 
			 bin_lengths[] is the only way to get the complete
			 picture.  This parameter is currently IGNORED!
@OUTPUT     : *area - an approximation to the area under the curve defined
                      by values[], as calculated by a trapezoidal integration
@RETURNS    : (void)
@DESCRIPTION: Performs a trapezoidal integration of a function which is
              known only at certain fixed points of its domain.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 11 August 1993 (?), Mark Wolforth, as part of C_trapz.c
@MODIFIED   : 22 August 1993, Greg Ward: took out of ntrapz (aka C_trapz)
              into source file trapint.c; beefed up comments.
---------------------------------------------------------------------------- */
void TrapInt (int num_bins, double *times, double *values,
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
