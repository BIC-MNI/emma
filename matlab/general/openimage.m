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

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Get the frame times and lengths for all frames.

FrameTimes = mireadvar (filename, 'time');
FrameLengths = mireadvar (filename, 'time-width');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Set up the other variables.  (Note that we're currently hard-coding
% the initially available slice and frame numbers; these will be
% referred and possibly added to by getnextline.)
CurLine = 1;

AvailFrames = [1];
AvailSlices = [1];
[tmp, ImageSize] = mcdinq (filename, 'xspace');		% assumes square images!!!
[tmp, NumSlices] = mcdinq (filename, 'zspace');
[tmp, NumFrames] = mcdinq (filename, 'time');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Assume that we want to start at the beginning, so read first frame
% of the first slice.  Note the "-1" because we will do everything in
% MATLAB with 1-based array indeces, but the CMEX functions want
% things 0-based.

PETimages = mireadimages (filename, AvailSlices-1, AvailFrames-1, 0);


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
eval(['PETimages'    int2str(ImageCount) ' = PETimages;']);

ImHandle = ImageCount;
