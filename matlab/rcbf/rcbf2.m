function [K1,k2,V0] = rcbf2 (filename, slice, progress)

% RCBF2  a two-compartment (triple-weighted integral) rCBF model.
%
%                [K1,k2,V0] = rcbf2 (filename, slice)
%
% rcbf2 implements the three-weighted integral method of calculating k2,
% K1, and V0 (in that order) for a particular slice.  It first reads in
% a great mess of data (viz., the brain activity for every frame of the
% slice, frame start times and lengths, blood sample activity, and blood
% sample times).  Then, a simple mask is created and used to filter out
% roughly all points outside the head.
%
% The actual calculations follow the procedure outlined in Ohta et. al.
% (see the references section in the document "RCBF Analysis Using
% Matlab").  Thus, all equation numbers cited in this M-file refer to
% that paper.
%
% Starting with Eq. 23, we calculate the ratio on the left-hand side,
% rL, for every pixel.  Then, we generate lookup table for the
% right-hand ratio, rL, for a series of k2 values.  The calculated rL
% values are used to lookup in this table (since rL == rR) to find k2
% for every pixel.  Then, Eq. 24 is used to calculate K1.  This 
% requires calculating the moderately complicated int_activity (left
% hand side of Eq. 24) and the extremely complicated k2_conv_ints
% (right hand side).  However, the expression for k2_conv_ints
% appeared already in the numerator of rR, so we preserve that lookup
% table as conv_int1 and use it to lookup k2_conv_ints.  These
% two long vectors (with one number for every pixel) are then
% divided to get K1.  Finally, V0 is calculated via Eq. 26.


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
% read in all the data we need.  FrameTimes, FrameLengths, and 
% midftimes should be self-explanatory.  Ca_even is the blood
% activity resampled at some evenly-spaced time domain; ts_even
% is the actual points of that domain.  Ca_mft is the blood activity
% resampled at mid-frame times (needed for integrating).

if (progress); disp ('Reading image information and generating mask'); end
img = openimage(filename);
FrameTimes = getimageinfo (img, 'FrameTimes');
FrameLengths = getimageinfo (img, 'FrameLengths');
midftimes = FrameTimes + (FrameLengths / 2);
[g_even, ts_even] = resampleblood (img, 'even');
g_even = g_even * 1.05;                 % convert to decay / (mL_blood * sec)

PET = getimages (img, slice, 1:length(FrameTimes));
PET = PET * 37 / 1.05;                  % convert to decay / (g_tissue * sec)

% Perform dispersion and delay correction.
PET_int1 = trapz( midftimes, PET')';

mask = getmask (PET_int1);
mask = (PET_int1>(1.8*mean(PET_int1)));
A = (mean (PET (find(mask),:)))' * 37 / 1.05;
[Ca_even, delta] = correctblood (A, FrameTimes, FrameLengths, g_even, ts_even, progress);

% Initialise the weighting functions w3 and w2; 
% w3=sqrt(midftimes) and w2=midftimes. 

w2 = midftimes;
w3 = sqrt (midftimes);

ImLen = size(PET,1);
PET_int2 = trapz (MidFTimes, PET' .* (w2 * ones(1,ImLen)))';
PET_int3 = trapz (MidFTimes, PET' .* (w3 * ones(1,ImLen)))';

% Apply a simple mask to eliminate data outside of the brain.

mask = PET_int1 > mean(PET_int1);
PET_int1 = PET_int1 .* mask;
PET_int2 = PET_int2 .* mask;
PET_int3 = PET_int3 .* mask;


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
      midftimes, FrameLengths, 1, w2, w3);


Ca_mft = frameint (ts_even, Ca_even, FrameTimes, FrameLengths);      
      
      
Ca_int1 = Ca_mft' * FrameLengths;
Ca_int2 = (w2 .* Ca_mft)' * FrameLengths;
Ca_int3 = (w3 .* Ca_mft)' * FrameLengths;

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

V0 = ((PET_int1) - (K1 .* lookup(k2_lookup,conv_int1,k2))) / Ca_int1;
% V0 = (PET_int1 - int_activity) / (Ca_mft' * FrameLengths);

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





