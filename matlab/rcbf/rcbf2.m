function [K1,k2,V0] = rcbf2 (filename, slice, progress)
% RCBF2
%        a two-compartment rCBF model implemented
%        as a MATLAB function.
%
%        [K1,k2,V0] = rcbf2 (filename, slice)

% Input argument checking

if (nargin == 2)
    progress = 0;
elseif (nargin ~= 3)
    help rcbf2
    error('Incorrect number of arguments.');
end

if (length(slice)~=1)
    help rcbf2
    error('<Slice> must be a scalar.');
end

% Input arguments are checked, so now we can do some REAL work, ie.
% read in all the data we need

if (progress); disp ('Reading image information and generating mask'); end
img = openimage(filename);
FrameTimes = getimageinfo (img, 'FrameTimes');
FrameLengths = getimageinfo (img, 'FrameLengths');
MidFrameTimes = FrameTimes + (FrameLengths / 2);
[Ca_even, ts_even] = resampleblood (img, 'even');

% Generate a simple mask: all pixels in integrated image > average

PET = getimages (img, slice, 1:length(FrameTimes));
summed = PET * FrameLengths;
mask = summed > mean(summed);

% Now apply the mask to the images in PET (this may be a little slow,
% but it's better memory-wise than creating 21 copies of mask 
% and .* with PET).

for i=1:size(PET,2)
   PET(:,i) = PET(:,i) .* mask;
end

% Find the value of rL for every pixel of the slice.

if (progress); disp ('Calculating rL image'); end
rL = findrl2 (PET, MidFrameTimes, FrameLengths, ts_even, Ca_even);

% Pick values of k2 and then calculate rR for every one.  (Ie. create
% the lookup table).  Note that rR is calculated as the quotient
% of conv_int1 and conv_int2, since conv_int1 is used again for
% looking up the values of K1.

if (progress); disp ('Generating k2/rR lookup table'); end
k2_lookup = (-50:0.02:50) / 60;
[conv_int1, conv_int2] = findconvints2 (k2_lookup, ...
	MidFrameTimes, FrameLengths, Ca_even, ts_even);

rR = conv_int1 ./ conv_int2;

% Generate K1 and K2 images

%=========================================================
if (progress); disp ('Calculating k2 image'); end
k2 = lookup(rR, k2_lookup, rL);

%=========================================================
if (progress); disp ('Calculating K1 image'); end
[im_size, num_im] = size(PET);
midframeCa = lookup (ts_even, Ca_even, MidFrameTimes);
w3 = sqrt (MidFrameTimes);
w2 = MidFrameTimes;

int_activity = ((w3.*midframeCa)'*FrameLengths) .* ...
                ((PET)*FrameLengths) - ...
                ((midframeCa)'*FrameLengths) .* ...
		((((ones(im_size,1)*w3').*PET)*FrameLengths));
k2_conv_ints = lookup (k2_lookup, conv_int1, k2);
K1 = int_activity ./ k2_conv_ints;

%=========================================================
if (progress); disp ('Calculating V0 image'); end

% Now calculate V0, using Eq. 26.  Note that the second term of the
% numerator is just K1 .* k2_conv_ints; however, K1 was calculate 
% from int_activity ./ k2_conv_ints.  Therefore just use int_activity
% for that term.  Also, the first term of the numerator -- M(t) 
% integrated across frames -- was calculated above for masking
% the slice.  The denominator is just Ca(t) integrated across
% frames, so we need to use the resampled Ca(t) -- midframeCa --
% for the integral.

V0 = (summed - int_activity) / (midframeCa' * FrameLengths0;

nuke = find (isnan (K1));
K1 (nuke) = zeros (size (nuke));
nuke = find (isinf (K1));
K1 (nuke) = zeros (size (nuke));

nuke = find (isnan (V0));
V0 (nuke) = zeros (size (nuke));
nuke = find (isinf (V0));
V0 (nuke) = zeros (size (nuke));

% Magic number time: convert the values to the correct units.
% k2 is currently expressed in units of s^-1 but should be expressed
% min^-1; K1 is in (nCi * sec * (g blood)) / ((mL tissue) * counts * sec)
% and should be (mL blood) / ((g tissue) * minute).  Since blood and
% tissue are both taken to be 1.05 g / mL, and 1 nCi = 37 count/sec, and
% of course 1 min = 60 sec, the conversion factor is 2013.605442176871
% NOT SURE ABOUT UNITS OF V0 RIGHT NOW!!!!!

k2 = k2 * 60;
K1 = K1 * 2013.605442176871;

% Cleanup

closeimage (img);





