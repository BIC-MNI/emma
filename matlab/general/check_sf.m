function msg = check_sf (handle, slices, frames)
%  check_sf  - for internal use only
%  msg = []: all is OK
%  msg = error message if there's anything wrong

% check_sf - makes sure that the given slices and frames vectors
% are consistent with the image specified by handle.  Checks for:
%   * both slices and frames cannot be vectors
%   * if image has no frames, frames vector should be empty (warning only)
%   * if frames vector is empty, image must have no frames
%   * if image has no slices, slices vector should be empty (warning only)
%   * if slices vector is empty, image must have no slices

msg = [];

% First retrieve the number of frames and slices from the global workspace

eval(['global NumFrames' int2str(handle)]);
eval(['global NumSlices' int2str(handle)]);
num_frames = eval(['NumFrames' int2str(handle)]);
num_slices = eval(['NumSlices' int2str(handle)]);

if (length(slices) > 1) & (length(frames) > 1)
   msg = 'Cannot specify both multiple slices and multiple frames';
end

if (num_frames == 0)
   if ~isempty (frames)
      disp ('Warning: image has no frames, frame list will be ignored');
   end
end

if (isempty (frames)) & (num_frames > 0)
   msg = 'Image has a time dimension; you must specify frames';
end

if (num_slices == 0)
   if ~isempty (slices)
      disp ('Warning: image has no slices, slice list will be ignored');
   end
end

if (isempty (slices)) & (num_slices > 0)
   msg = 'Image has a slice dimension; you must specify slices';
end
