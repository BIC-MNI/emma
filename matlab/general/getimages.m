function images = getimages (handle, slices, frames)
%GETIMAGES  Retrieve whole images from an open MINC file.
%  images = getimages (handle, slices[, frames]) reads the slices or frames
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
%	    list of frame numbers, they will be ignored and a warning message
%		 printed.
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
%					image data) to read from
%					slices - list of slices (1-based) to read
%					frames - list of frames (1-based) to read
%@OUTPUT     : 
%@RETURNS    : images - matrix whose columns contain entire images
%					layed out linearly.
%@DESCRIPTION: 
%@METHOD     : 
%@GLOBALS    : Filename#
%@CALLS      : check_sf to check validity of slices/frames arguments
%					mireadimages (CMEX)
%@CREATED    : June 1993, Greg Ward & Mark Wolforth
%@MODIFIED   : 
%-----------------------------------------------------------------------------


% Check for valid number of arguments

if (nargin < 2) | (nargin > 3)
   error ('Incorrect number of arguments.');
end

if (nargin < 3)			% no frames vector given, so make it empty
	frames = [];
end

% Make the handle's filename global, and check that it exists

eval(['global Filename' int2str(handle)]);
if exist (['Filename' int2str(handle)]) ~= 1
   error ('Image has not been opened or has been closed - use openimage.');
end 

% and copy it to a local variable for ease of use

filename = eval (['Filename' int2str(handle)]);

% now make sure input arguments are valid: check_sf aborts with error 
% message if not

check_sf (handle, slices, frames);

% Now read the images!  (remembering to make slices and frames zero-based for
% mireadimages).  If frames was not supplied, then do not attempt to pass
% it to mireadimages.

images = mireadimages (filename, slices-1, frames-1);
