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
%  The value returned by openimage is a handle to be passed to getimages,
%  putimages, getimageinfo, etc.

% ------------------------------ MNI Header ----------------------------------
%@NAME       : openimage
%@INPUT      : filename - name of MINC file to open
%@OUTPUT     : 
%@RETURNS    : handle - for use with other image functions (eg. getimages,
%              putimages, getimageinfo, etc.)
%@DESCRIPTION: Prepares for reading/writing a MINC file from within
%              MATLAB by generating a handle and creating a number of
%              global variables for use by getimages, putimages, etc.
%@METHOD     : (Note: none of this needs to be known by the end user.  It
%              is only here to document the inner workings of the
%              open/get/put/close image functions.)  
%
%              Increments the global variable ImageCount, and uses the
%              new ImageCount as a handle to the image.  Handles are
%              simply integers that are appended to various names to
%              give the names of various global variables; eg., the
%              global variable Filename3 is the name of the MINC file
%              tagged by the handle 3.  This appending is universally
%              done by the MATLAB string concatenation: eg., for
%              handle=3, ['Filename' int2str(handle)] yields
%              Filename3.  This is frequently combined with the eval
% 
%              This convention is followed for the variables Filename,
%              NumFrames, NumSlices, ImageSize, PETimages, FrameTimes,
%              FrameLengths, AvailFrames, AvailSlices, and CurLine.
%              Note that not all of these variables are currently
%              used; some of them are meant for a line-by-line image
%              retrieval/storage system (coming Real Soon Now), rather
%              than the currently implemented and rather memory
%              intensive image-by-image system.
%
%              The functions getimages, putimages, getimageinfo,
%              viewimage, getblooddata, check_sf, and closeimage also
%              follow this convention for retrieving/storing data in
%              these global variables.
%
%              Note that in the documentation for all of these
%              functions, we will use the convention Filename# or
%              PETimages# to refer to the "instance" of those (or
%              other) image variables associated with the current
%              handle.
%@GLOBALS    : reads/increments: ImageCount
%              creates: Filename#, DimSizes#, FrameTimes#, FrameLengths#
%@CALLS      : mireadvar (CMEX)
%              miinquire (CMEX)
%@CREATED    : June 1993, Greg Ward & Mark Wolforth
%@MODIFIED   : 93-7-29, Greg Ward: added calls to miinquire, took out
%              mincinfo and length(various variables) to determine
%              image sizes.
%-----------------------------------------------------------------------------


global ImageCount                % this does NOT create the variable if it
                                 % doesn't exist yet!

if (nargin ~= 1)
   error ('Incorrect number of arguments');
end

% Get the current directory if filename only has a relative path, tack
% it onto filename, and make sure filename exists.

if (filename (1) ~= '/')
   curdir = exec ('pwd');
   curdir (find (curdir == 10)) = [];        % strip out newline
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

% Get sizes of ALL possible image dimensions. Time/frames, slices, 
% height, width will be the elements of DimSizes where height and
% width are the two image dimensions.  DimSizes WILL have four 
% elements; if any of the dimensions do not exist, the corresponding
% element of DimSizes will be zero.  (See also miinquire documentation
% ... when it exists!)

DimSizes = miinquire (filename, 'imagesize');

NumFrames = DimSizes (1);
NumSlices = DimSizes (2);
Height = DimSizes (3);
Width = DimSizes (4);

% Get the frame times and lengths for all frames.  Note that mireadvar
% returns an empty matrix for non-existent variables, so we don't need
% to check the dimensions of the file.

FrameTimes = mireadvar (filename, 'time');
FrameLengths = mireadvar (filename, 'time-width');

% Now make "numbered" copies of the four variables we just created
% (Filename, DimSizes, FrameTimes, and FrameLengths); the number used
% is ImageCount, and the effect of the following statements is to
% declare these numbered variables as global (so other functions can
% access them) and to copy the local data to the global variables.

eval(['global Filename'     int2str(ImageCount)]);
eval(['global DimSizes'     int2str(ImageCount)]);
eval(['global FrameTimes'   int2str(ImageCount)]);
eval(['global FrameLengths',int2str(ImageCount)]);

%eval(['global NumFrames',   int2str(ImageCount)]);
%eval(['global NumSlices',   int2str(ImageCount)]);
%eval(['global ImageSize',   int2str(ImageCount)]);
%eval(['global PETimages'    int2str(ImageCount)]);
%eval(['global AvailFrames', int2str(ImageCount)]);
%eval(['global AvailSlices', int2str(ImageCount)]);
%eval(['global CurLine',     int2str(ImageCount)]);

eval(['Filename'     int2str(ImageCount) ' = filename;']);
eval(['FrameTimes'   int2str(ImageCount) ' = FrameTimes;']);
eval(['FrameLengths' int2str(ImageCount) ' = FrameLengths;']);
eval(['DimSizes'     int2str(ImageCount) ' = DimSizes;']);

%eval(['NumFrames'    int2str(ImageCount) ' = NumFrames;']);
%eval(['NumSlices'    int2str(ImageCount) ' = NumSlices;']);
%eval(['ImageSize'    int2str(ImageCount) ' = ImageSize;']);
%eval(['CurLine'      int2str(ImageCount) ' = 1;']);
%eval(['AvailFrames'  int2str(ImageCount) ' = [];']);
%eval(['AvailSlices'  int2str(ImageCount) ' = [];']);
%eval(['PETimages'    int2str(ImageCount) ' = [];']);

ImHandle = ImageCount;
