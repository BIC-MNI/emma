function [K1,k2] = rcbf1 (filename, slice, progress)
% RCBF1  
%        a first try at a two-compartment rCBF model (without V0
%        or shifting) implemented as a MATLAB function.
%
%        [K1,k2] = rcbf1 (filename, slice)

% Input argument checking

if (nargin == 2)
    progress = 0;
elseif (nargin ~= 3)
    help rcbf1
    error('Incorrect number of arguments.');
end

if (length(slice)~=1)
    help rcbf1
    error('<Slice> must be a scalar.');
end

% Input arguments are checked, so now we can do some REAL work.

if (progress); disp ('Reading image information'); end
img = openimage(filename);
fstarts = getimageinfo (img, 'FrameTimes');
flengths = getimageinfo (img, 'FrameLengths');
midftimes = fstarts + (flengths / 2);
PET = getimages (img, slice, 1:length(fstarts));
PET = PET .* (PET > 0);			% set all negative values to zero

if (progress); disp ('Calculating rL image'); end
rL = findrl (PET, midftimes, flengths);

% Find the minimum/maximum of "reasonable" points in rL (i.e. those within
% one standard deviation of the mean -- this may seem overly picky, but
% typical values of rL appear to all be quite close to the mean with the
% few outliers VERY obvious).

%mu = mean (rL); sigma = std (rL);
%ok_points = find ( (abs(rL-mu) <= sigma) & (rL > 0));
%rLmin = min (rL (ok_points));
%rLmax = max (rL (ok_points));

if (progress); disp ('Calculating k2/rR lookup table'); end
[Ca_even, ts_even] = resampleblood (img, 'even');

k2_lookup = (0.001:0.02:5) / 60;
[conv_int1, conv_int2] = findintconvo (Ca_even, ts_even, k2_lookup,...
                            midftimes, flengths, [], midftimes);
rR = conv_int1 ./ conv_int2;

% Generate K1 and k2 images

if (progress); disp ('Calculating k2 image'); end
k2 = lookup(rR, k2_lookup, rL);

if (progress); disp ('Calculating K1 image'); end
int_activity = PET * flengths;
k2_conv_ints = lookup (k2_lookup, conv_int1, k2);
K1 = int_activity ./ k2_conv_ints;

nuke = find (isnan (K1) | isinf (K1));
K1 (nuke) = zeros (size (nuke));

% Magic number time: convert the values to the correct units.
% k2 is currently expressed in units of 1/sec but should be expressed
% 1/min; K1 is in (nCi * sec * (g blood)) / ((mL tissue) * counts * sec)
% and should be (mL blood) / ((g tissue) * minute).  Since blood and
% tissue are both taken to be 1.05 g / mL, and 1 nCi = 37 count/sec, and
% of course 1 min = 60 sec, the conversion factor is 2013.605442176871

k2 = k2 * 60;
K1 = K1 * 2013.605442176871;

% Cleanup

closeimage (img);



