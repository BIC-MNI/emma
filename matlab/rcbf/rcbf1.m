function [K1,k2] = rcbf1 (filename, slice, progress)
% RCBF1  
%
%        [K1,k2] = rcbf1 (filename, slice)
%
%  a first try at a two-compartment rCBF model (without V0
%  or shifting) implemented as a MATLAB function.

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
MidFTimes = FrameTimes + (FrameLengths / 2);

[g_even, ts_even] = resampleblood (img, 'even');
% Apply the cross-calibration factor.
XCAL = 0.11;
g_even = g_even*XCAL*37;              % units are decay / (g_tissue * sec)

Ca_even = g_even; 			% no delay/dispersion correction!!!

PET = getimages (img, slice, 1:length(FrameTimes));
PET = PET * 37 / 1.05;                  % convert to decay / (g_tissue * sec)
PET = PET .* (PET > 0);			% set all negative values to zero
ImLen = size (PET, 1);                  % num of rows = length of image

if (progress); disp ('Calculating mask and rL image'); end

PET_int1 = trapz (MidFTimes, PET')';
PET_int2 = trapz (MidFTimes, PET' .* (MidFTimes * ones(1,ImLen)))';

%PET_int1 = PET * FrameLengths;
%PET_int2 = PET * (MidFTimes .* FrameLengths);

mask = PET_int1 > mean (PET_int1);
PET_int1 = PET_int1 .* mask;
PET_int2 = PET_int2 .* mask;

rL = PET_int1 ./ PET_int2;

if (progress); disp ('Calculating k2/rR lookup table'); end

k2_lookup = (0:0.02:3) / 60;
[conv_int1, conv_int2] = findintconvo (Ca_even, ts_even, k2_lookup,...
                            MidFTimes, FrameLengths, 1, MidFTimes);
rR = conv_int1 ./ conv_int2;

% Generate K1 and k2 images

if (progress); disp ('Calculating k2 image'); end
k2 = lookup(rR, k2_lookup, rL);

if (progress); disp ('Calculating K1 image'); end
k2_conv_ints = lookup (k2_lookup, conv_int1, k2);
K1 = PET_int1 ./ k2_conv_ints;

nuke = find (isnan (K1) | isinf (K1));
K1 (nuke) = zeros (size (nuke));

% Cleanup

closeimage (img);



