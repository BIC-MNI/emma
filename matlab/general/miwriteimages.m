function miwriteimages (filename, images, slices, frames)
%
%   miwriteimages (filename, images, slices, frames)
%
%  writes images (in the format as returned by mireadimages) to a MINC
%  file.  The MINC file must already exist and must have room for the 
%  data.  slices and frames only tell miwriteimages where to put the data
%  in the MINC file, they are not used to select certain columns from images.

slicelist = '';
for i = 1:(length(slices) - 1)
	slicelist = [slicelist int2str(slices(i)-1)];
end
slicelist = [slicelist int2str(slices(length(slices))-1)]

if (nargin < 4)
	framelist = '-'
else
	if (~isempty(frames))
		framelist = '';
		for i = 1:(length(frames) - 1)
			framelist = [framelist int2str(frames(i)-1)];
		end
		framelist = [framelist int2str(frames(length(frames))-1)]
	end
end

tempfile = tempfilename;
outfile = fopen (tempfile, 'w');
if (outfile == -1)
	error (['Could not open temporary file ' tempfile ' for writing!']);
end

count = fwrite (outfile, images, 'double');

unix (['miwriteimages ' filename ' ' slicelist ' ' framelist ' ' tempfile]);
