function images = getimages (handle, slices, frames, old_matrix)
%GETIMAGES  Retrieve whole images from an open MINC file.
%
%  images = getimages (handle, slices[, frames])
%
%  reads the slices or frames
%  listed in the vectors slices and frames from the MINC file specfied by
%  handle.  
%
%  Rules:
%
%    All slice and frame numbers are one-based.  
%
%    Only one of slices or frames can have multiple elements; that is,
%      you cannot read in multiple slices and multiple frames at the
%      same time.
%
%    If the file is non-dynamic (ie. has no time dimension), then you
%      can either not specify the frames vector, or make it an empty
%      vector.  Either method will work.  If, however, you include a
%      list of frame numbers, they will be ignored and a warning message
%      printed.
%
%  The returned matrix, images, contains the images as vectors, one per
%  column.  Thus if you read in 5 128x128 images, the matrix images
%  will have 16384 (=128*128) rows and 5.
%
%  EXAMPLES (assuming handle = openimage ('some_minc_file');)
%   To read in the first frame of the first slice:
%     one_image = getimages (handle, 1, 1);
%   To read in the first 10 frames of the first slice:
%     first_10 = getimages (handle, 1, 1:10);
%   To read in the first 10 slices of a non-dynamic (i.e. no frames) file:
%     first_10 = getimages (handle, 1:10);

% ------------------------------ MNI Header ----------------------------------
%@NAME       : getimages
%@INPUT      : handle - tells which MINC file (or internal-to-MATLAB
%              image data) to read from
%              slices - list of slices (1-based) to read
%              frames - list of frames (1-based) to read
%@OUTPUT     : 
%@RETURNS    : images - matrix whose columns contain entire images
%              layed out linearly.
%@DESCRIPTION: Reads images from the MINC file associated with a MATLAB
%              image handle.  The handle must have an associated MINC file;
%              purely internal image sets are not yet supported.  
%
%              Note that if care is not taken, this can easily take up
%              large amounts of memory.  Each image takes up 128 k of
%              MATLAB memory, so reading all frames for a single slice
%              from a 21-frame dynamic study will take up 2,688 k.
%              When various analysis routines are carried out on this
%              data, the amount of memory allocated by MATLAB can
%              easily triple or quadruple.  getimages attempts to combat
%              this by assigning a "maximum" number of images for each
%              of PET's four main SGI's (as of 93/7/6: priam, duncan, lear,
%              portia) depending on their current memory 
%              configurations.  If the number of slices/frames specified
%              is greater than this "maximum", getimages will print a 
%              warning and then read the data.  
%
%@METHOD     : 
%@GLOBALS    : Filename#
%@CALLS      : check_sf to check validity of slices/frames arguments
%              mireadimages (CMEX)
%@CREATED    : June 1993, Greg Ward & Mark Wolforth
%@MODIFIED   : 6 July 1993, Greg Ward: 
%-----------------------------------------------------------------------------


% Check for valid number of arguments

if (nargin < 1) | (nargin > 4)
   error ('Incorrect number of arguments.');
end

if (nargin < 2)
   slices = [];         % no slices vector given, so make it empty
end

if (nargin < 3)         % no frames vector given, so make it empty
   frames = [];
end


% Make the handle's filename global, and check that it exists

eval(['global Filename' int2str(handle)]);
if exist (['Filename' int2str(handle)]) ~= 1
   disp ('getimages: image unknown - use openimage');
end 

% and copy it to a local variable for ease of use

filename = eval (['Filename' int2str(handle)]);

if isempty (filename)
   disp ('getimages: no MINC file associated with image, cannot read images');
end

% now make sure input arguments are valid: check_sf returns an error message
% if not.

s = check_sf (handle, slices, frames);
if ~isempty (s); error (s); end;

% Find out the machine we're on so we can make some educated guesses
% as to how much memory usage should elicit a warning

host = getenv ('HOST');
if (strcmp (host, 'priam'))        % 80 MB of main memory on priam
   max_im = 80;                    % 80 images = 10 MB
elseif (strcmp (host, 'duncan') | strcmp (host, 'lear'))
   max_im = 40;                    % 40 images = 5 MB
elseif strcmp (host, 'portia')
   max_im = 12;                    % 12 images = 1.5 MB
else
   max_im = 30;                    % completely arbitrary for unknown machine
end

num_im = max (length (slices), length (frames));
if num_im > max_im
   disp (['getimages warning: reading enough images to possibly bring ' host ' to its knees']);
end

% Now read the images!  (remembering to make slices and frames zero-based for
% mireadimages).

if (nargin < 4)
    images = mireadimages (filename, slices-1, frames-1);
else
    images = mireadimages (filename, slices-1, frames-1, old_matrix);
end
