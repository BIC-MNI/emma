function putimage (handle, images, slices, frames)
%PUTIMAGES  Writes whole images to an open MINC file.
%  putimages (handle, images, slices, frames)  writes images
%  (a matrix with each column containing a whole image) to the
%  MINC file specified by handle, at the slices/frames specfied
%  by the slices/frames vectors.
%
% NOTE! This function does *NOT* currently write anything to disk.
% that should be done by closeimage or setslice/setframe (forthcoming).

if ((nargin < 3) | (nargin >4))
    error ('Incorrect number of arguments.');
end

% if frames not supplied; make it empty

if (nargin < 4)
	frames = [];
end

% figure out number of images we expect to see in matrix images

num_required = max (length(slices), length(frames));

% check that slices and frames are valid

check_sf (handle, slices, frames);

% If both frames and slices given, make sure they are not both vectors
% and use the number of elements in whichever one IS a vector as the
% number of images we expect to find in images.

%if (nargin == 4)
%	if ((length(slices) > 1) & (length(frames) > 1))
%   	 error('Cannot handle both multiple slices and multiple frames');
%	end
%else
%	num_required = length(slices);
%end

% N.B. number of rows in images is the image length (eg., 16384 for 
% 128 x 128 images); number of columns is the number of images specified.
% This must be the same as the number of elements in whichever of slices
% or frames has multiple elements.

[im_len, num_im] = size (images);

if (num_required ~= num_im)
	error (['Number of images given was ' int2str(num_im) '; number expected was ' int2str(num_required)]);
end

% Make Filename#, PETimages#, AvailSlices# and AvailFrames# global; copy
% Filename#, AvailSlices#, AvailFrames# to local variables.

eval(['global Filename' int2str(handle)]);
eval(['global PETimages' int2str(handle)]);
eval(['global AvailSlices' int2str(handle)]);
eval(['global AvailFrames' int2str(handle)]);
eval(['filename = Filename' int2str(handle) ';']);
eval(['avail_slices = AvailSlices' int2str(handle) ';']);
eval(['avail_frames = AvailFrames' int2str(handle) ';']);


if ~isempty (filename)			% write images to MINC file if there is one
	miwriteimages (filename, images, slices, frames);
end

return



% Now deposit the data in memory.  The two clauses of this if/else are
% identical, save that "slice" and "frame" are interchanged.  The
% first case is for when there are multiple (or one) slices and a
% single (or no) frame; so we require that the given frame number --
% if there is one -- be in avail_frames.  (Actually, avail_frames in
% this case had better be a scalar, but this is not checked.)  Then,
% we loop through the members of slices, and for each one make sure
% the slice number is in avail_slices, and then copy the next column
% of images to the appropriate column of PETimages#.

if (length(slices) > length(frames))
	if ~isempty (frames) & (frames ~= avail_frames)
		error(['Frame ' int2str(frames) ' not available in memory.']);
	end

	for i = 1:length(slices)
		loc = find (avail_slices == slices (i));
		if isempty (loc)
			error (['Slice ' int2str(slices(i)) ' not available to write to']);
		else
			eval(['PETimages' int2str(handle) ' (:,loc) = images (:,i);']);
		end
	end

else			 % we were given multiple frames and a single slice, so do the
				 % same thing with slices and frames interchanged
	if ~isempty (slices) & (slices ~= avail_slices)
		error (['Slice ' int2str(slices) ' not available in memory.']);
	end

	for i = 1:length(frames)
		loc = find (avail_frames == frames (i));
		if isempty (loc)
			error (['Frame ' int2str(frames(i)) ' not available to write to']);
		else
			eval(['PETimages' int2str(handle) ' (:,loc) = images (:,i);']);
		end
	end

end




