function ImHandle = openimage (filename)
% OPENIMAGE   setup appropriate variables in MATLAB for reading a MINC file
%
%  handle = openimage (filename)
%
%  Sets up a MINC file and prepares for reading.  This function
%  creates all variables required by subsequent get/put functions such
%  as getimages and putimages.  It also reads in various data about
%  the size and number of images on the file, all of which can be
%  queried via getimageinfo.
%
%  The value returned by openimage is a handle to be passed to getimages
%  putimages, getimageinfo, etc.

global ImageCount MAX_FRAMES     % this does NOT create the variable if it
                                 % doesn't exist yet!
MAX_FRAMES = 30;                 % Size of the cache expressed in images

if (nargin ~= 1)
	error ('Incorrect number of arguments');
end

% Get the current directory if filename only has a relative path, tack
% it onto filename, and make sure filename exists.

if (filename (1) ~= '/')
	curdir = pwd;
	curdir (find (curdir == 10)) = [];			% strip out newline
	filename = [curdir '/' filename];
end

% disp (['Looking for ' filename]);
if exist (filename) ~= 2
	error ([filename ': file not found']);
end

% The file exists, so we will be opening it... so figure out the handle.

if exist ('ImageCount') == 1
   ImageCount = ImageCount + 1;
else
   ImageCount = 1;
end

% Get the frame times and lengths for all frames.  Note that mireadvar
% returns an empty matrix for non-existent variables, so we don't need
% to check the dimensions of the file.

FrameTimes = mireadvar (filename, 'time');
FrameLengths = mireadvar (filename, 'time-width');
Zspace = mireadvar (filename, 'zspace');

NumFrames = length (FrameTimes);
NumSlices = length (Zspace);

% Now call mincinfo to get the image size, which is just the length
% of the x (or y - assuming they're the same!!!) dimension

[res, out] = unix (['mincinfo -dimlength xspace -error_string "" ' filename]);
ImageSize = sscanf (out, '%d');

% Now make "numbered" copies of the six variables we just created; the
% number used is ImageCount, and the effect of the following
% statements is to declare these numbered variables as global (so
% other functions can access them) and to copy the local data to the
% global variables.

eval(['global Filename'     int2str(ImageCount)]);
eval(['global NumFrames',   int2str(ImageCount)]);
eval(['global NumSlices',   int2str(ImageCount)]);
eval(['global ImageSize',   int2str(ImageCount)]);
eval(['global PETimages'    int2str(ImageCount)]);
eval(['global FrameTimes'   int2str(ImageCount)]);
eval(['global FrameLengths',int2str(ImageCount)]);
eval(['global AvailFrames', int2str(ImageCount)]);
eval(['global AvailSlices', int2str(ImageCount)]);
eval(['global CurLine',     int2str(ImageCount)]);

eval(['Filename'     int2str(ImageCount) ' = filename;']);
eval(['NumFrames'    int2str(ImageCount) ' = NumFrames;']);
eval(['NumSlices'    int2str(ImageCount) ' = NumSlices;']);
eval(['ImageSize'    int2str(ImageCount) ' = ImageSize;']);
eval(['FrameTimes'   int2str(ImageCount) ' = FrameTimes;']);
eval(['FrameLengths' int2str(ImageCount) ' = FrameLengths;']);
eval(['CurLine'      int2str(ImageCount) ' = 1;']);
eval(['AvailFrames'  int2str(ImageCount) ' = [];']);
eval(['AvailSlices'  int2str(ImageCount) ' = [];']);
eval(['PETimages'    int2str(ImageCount) ' = [];']);

ImHandle = ImageCount;
