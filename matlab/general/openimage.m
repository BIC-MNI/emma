function ImHandle = openimage (filename)
% openimage - sets up a MINC file and prepares for reading.
% Returns a "handle" to the MINC file and its associated image,
% for use by getnextimage, etc.

% Documentation for the returned handle (not necessary for the user to
% know, but handy all the same): ImHandle is a character matrix, so
% that each row consists of a string.  All strings are padded with
% however many spaces are necessary to make them all the same length;
% this will only need to be taken into account when using the
% filename.  The contents of the rows are as follows:
%
%   ImHandle(1,:) = the filename passed to openimage
%   ImHandle(2,:) = the name of the variable where images are stored
%   ImHandle(3,:) = the name of the variable containing the start time
%                   (in seconds, relative to the start of the study)
%                   of *every* frame in the file, NOT just the frames
%                   currently in memory.
%   ImHandle(4,:) = the name of the variable containing the length
%                   in seconds of every frame of the file
%   ImHandle(5,:) = the name of the variable containing the list of 
%                   frame numbers currently in memory (available via the
%                   variable named in row 2)
%   ImHandle(6,:) = the name of the variable containing the number of
%                   the currently available slice (slices?).
%   ImHandle(7,:) = the name of the variable containing the current
%                   location (image line number) for each frame of the
%                   file.  For frames not in memory, this variable
%                   will hold 0.
%                   


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Get the frame times and lengths for all frames.

FrameTimes = mireadvar (filename, 'time');
FrameLengths = mireadvar (filename, 'time-width');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Set up the variables.  (Note that we're currently hard-coding the 
% available slice and frame numbers; these will be referred and possibly
% added to by getnextline.)

CurLocs = zeros (length (FrameTimes), 1);
CurLocs (1) = 1;
AvailFrames = [1];
AvailSlices = 1;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Assume that we want to start at the beginning, so read first frame
% of the first slice.

PETimages = mireadimages (filename, 0, 0, 0);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Put all the variables into the global workspace so that they
% persist.

global PETimages FrameTimes FrameLengths AvailFrames AvailSlices CurLocs


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Create the handle itself.

len = max (length (filename), length ('FrameLengths'));
ImHandle = padstring (filename, len);
eval(['ImHandle = [ImHandle; padstring(' '''PETimages''' ',len)];']); 
eval(['ImHandle = [ImHandle; padstring(' '''FrameTimes''' ',len)];']); 
eval(['ImHandle = [ImHandle; padstring(' '''FrameLengths''' ',len)];']); 
eval(['ImHandle = [ImHandle; padstring(' '''AvailFrames''' ',len)];']); 
eval(['ImHandle = [ImHandle; padstring(' '''AvailSlices''' ',len)];']); 
eval(['ImHandle = [ImHandle; padstring(' '''CurLocs''' ',len)];']); 









