% RCBF1  a first try at a two-compartment rCBF model (without V0)
%        implemented as a MATLAB script with filename currently 
%        hardcoded. (sorry)

cd /usr/people/wolforth/matlab
img = openimage ('images/arnaud_20547.mnc');
FrameTimes = getimageinfo (img, 'FrameTimes');
FrameLengths = getimageinfo (img, 'FrameLengths');
MidFrameTimes = FrameTimes + (FrameLengths / 2);
PET = getimages (img, 1, 1:length(FrameTimes));

disp ('RCBF1: calling findrl');
keyboard
rL = findrl (images, MidFrameTimes, FrameLengths);


