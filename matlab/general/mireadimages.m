%MIREADIMAGES  Read images from specified slice(s)/frame(s) of a MINC file.
%
%  images = mireadimages ('minc_file'[, slices[, frames[, options]]])
%
%  opens the given MINC file, and attempts to read whole images from
%  the slices and frames specified in the slices and frames vectors.
%  For the case of 128 x 128 images, the images are returned as the 
%  the columns of a 16384-row matrix, with the highest image dimension
%  varying the fastest.  That is, if x is the highest image dimension,
%  each contiguous block of 128 elements will correspond to one row
%  of the image.
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
%  based!  Thus, frames 0 .. 20 of the MINC file are read into columns
%  1 .. 21 of the matrix images.
%
%  For most dynamic analyses, it will also be necessary to extract
%  the frame timing data.  This can be done using MIREADVAR.
%
%  Currently, only one of the vectors slices or frames can contain multiple
%  elements.

%  MIREADIMAGES -- written by Greg Ward 93/6/6.
