function info = getimageinfo (handle, what)
% info = getimageinfo (handle, what)
%
% Get some information about an open image.  what is a string that describes
% what you want to know about, possible values (and what they return) are
% currently:
%      Filename       character string - supplied to openimage
%      NumFrames      number of frames in the study, 0 if non-dynamic study
%      NumSlices      number of slices in the study
%      ImageSize      size of a single image (eg. 128, 256)
%      FrameLengths   vector with NumFrames elements - duration of each frame
%                     in the study, in seconds
%      FrameTimes     vector with NumFrames elements - start time of each 
%                     frame, relative to start of study, in seconds
%      
% If the requested data item is invalid, info will be empty.

% Created: 93-6-17, Greg Ward

if nargin ~= 2
   error ('Incorrect number of arguments');
end

if length(handle) ~= 1
   error ('handle must be a scalar');
end

if ~isstr(what)
   error ('what must be a string');
end

eval(['global ' what int2str(handle)])
if exist ([what int2str(handle)]) ~= 1
   info = [];
else
   info = eval([what int2str(handle)]);
end
