function [new_g, new_ts] = resampleblood (handle, type)
%  RESAMPLEBLOOD  resample the blood activity in some time new domain
%
%  [new_g, new_ts] = resampleblood (handle, type)
%
%  reads the blood activity and sample timing data from the study specified
%  by handle, and resamples the activity data at times specified by
%  the string type.  Currently, type can be one of 'even' or 'frame'.
%  For 'even', a new, evenly-spaced set of times will be generated
%  and used as the resampling times.  For 'frame', the mid frame times
%  will be used.  In either case, the resampled blood activity is
%  returned as new_g, and the times used are returned as new_ts.
%
%  Note that the number of elements in new_g is not necessarily the
%  same as that in Ca, the actual blood activity.  For 'even' sampling,
%  it will in fact by twice the number of original blood activity
%  data points (with a nod to Nyquist).  For 'frame' sampling, it
%  will simply be the number of frames.

if (nargin ~= 2)
	help resampleblood
	error('Incorrect number of arguments');
end

if (~isstr (type))
	help resampleblood
	error('argument "type" must be a string');
end

% Get the original blood activity data, and the start/stop times for
% each sample.  The mid-sample times, ts_mid, are presumed to be the
% times at which each element of Ca is the blood activity, hence ts_mid
% is used as "old x" for any resampling.

[Ca, ts_start, ts_stop] = getblooddata (handle);
ts_mid = (ts_start + ts_stop) / 2;

if (strcmp (type, 'even'))
	new_ts = linspace (min(ts_mid), max(ts_mid), 2 * length (ts_mid))';
	new_g = lookup (ts_mid, Ca, new_ts);
elseif (strcmp (type, 'frame'))
	tf_start = getimageinfo (handle, 'FrameTimes');
	tf_len = getimageinfo (handle, 'FrameLengths');
	new_ts = tf_start + (tf_len/2);
	new_g = lookup (ts_mid, Ca, new_ts);
else
	help resampleblood
	error(['Unknown sampling type: ' type]);
end
