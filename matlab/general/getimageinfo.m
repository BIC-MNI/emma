function info = getimageinfo (handle, what)
% GETIMAGEINFO   retrieve helpful trivia about an open image
% info = getimageinfo (handle, what)
%
% Get some information about an open image.  what is a string that
% describes what you want to know about.  The first possibility is one
% of the standard MINC image dimensions ('time', 'zspace', 'yspace',
% 'xspace') which will return the length of their respective
% dimensions.  'time' and 'zspace' are equivalent to the special (and
% somewhat more intuitive) strings 'NumFrames' and 'NumSlices'.  Other
% non-MINC-standard strings, and the values returned by getimageinfo
% when they are passed, are:
%
%      Filename       the name of the MINC file (if applicable) as
%                     as supplied to openimage or newimage
%      NumFrames      number of frames in the study, 0 if non-dynamic
%                     study (equivalent to 'time')
%      NumSlices      number of slices in the study (equivalent to 'zspace')
%      ImageSize      size of a single image (eg. 128, 256) (equivalent
%                     to both 'xspace' and 'yspace')
%      FrameLengths   vector with NumFrames elements - duration of each frame
%                     in the study, in seconds
%      FrameTimes     vector with NumFrames elements - start time of each 
%                     frame, relative to start of study, in seconds
%      
% If the requested data item is invalid, info will be empty.

% Created: 93-6-17, Greg Ward
% Modified: 93-6-25, Greg Ward: added the standard MINC dimension names,
% and spruced up the help.

if nargin ~= 2
   error ('Incorrect number of arguments');
end

if length(handle) ~= 1
   error ('handle must be a scalar');
end

if ~isstr(what)
   error ('what must be a string');
end

% Make global the three image-size variables, and also the (possibly)
% named value.  This may be reduntant, eg. if what=='NumFrames'.  But
% it assures that everything we could possible need global is global
% before calling exist or eval with it.

eval(['global NumFrames' int2str(handle)]);
eval(['global NumSlices' int2str(handle)]);
eval(['global ImageSize' int2str(handle)]);
eval(['global ' what int2str(handle)])

% If 'what' doesn't exist as a variable, try interpreting it as a dimension
% name and retrieving the associated variable.  Else just get the 
% (numbered) variable.

if exist ([what int2str(handle)]) ~= 1
	if (strcmp (what, 'time'))				% MINC dimension name, so get dim size
		info = eval (['NumFrames' int2str(handle)]);
	elseif (strcmp (what, 'zspace'))		% ditto - another dimension name
		info = eval (['NumSlices' int2str(handle)]);
	elseif (strcmp (what, 'xspace') | strcmp (what, 'yspace'))
		info = eval (['ImageSize' int2str(handle)]);
	else
		disp (['Variable/dimension not found: ' what int2str(handle)]);
		info = [];
	end

else
   info = eval([what int2str(handle)]);
end
