function [new_g, new_ts] = resampleblood (handle, type, samples)
%  RESAMPLEBLOOD  resample the blood activity in some new time domain
%
%  [new_g, new_ts] = resampleblood (handle, type[, samples])
%
%  reads the blood activity and sample timing data from the study
%  specified by handle, and resamples the activity data at times
%  specified by the string type.  Currently, type can be one of 'even'
%  or 'frame'.  For 'even', a new, evenly-spaced set of times will be
%  generated and used as the resampling times.  For 'frame', the mid
%  frame times will be used.  In either case, the resampled blood
%  activity is returned as new_g, and the times used are returned as
%  new_ts.
%
%  The optional argument samples specifies the increase in number of
%  samples to take.  It defaults to two; that is, by default new_g will
%  return twice as many data points as the original g(t) data (from 
%  the MINC or BNC file).  For 'frame' sampling, samples has no effect:
%  new_g will simply contain the same number of points as there are
%  frames.

if (nargin < 2) | (nargin > 3)
   help resampleblood
   error('Incorrect number of arguments');
end

if (~isstr (type))
   help resampleblood
   error('argument "type" must be a string');
end

if (nargin == 2)								 % samples not supplied
   samples = 2;
end

% Get the original blood activity data, and the start/stop times for
% each sample.  The mid-sample times, ts_mid, are presumed to be the
% times at which each element of Ca is the blood activity, hence ts_mid
% is used as "old x" for any resampling.

[Ca, ts_start, ts_stop] = getblooddata (handle);
ts_mid = (ts_start + ts_stop) / 2;

if (strcmp (type, 'even'))
   new_ts = linspace (min(ts_mid), max(ts_mid), samples * length (ts_mid))';
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
