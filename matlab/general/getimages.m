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

% Check for valid number of arguments

if (nargin < 2) | (nargin > 3)
   error ('Incorrect number of arguments.');
end

% If both frames and slices given, make sure they are not both vectors

if (nargin == 3)
	if (length(frames) > 1) & (length(slices) > 1)
		error ('Cannot handle both multiple slices and multiple frames');
	end
end

% Make the handle's filename global, and check that it exists

eval(['global Filename' int2str(handle)]);
if exist (['Filename' int2str(handle)]) ~= 1
   error ('Image has not been opened or has been closed - use openimage.');
end 

% and copy it to a local variable for ease of use

filename = eval (['Filename' int2str(handle)]);

% Now read the images!  (remembering to make slices and frames zero-based for
% mireadimages).  If frames was not supplied, then do not attempt to pass
% it to mireadimages.

if (nargin == 3)
	disp (['Reading slice ' int2str(slice) ' frame ' int2str(frame) ' from ' filename]);
	images = mireadimages (filename, slices-1, frames-1);
else
	disp (['Reading slice ' int2str(slice) ' from ' filename]);
	images = mireadimages (filename, slices-1);
end
