function [k1,k2] = rcbf1 (filename, slice)
% RCBF1  
%        a first try at a two-compartment rCBF model (without V0
%        or shifting) implemented as a MATLAB function.
%
%        [k1,k2] = rcbf1 (filename, slice)

% Input argument checking

if (nargin ~= 2)
    help rcbf1
    error('Incorrect number of arguments.');
end

if (length(slice)~=1)
    help rcbf1
    error('<Slice> must be a scalar.');
end

% Input arguments are checked, so now we can do some REAL work.

img = openimage(filename);
FrameTimes = getimageinfo (img, 'FrameTimes');
FrameLengths = getimageinfo (img, 'FrameLengths');
MidFrameTimes = FrameTimes + (FrameLengths / 2);
PET = getimages (img, slice, 1:length(FrameTimes));

rL = findrl (PET, MidFrameTimes, FrameLengths);
[k2_lookup, rR, Ca_even, ts_even] = findrr (img, MidFrameTimes, FrameLengths);

% Generate K1 and K2 images

k2 = lookup(rR, k2_lookup, rL);
k1 = zeros(length(PET),1);


% Cleanup

closeimage (img);
