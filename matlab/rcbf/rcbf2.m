function [K1,k2,V0,delta] = rcbf2 (filename, slice, progress, correction)

% RCBF2 a two-compartment (triple-weighted integral) rCBF model.
%
%       [K1,k2,V0,delta] = rcbf2 (filename, slice)
% 
% rcbf2 implements the three-weighted integral method of calculating
% k2, K1, and V0 (in that order) for a particular slice.  This
% function also returns the delay value calculated for blood
% correction.  It first reads in a great mess of data (viz., the brain
% activity for every frame of the slice, frame start times and
% lengths, plasma activity, and blood sample times).  Then, a simple
% mask is created and used to filter out roughly all points outside
% the head.
% 
% The actual calculations follow the procedure outlined in the
% document "RCBF Analysis Using Matlab".  Occasionally, comments in
% the source code or documentation for various functions involved in
% the analysis will refer to equations in this document.  The most
% relevant functions in this respect are rcbf2 itself, correctblood
% and findintconvos.
% 
% The starting point of the three-weighted integration method is Eq.
% 10 of the RCBF document.  The left hand side of this equation, rL,
% is calculated for every pixel.  Then, a lookup table relating a
% series of k2 values to the rR (the right-hand side of Eq. 10) is
% calculated.  This lookup table should have a few hundred elements,
% as calculating rR is considerably more expensive than calculating
% rL.  Since rL and rR are equal, we use the pixel-wise values of rL
% to lookup k2 for every pixel.
% 
% Then, Eq. XXX is used to calculate K1.  This requires calculating
% the moderately complicated int_activity (left hand side of Eq. XXX)
% and the extremely complicated k2_conv_ints (right hand side).
% However, the expression for k2_conv_ints appeared already in the
% numerator of rR, so we preserve that lookup table as conv_int1 and
% use it to lookup k2_conv_ints.  These two long vectors (with one
% number for every pixel) are then divided to get K1.  Finally, V0 is
% calculated via Eq. YYY.


% Input argument checking

if (nargin < 2)
   help rcbf2
   error ('Not enough input arguments');
elseif (nargin < 3)
   progress = 0;
elseif (nargin < 4)
   correction = 1;
elseif (nargin > 4)
   help rcbf2
   error('Incorrect number of arguments.');
end

if (length(slice)~=1)
   help rcbf2
   error('<Slice> must be a scalar.');
end

% Input arguments are checked, so now we can do some REAL work, ie.
% read in all the data we need.  FrameTimes, FrameLengths, and 
% MidFTimes should be self-explanatory.  Ca_even is the blood
% activity resampled at some evenly-spaced time domain; ts_even
% is the actual points of that domain.  Ca_mft is the blood activity
% integrated frame-by-frame (ie. the average of Ca over every frame).

if (progress); disp ('Reading image information and generating mask'); end
img = openimage(filename);
FrameTimes = getimageinfo (img, 'FrameTimes');
FrameLengths = getimageinfo (img, 'FrameLengths');
MidFTimes = FrameTimes + (FrameLengths / 2);
[g_even, ts_even] = resampleblood (img, 'even');


PET = getimages (img, slice, 1:length(FrameTimes));
PET = PET * 37 / 1.05;                  % convert to decay / (g_tissue * sec)

