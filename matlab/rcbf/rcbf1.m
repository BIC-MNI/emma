function [K1,k2] = rcbf1 (filename, slice, progress)
% RCBF1 a one-compartment (double-weighted integral) rCBF model.
%
%        [K1,k2] = rcbf1 (filename, slice)
%
% A one-compartment rCBF model (without V0 or blood delay and 
% dispersion) implemented as a MATLAB function.  The
% compartmental equation is solved by integrating it across
% the entire study, and then weighting this integral with two
% different weights.  When these two integrals are divided by
% each other, K1 is eliminated, leaving only k2.  A lookup
% table is calculated, relating values of k2 to values of the
% integral.  From this, k2 and be calculated.  From k2, K1 is
% easily found by substitution into the original compartmental
% equation.  See the document "rCBF Analysis Using Matlab" for
% further details of both the compartmental equations
% themselves, and the method of solution.

% ----------------------------- MNI Header -----------------------------------
% @NAME       : rcbf1
% @INPUT      : 
% @OUTPUT     : 
% @RETURNS    : 
% @DESCRIPTION: 
% @METHOD     : 
% @GLOBALS    : 
% @CALLS      : 
% @CREATED    : 
% @MODIFIED   : 
% @COPYRIGHT  :
%             Copyright 1993 Mark Wolforth and Greg Ward, McConnell Brain
%             Imaging Centre, Montreal Neurological Institute, McGill
%             University.
%             Permission to use, copy, modify, and distribute this
%             software and its documentation for any purpose and without
%             fee is hereby granted, provided that the above copyright
%             notice appear in all copies.  The author and McGill University
%             make no representations about the suitability of this
%             software for any purpose.  It is provided "as is" without
%             express or implied warranty.
%
% ---------------------------------------------------------------------------- */


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
g_even = g_even*XCAL*37/1.05;           % units are decay / (g_tissue * sec)

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



