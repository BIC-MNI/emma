function ImHandle = openimage (filename)
%  handle = openimage (filename)
% sets up a MINC file and prepares for reading.
% Returns a "handle" to the MINC file and its associated image,
% for use by getnextimage, etc.

global ImageCount MAX_FRAMES     % this does NOT create the variable if it
                                 % doesn't exist yet!
MAX_FRAMES = 30;                 % Size of the cache expressed in images
if exist ('ImageCount') == 1
   ImageCount = ImageCount + 1;
else
   ImageCount = 1;
end

if exist (filename) ~= 2
	error ([filename ': file not found']);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Get the frame times and lengths for all frames.

[tmp, ImageSize] = mcdinq (filename, 'xspace');		% assumes square images!!!
[tmp, NumSlices] = mcdinq (filename, 'zspace');
[tmp, NumFrames] = mcdinq (filename, 'time');

if isempty (NumFrames)
	NumFrames = 0;
	disp ('Study is non-dynamic');
else
	FrameTimes = mireadvar (filename, 'time');
	FrameLengths = mireadvar (filename, 'time-width');
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Set up the other variables.  (Note that we're currently hard-coding
% the initially available slice and frame numbers; these will be
% referred and possibly added to by getnextline.)
CurLine = 1;

AvailFrames = [];
AvailSlices = [];

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
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
eval(['CurLine'      int2str(ImageCount) ' = CurLine;']);
eval(['AvailFrames'  int2str(ImageCount) ' = AvailFrames;']);
eval(['AvailSlices'  int2str(ImageCount) ' = AvailSlices;']);
eval(['PETimages'    int2str(ImageCount) ' = [];']);

ImHandle = ImageCount;
