%MIREADIMAGES  Read images from specified slice(s)/frame(s) of a MINC file.
%
%  images = mireadimages ('minc_file' [, slices [, frames ...
%                         [, old_matrix [, start_row [, num_rows]]]]])
%
%  opens the given MINC file, and attempts to read whole or partial
%  images from the slices and frames specified in the slices and
%  frames vectors.  If start_row and num_rows are not specified, then
%  whole images are read.  If only start_row is specified, a single
%  row will be read.  If both start_rows and num_rows are given, then
%  the specified number of rows will be read (unless either is out of
%  bounds, which will result in an error message).
%
%  For the case of 128 x 128 images, the images are returned as the
%  the columns of a 16384-row matrix, with the highest image dimension
%  varying the fastest.  For example, with transverse images, the x
%  dimension is the image "width" dimension, and varies fastest in the
%  MINC file.  Thus, the fastest varying dimension in the MATLAB
%  column vector that represents the image will be x (width), so each
%  contiguous block of 128 elements will represent a single row of the
%  image.  If only (say) eight rows are read, then the matrix returned
%  by mireadimages will be only 1024 (= 8*128) elements deep.  Thus,
%  it is straightforward to read successive partial images (eg., 8 or
%  16 rows at a time) to sequentially process entire images when
%  memory is tight.
%
%  Another way to economise on memory is to make use of the old_matrix
%  argument -- when doing successive reads of identically-sized blocks
%  of image data, passing the MATLAB matrix that contains the previous
%  image(s) as old_matrix allows mireadimages to "recycle"
%  previously-used memory, and partially alleviate some of MATLAB's
%  deficient memory management.
%
%  To manipulate a single image as a 128x128 matrix, it is necessary
%  to extract the desired column (image), and then reshape it to the
%  appropriate size.  For example, to load all frames of slice 5, and 
%  then extract frame 7 of the file foobar.mnc:
%
%  >> images = mireadimages ('foobar.mnc', 4, 0:20);
%  >> frame7 = images (:, 7);
%  >> frame7 = reshape (frame7, 128, 128);
%
%  Note that mireadimages expects slice and frame numbers to be zero-
%  based, whereas in MATLAB array indexing is one-based.  Thus, frames
%  0 .. 20 of the MINC file are read into columns 1 .. 21 of the
%  matrix images.
%
%  For most dynamic analyses, it will also be necessary to extract
%  the frame timing data.  This can be done using MIREADVAR.
%
%  Currently, only one of the vectors slices or frames can contain multiple
%  elements.

% $Id: mireadimages.m,v 1.4 1997-10-20 18:23:20 greg Rel $
% $Name:  $

%  MIREADIMAGES -- written by Greg Ward 93/6/6.
