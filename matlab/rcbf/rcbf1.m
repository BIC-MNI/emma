function [k1,k2] = rcbf1 (filename, slice, progress)
% RCBF1  
%        a first try at a two-compartment rCBF model (without V0
%        or shifting) implemented as a MATLAB function.
%
%        [k1,k2] = rcbf1 (filename, slice)

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
FrameTimes = getimageinfo (img, 'FrameTimes');
FrameLengths = getimageinfo (img, 'FrameLengths');
MidFrameTimes = FrameTimes + (FrameLengths / 2);
PET = getimages (img, slice, 1:length(FrameTimes));
PET = PET .* (PET > 0);			% set all negative values to zero

if (progress); disp ('Calculating rL image'); end
rL = findrl (PET, MidFrameTimes, FrameLengths);

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
[conv_int1, conv_int2] = findconvints (k2_lookup, ...
	MidFrameTimes, FrameLengths, Ca_even, ts_even);
rR = conv_int1 ./ conv_int2;

% Generate K1 and K2 images

if (progress); disp ('Calculating k2 image'); end
k2 = lookup(rR, k2_lookup, rL);

if (progress); disp ('Calculating k1 image'); end
int_activity = PET * FrameLengths;

k2_conv_ints = lookup (k2_lookup, conv_int1, k2);
k1 = int_activity ./ k2_conv_ints;

nuke = find (isnan (k1));
k1 (nuke) = zeros (size (nuke));
nuke = find (isinf (k1));
k1 (nuke) = zeros (size (nuke));


% Magic number time: convert the values to the correct units.
% k2 is currently expressed in units of s^-1 but should be expressed
% min^-1; k1 is in (nCi * sec * (g blood)) / ((mL tissue) * counts * sec)
% and should be (mL blood) / ((g tissue) * minute).  Since blood and
% tissue are both taken to be 1.05 g / mL, and 1 nCi = 37 count/sec, and
% of course 1 min = 60 sec, the conversion factor is 2013.605442176871

k2 = k2 * 60;
k1 = k1 * 2013.605442176871;

% Cleanup

closeimage (img);



