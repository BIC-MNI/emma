function putimages (handle, images, slices, frames)
%PUTIMAGES  Writes whole images to an open MINC file.
%  putimages (handle, images, slices, frames)  writes images
%  (a matrix with each column containing a whole image) to the
%  MINC file specified by handle, at the slices/frames specfied
%  by the slices/frames vectors.

% ------------------------------ MNI Header ----------------------------------
%@NAME       : putimages
%@INPUT      : handle - to an already-created image in MATLAB
%              images - matrix of images, where the columns of the matrix
%              contain whole images laid out linearly
%              slices, frames - vectors describing which slices/frames
%              the columns of images correspond to.  Only one of these
%              vectors may have multiple elements, i.e. all the columns
%              of images must correspond to either a certain slice and
%              various frames (listed in the frames vector) or a certain
%              frame and various slices (listed in the slices vector).
%@OUTPUT     : 
%@RETURNS    : (none)
%@DESCRIPTION: Write images to the MINC file associated with handle.  If
%              there is no such MINC file, no action is taken.
%@METHOD     : 
%@GLOBALS    : 
%@CALLS      : miwriteimages (if there is a MINC file)
%@CREATED    : June 1993, Greg Ward & Mark Wolforth
%@MODIFIED   : 93-7-5, Greg Ward: foisted most of the work onto miwriteimages
               (the .m file, not the CMEX routine).
%-----------------------------------------------------------------------------



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

stat = check_sf (handle, slices, frames);
if (isstr(stat)); error (stat); end;

% N.B. number of rows in images is the image length (eg., 16384 for 
% 128 x 128 images); number of columns is the number of images specified.
% This must be the same as the number of elements in whichever of slices
% or frames has multiple elements.

[im_len, num_im] = size (images);

if (num_required ~= num_im)
   error (['Number of images given was ' int2str(num_im) '; number expected was ' int2str(num_required)]);
end

% make the MINC file's name global and copy to a local variable

eval(['global Filename' int2str(handle)]);
eval(['filename = Filename' int2str(handle) ';']);

if ~isempty (filename)        % write images to MINC file if there is one
   miwriteimages (filename, images, slices, frames);
else
   disp ('Warning: cannot put images without a filename');
end
