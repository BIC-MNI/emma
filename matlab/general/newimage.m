function handle = newimage (new_file, parent_image, numslices, numframes)
% NEWIMAGE   create a new image and possibly an associated MINC file.
%
% handle = newimage (new_file, parent_file, numslices, numframes)
%
%  Creates a new image.  If the character string new_file is not
%  empty, then a MINC file is also created; data will be written to
%  the MINC file at some stage.  (It is possible that it will not be
%  written until closeimage is called, so be extra sure to call
%  closeimage!)  Otherwise, data will simply be accumulated in
%  memory -- it can be queried via getimages and getnextline if
%  desired.  (Note that if data is deposited using putnextline, then
%  you should call rewindimage or setimageline before using
%  getnextline, as getnextline will attempt to return data that has
%  not yet been written!)
%
%  parent_file can be either the name of a MINC file or a handle to an
%  image already open within MATLAB.  In the former case (which only
%  applies if you are creating a new MINC file by supplying new_file),
%  the "parent" MINC file is used as a skeleton to create the new MINC
%  file.  See micreate and micreateimages for more information.  All
%  you need to know to work with the new image in MATLAB is that the
%  image size (eg. 128x128 or 256x256) of the new MINC file will be
%  the same as that of its parent file.  When parent_image is a handle
%  to an already existing image, then newimage again simply copies the
%  image size to the new image.  An error will result if the specified
%  handle or filename is invalid.
%
%  numslices and numframes describe the size of both the image data in
%  MATLAB memory and the MINC file, if applicable.  To create a
%  non-dynamic image or MINC file (ie. no time dimension), simply make
%  numframes 0.

% Check validity of input arguments

if (nargin ~= 4)
	error ('Incorrect number of arguments');
end

if ~isstr (new_file)
	error ('new_file must be a string; empty if you do not want to create a MINC file');
end

if (length(numslices) ~= 1) | (length(numframes) ~= 1)
	error ('numslices and numframes must both be integer scalars');
end

% Figure out what the handle to return should be

global ImageCount

if exist ('ImageCount') == 1
   ImageCount = ImageCount + 1;
else
   ImageCount = 1;
end

% If the supplied parent_image was a filename, get the image size from
% it (assuming square images!!!), else assume parent_image is a handle
% and use that handle to get the image size and the filename of the
% parent.

if isstr (parent_image)
	[res, out] = unix (['mincinfo -dimlength xspace ' parent_image]);
	imagesize = sscanf (out, '%d');
else
	imagesize = getimageinfo (parent_image, 'ImageSize');
	parent_image = getimageinfo (parent_image, 'Filename');
end

% If a non-empty string was supplied for new_file, create a MINC file
% with that name.  Note that parent_image will ALWAYS be a string
% going into this, because of the second getimageinfo call just above.

if ~isempty (new_file)

	unix (['micreate ' parent_image ' ' new_file ' patient study acquisition']);
	if (numframes == 0)
		unix (['micreateimage ' new_file ' ' int2str(imagesize) ' ' int2str(numslices)]);
	else
		unix (['micreateimage ' new_file ' ' int2str(imagesize) ' ' int2str(numslices) ' ' int2str(numframes)]);
	end
end

% MINC file is now created (if applicable), so we must create the
% MATLAB variables that will be used by putnextline, etc.

eval(['global Filename'     int2str(ImageCount)]);
eval(['global NumFrames',   int2str(ImageCount)]);
eval(['global NumSlices',   int2str(ImageCount)]);
eval(['global ImageSize',   int2str(ImageCount)]);
eval(['global PETimages'    int2str(ImageCount)]);
eval(['global FrameTimes'   int2str(ImageCount)]);
eval(['global FrameLengths',int2str(ImageCount)]);
eval(['global AvailFrames', int2str(ImageCount)]);
eval(['global AvailSlices', int2str(ImageCount)]);
eval(['global CurLine',    int2str(ImageCount)]);

eval(['Filename'     int2str(ImageCount) ' = new_file;']);
eval(['NumFrames'    int2str(ImageCount) ' = numframes;']);
eval(['NumSlices'    int2str(ImageCount) ' = numslices;']);
eval(['ImageSize'    int2str(ImageCount) ' = imagesize;']);
eval(['FrameTimes'   int2str(ImageCount) ' = zeros(numframes, 1);']);
eval(['FrameLengths' int2str(ImageCount) ' = zeros(numframes, 1);']);
eval(['CurLine'      int2str(ImageCount) ' = 1;']);

% If there are multiple slices or no frames, create PETimages and
% Avail{...} so that the columns of PETimages correspond to the
% slices.  Otherwise, make columns of PETimages correspond to frames.

imagelen = imagesize * imagesize;
if (numslices > 1) | (numframes == 0)
	if (numframes == 0)
		eval(['AvailFrames'  int2str(ImageCount) ' = [];']);
	else
		eval(['AvailFrames'  int2str(ImageCount) ' = 1;']);
	end
	eval(['AvailSlices'  int2str(ImageCount) ' = 1:numslices;']);
	eval(['PETimages'    int2str(ImageCount) ' = zeros(imagelen, numslices);']);
else
	if (numslices == 0)
		eval(['AvailSlices'  int2str(ImageCount) ' = [];']);
	else
		eval(['AvailSlices'  int2str(ImageCount) ' = 1;']);
	end
	eval(['AvailFrames'  int2str(ImageCount) ' = 1:numframes;']);
	eval(['PETimages'    int2str(ImageCount) ' = zeros(imagelen, numframes);']);
end

handle = ImageCount;
