function ImHandle = openimage (filename, mode)
% OPENIMAGE   setup appropriate variables in MATLAB for reading a MINC file
%
%   handle = openimage (filename)
% 
% Sets up a MINC file and prepares for reading.  This function creates
% all variables required by subsequent get/put functions such as
% getimages and putimages.  It also reads in various data about the
% size and number of images on the file, all of which can be queried
% via getimageinfo.
%  
% If the file in question is compressed (i.e., it ends with `.z',
% `.gz', or `.Z', then openimage will transparently uncompress it to a
% uniquely named temporary directory.  The filename returned by
% getimageinfo (handle, 'filename') in this case will be the name of
% the temporary, uncompressed file.  When the file is closed with
% closeimage, this temporary file (and its directory) will be deleted.
% 
% The value returned by openimage is a handle to be passed to
% getimages, putimages, getimageinfo, etc.
% 
% Note that by default you cannot use putimages to write data into a
% file opened with openimage.  This differs from the behaviour of
% previous versions of EMMA.  However, this can be overridden by
% supplying a `mode' description when you open the file.  In
% particular,
%
%    openimage (filename, 'w')
% 
% emulates the old behaviour of EMMA: you can use the image handle
% returned by openimage here to either read from or write to the file.
% However, use of this feature should be strongly avoided, as it means
% an image volume can be modified with no backup copy and no record of
% the changes made.  When you wish to write data to a MINC volume, you
% should always create a new volume using newimage.

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
%              function to access the per-handle global variables.
%              
%              This convention is followed for the variables Filename,
%              NumFrames, NumSlices, ImageSize, FrameTimes,
%              FrameLengths, and Flags.
%              
%              The functions getimages, putimages, getimageinfo,
%              viewimage, getblooddata, check_sf, and closeimage also
%              follow this convention for retrieving/storing data in
%              these global variables.
%              
%              Note that in the documentation for all of these
%              functions, we will use the convention (e.g.) Filename#
%              to refer to the "instance" of those (or other) image
%              variables associated with the current handle.
%@GLOBALS    : reads/increments: ImageCount
%              creates: Filename#, DimSizes#, FrameTimes#, FrameLengths#
%@CALLS      : mireadvar (CMEX)
%              miinquire (CMEX)
%@CREATED    : June 1993, Greg Ward & Mark Wolforth
%@MODIFIED   : 93-7-29, Greg Ward: added calls to miinquire, took out
%              mincinfo and length(various variables) to determine
%              image sizes.
%              95-4-12 - 4/20, Greg Ward: changed to handle compressed
%                              files and the new Flags# global variable
%              97-5-27 Mark Wolforth: Minor modification to work with
%                                     Matlab 5, which handles global
%                                     variables differently from Matlab 4.x
%-----------------------------------------------------------------------------


% Note that the effects of 'global' on a brand-new variable are subtly
% different between MATLAB 4 and 5.  In both cases, 'global foo' makes
% 'foo' spring into existence as an empty (0x0) matrix.  However, in
% MATLAB 4, exist ('foo') would still return false; in retrospect, this
% doesn't make a lot of sense, and it was a mistake to rely on this
% behaviour (as openimage used to do).  MATLAB 5 is more sensible: after
% 'foo' is sprung into existence by 'global foo', exist ('foo') is true.
% In both versions, isempty (foo) is true, as expected -- when a
% variable is created by 'globale', it is indeed an empty matrix.

global ImageCount



error (nargchk (1, 2, nargin));

% disp (['Looking for ' filename]);
if exist (filename) ~= 2
   error ([filename ': file not found']);
end

% Initialize the flags for this volume.  Flags(1) is "read-write", 
% Flags(2) is "compressed".

Flags = [0 0];

% Did the caller supply a `mode' argument?  Then check to see if it's
% 'w', and if so, set the "read-write" flag.

if (nargin > 1)
   if (~isstr (mode) | length(mode) ~= 1)
      error ('mode must be a string of length 1');
   end
   if (mode == 'w')
      Flags(1) = 1;
   elseif (mode == 'r')
      Flags(1) = 0;
   else
      error (['Illegal mode: ' mode]);
   end
end
      

% Check to see if it's a compressed file, and if so uncompress
% (and give it a new filename)

len = length (filename);
if (strcmp (filename(len-2:len), '.gz') | ...
    strcmp (filename(len-1:len), '.z') | ...
    strcmp (filename(len-1:len), '.Z'))

   Flags(2) = 1;
   if (Flags(1))
      error (['Cannot open compressed files for writing']);
   end
   
   % Parse the filename (strip off directory and last extension)

   dots = find (filename == '.');
   lastdot = dots (length (dots));
   slashes = find (filename == '/');
   if (isempty(slashes))
      lastslash = 0;
   else
      lastslash = slashes (length (slashes));
   end
   
   % Create a (hopefully) unique temporary directory -- only way
   % there'll be a clash is if another MATLAB does an openimage
   % on a compressed file within the same second as this one.
   % Note that checking the directory with fopen might not be
   % portable!  Works on IRIX and SunOS, at least.
   
   timestring = sprintf ('%02d', fix (clock));
   tdir = [tempdir 'emma' timestring];
   id = fopen (tdir, 'r');		% try to open the temp dir
   if (id ~= -1)			% if it succeeded, that's bad! means
      fclose (id);			% the dir already exists
      error (['Temporary directory ' tdir ' already exists']);
   end
   status = unix (['mkdir ' tdir]);
   if (status ~= 0)                     % mkdir failed
      error (['Unable to create temporary directory ' tdir]);
   end
   
   % Now generate the name of the temporary file, and uncompress to it.
   % If the file already exists, that's an internal error -- we
   % shouldn't make it past the directory check above!
   
   newname = [tdir '/' filename((lastslash+1):(lastdot-1))];
   if (exist (newname) ~= 2)
      fprintf ('(uncompressing...');
      status = unix (['gunzip -c ' filename ' > ' newname]);
      if (status ~= 0)
	 error (['Error trying to uncompress file ' filename]);
      end
      fprintf (')\n');
   else
      error (['INTERNAL ERROR - file ' newname ' exists in new directory?!?']);
   end

   filename = newname;
end
   
   
% Get the current directory if filename only has a relative path, tack
% it onto filename, and make sure filename exists.

if (filename (1) ~= '/')
   curdir = pwd;
%   curdir = mexec ('pwd');
   curdir (find (curdir == 10)) = [];        % strip out newline
   filename = [curdir '/' filename];
end

   
% The file exists, so we will be opening it... so figure out the handle.

if ~isempty (ImageCount)
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
eval(['global Flags',       int2str(ImageCount)]);

eval(['Filename'     int2str(ImageCount) ' = filename;']);
eval(['FrameTimes'   int2str(ImageCount) ' = FrameTimes;']);
eval(['FrameLengths' int2str(ImageCount) ' = FrameLengths;']);
eval(['DimSizes'     int2str(ImageCount) ' = DimSizes;']);
eval(['Flags'        int2str(ImageCount) ' = Flags;']);

ImHandle = ImageCount;