% Create the weighting functions (N.B. w1 is just one, so don't bother),
% and calculate the three weighted integrals of PET.
w2 = MidFTimes;
w3 = sqrt (MidFTimes);

ImLen = size(PET,1);
PET_int1 = C_trapz (MidFTimes, PET')';
PET_int2 = C_trapz (MidFTimes, PET' .* (w2 * ones(1,ImLen)))';
PET_int3 = C_trapz (MidFTimes, PET' .* (w3 * ones(1,ImLen)))';

% Now use PET_int1 to create a simple mask, and mask all three PET integrals.
% This does a good job of removing the outside-of-head data for CBF studies.

mask = PET_int1 > mean(PET_int1);
PET_int1 = PET_int1 .* mask;
PET_int2 = PET_int2 .* mask;
PET_int3 = PET_int3 .* mask;


% Use getmask to interactively create a threshold mask, and then perform
% delay/dispersion correction.

% Apply the cross-calibration factor.
XCAL = 0.11;
g_even = g_even*XCAL*37;              % units are decay / (g_tissue * sec)

if (correction)
   mask = getmask (PET_int1);
   A = (mean (PET (find(mask),:)))';
   [ts_even, Ca_even, delta] = correctblood ...
                       (A, FrameTimes, FrameLengths, g_even, ts_even, progress);
else
   Ca_even = g_even;
end


% Pick values of k2 and then calculate rR for every one.  (Ie. create
% the lookup table).  This is done by first calculating various
% weighted integrals (conv_int{1,2,3} and Ca_int{1,2,3}) which are
% in turn used to calculate lots of other RCBF parameters.  Here 
% we use them specifically to find rR.  NB. conv_int{1,2,3} are tables,
% with one value for every k2 in k2_lookup.  However, Ca_int{1,2,3} are
% simply scalars.

if (progress); disp ('Generating k2/rR lookup table'); end
k2_lookup = (-10:0.05:10) / 60;
[conv_int1,conv_int2,conv_int3] = findintconvo (Ca_even,ts_even,k2_lookup,...
      MidFTimes, FrameLengths, 1, w2, w3);


Ca_mft = nframeint (ts_even, Ca_even, FrameTimes, FrameLengths);      

Ca_int1 = C_trapz(MidFTimes, Ca_mft);
Ca_int2 = C_trapz(MidFTimes, (w2 .* Ca_mft));
Ca_int3 = C_trapz(MidFTimes, (w3 .* Ca_mft));

% Find the value of rL for every pixel of the slice.
rL = ((Ca_int3 .* PET_int1) - (Ca_int1 .* PET_int3)) ./ ...
     ((Ca_int3 .* PET_int2) - (Ca_int2 .* PET_int3));

rR = ((Ca_int3 * conv_int1) - (Ca_int1 * conv_int3)) ./ ...
      ((Ca_int3 * conv_int2) - (Ca_int2 * conv_int3));

% Now, we must have the k2/rR lookup table in order by rR; however, 
% we also want to keep k2_lookup in the original order.  This
% is because the first lookup uses rL as a lookup into rR to
% find k2 (which requires that rR be monotonic, ie. sorted) whereas
% subsequent lookups all use k2 to find conv_int{1,2,3} -- which 
% requires that k2_lookup be monotonic.  So k2_lookup will be the
% list of k2's in order, and k2_sorted will be the same list, but 
% in order according to the sorted rR.

[rR,sort_order] = sort (rR);
k2_sorted = k2_lookup (sort_order);

% Generate K1 and K2 images

%=========================================================
if (progress); disp ('Calculating k2 image (via table lookup)'); end
k2 = lookup(rR, k2_sorted, rL);


%=========================================================
if (progress); disp ('Calculating K1 image'); end

% Note that PET_int1 = PET integrated across frames, with weighting
% function w1 = ones.

K1_numer = ((Ca_int3*PET_int1) - (Ca_int1 * PET_int3));
K1_denom = (Ca_int3 * lookup(k2_lookup,conv_int1,k2)) - ...
           (Ca_int1 * lookup(k2_lookup,conv_int3,k2));
K1 = K1_numer ./ K1_denom;

%=========================================================
if (progress); disp ('Calculating V0 image'); end

% Now calculate V0, using Eq. 26.  Note that the second term of the
% numerator is just K1 .* k2_conv_ints; however, K1 was calculate 
% from int_activity ./ k2_conv_ints.  Therefore just use int_activity
% for that term.  Also, the first term of the numerator -- M(t) 
% integrated across frames -- was calculated above for masking
% the slice.  The denominator is just Ca(t) integrated across
% frames, so we need to use the resampled Ca(t) -- Ca_mft --
% for the integral.

V0 = (PET_int1 - (K1 .* lookup(k2_lookup,conv_int1,k2))) / Ca_int1;


nuke = find (isnan (K1));
K1 (nuke) = zeros (size (nuke));
nuke = find (isinf (K1));
K1 (nuke) = zeros (size (nuke));

nuke = find (isnan (V0));
V0 (nuke) = zeros (size (nuke));
nuke = find (isinf (V0));
V0 (nuke) = zeros (size (nuke));

% Cleanup

closeimage (img);





