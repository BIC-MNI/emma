function rcbf1 (filename)
% RCBF1  
%        a first try at a two-compartment rCBF model (without V0
%        or shifting) implemented as a MATLAB function.
%
%        rcbf1 (filename)

if (nargin ~= 1)
    help rcbf1
    error('Filename must be specified.');
end

img = openimage(filename);
FrameTimes = getimageinfo (img, 'FrameTimes');
FrameLengths = getimageinfo (img, 'FrameLengths');
MidFrameTimes = FrameTimes + (FrameLengths / 2);
PET = getimages (img, 1, 1:length(FrameTimes));

disp ('RCBF1: calling findrl');

rL = findrl (PET, MidFrameTimes, FrameLengths);
[k2, rr] = findrr (img, MidFrameTimes, FrameLengths);

keyboard

closeimage (img);
