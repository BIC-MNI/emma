function [int1, int2, int3] = findintconvo (Ca_even, ts_even, k2_lookup,...
                                     midftimes, flengths, w1, w2, w3)

% FINDINTCONVO   calculate tables of the integrated convolutions commonly used
%
%   [int1,int2,int3] = findintconvo (Ca_even, ts_even, k2_lookup,...
%                                    midftimes, flengths, w1[, w2[, w3]])
%
% given a table of k2 values, generates tables of weighted integrals
% that commonly occur in RCBF analysis.  Namely, int_convo is a table of
% the same size as k2_lookup containing
%
%       int ( conv (Ca(t), exp(-k2*t)) * weight )
%
% where the integration is carried out across frames.  weight is
% one of w1, w2, or w3, each of which will generally be some simple
% function of midftimes.  findintconvo will return int2 if and only if
% w2 is supplied, and int3 if and only if w3 is supplied.  w1 is 
% required, and int1 will always be returned.  Normally, the weight
% functions should be vectors with the same number of elements as
% midftimes; however, if w1 is empty then the weighting function 
% is taken to be unity.
%
% Note that in order to correctly calculate the convolution, Ca(t) must
% be resampled at evenly spaced time intervals, and this resampled blood
% activity should be passed as Ca_even.  The times at which it is
% sampled should be passed as ts_even.  (These can be calculated by
% resampleblood before calling findconvints.)
%
% Then, the convolution of Ca(t) and exp(-k2*t) is resampled at the
% mid-frame times (passed as midftimes) and integrated across frames
% using flengths as dt.


error (nargchk (6, 8, nargin));

% Get size of various time vectors - needed for initialization below

NumEvenTimes = length(ts_even);
NumFrames = length(midftimes);
fstart = midftimes - (flengths / 2);

% Now we need to calculate the function to convolve with Ca_even
% [a/k/a Ca(t)].  A note on the variables: exp_fun and integrand
% represent, respectively, the functions exp (-k2 * t) and [Ca(t) (*)
% exp (-k2 * t)] where (*) represents convolution.  (The t here is
% ts_even.)  integrand is then integrated across all frames
% by a simple rectangular integration, just as the image data is
% integrated in findrl.m.  This is then repeated for every element in
% the k2 vector to create a table of possible k2's and the two
% integrated convolutions conv_int1 and conv_int2 as described above.

TableSize = length (k2_lookup);
integrand = zeros (NumFrames, 1);               % this is integrated across frames

if (nargin >= 6); int1 = zeros (1, TableSize); end;
if (nargin >= 7); int2 = zeros (1, TableSize); end;
if (nargin == 8); int3 = zeros (1, TableSize); end;

% if w1 is empty, assume that it should be all ones

if isempty (w1)
   w1 = ones (size(NumFrames));
end


for i = 1:TableSize

fprintf('.')

   exp_fun = exp(-k2_lookup(i) * ts_even);
%   convo = conv (Ca_even, exp_fun);
   convo = conv(Ca_even, exp_fun, ts_even(2) - ts_even(1));

   integrand = frameint (ts_even, convo(1:length(ts_even)), fstart, flengths);

   % w1 given?

   if (nargin >= 6)
      int1 (i) = trapz(midftimes, (w1 .* integrand));
   end
   
   % w2 given?

   if (nargin >= 7)
      int2 (i) = trapz(midftimes, (w2 .* integrand));
   end

   % w3 given?
   
   if (nargin == 8)
       int3 (i) = trapz(midftimes, (w3 .* integrand));
   end
end

