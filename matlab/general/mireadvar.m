%MIREADVAR  Read a hyperslab of data from any variable in a MINC file.
%
%  data = mireadvars ('MINC_file', 'var_name', [, start, count[, options]])
%
%  Given vectors describing the starting corner (zero-based!) and edge
%  lengths, mireadvars reads an n-dimensional hyperslab from a MINC
%  (or NetCDF) file.  The data is returned as a MATLAB vector, with
%  the highest dimension of the variable changing fastest.
% 
%  The simplest (and intended) use of mireadvars is to read an entire
%  one-dimensional variable.  For example:
%
%    time = mireadvars ('foobar.mnc', 'time');
%
%  will read the entire contents of the variable 'time' from the file
%  foobar.mnc.  (If the start and count vectors are not given, they
%  default to reading the entire variable.  Currently, if start is
%  given, count must be given, and they must each have exactly one
%  element per dimension.)
%
%  A more complicated example is to use mireadvar as a low-rent
%  substitute for mireadimages.  For example, to read slice 5, frame 7
%  (note that these are 1-based, and mireadvar expects 0-based
%  indeces!) of foobar.mnc:
%
%    image = mireadvar ('foobar.mnc', 'image', [6 4 0 0], [1 1 128 128]);
%
%  The disadvantages of this approach are numerous.  First of all,
%  mireadimages will perform the scaling and shifting necessary to
%  transform the image data from scaled bytes or shorts (or however it
%  happens to be stored in the MINC file) to floating point values
%  representing the actual physical data.  Second, with mireadvar you
%  must know the exact order of the dimensions: in the above example,
%  "slice 5" corresponds to the 4 (note zero-based!) at position 2 of
%  the Start vector, and "frame 7" is the 6.  Also, you must know the
%  size of the image; mireadimages will handle anything, not just
%  128x128.  Finally, mireadimages provides greater flexibility with
%  respect to slice and frame selection.  With mireadvar, you can only
%  read contiguous ranges of slices and frames, and you must figure
%  out the start and count values for each dimension yourself.
%  Mireadimages, however, does all that work for you given just slice
%  and frame numbers.

%  MIREADVAR by Greg Ward.  See mireadvar.c for more illumination.
