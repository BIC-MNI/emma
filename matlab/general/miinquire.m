% MIINQUIRE   find out various things about a MINC file from MATLAB
%
%   info = miinquire ('minc_file' [, 'option' [, 'item']], ...)
% 
% retrieves some item(s) of information about a MINC file.  The first
% argument is always the name of the MINC file.  Following the
% filename can come any number of "option sequences", which consist of
% the option (a string) followed by zero or more items (more strings).
% Generally speaking, the option tells miinquire the general class of
% information you're looking for (such as an attribute value or a
% dimension length), and the item or items that follow it give
% miinquire more details, such as the dimension name or
% variable/attribute name.
% 
% Any number of option sequences can be included in a single call to
% miinquire, as long as enough output arguments are provided (this is
% checked mainly as a debugging aid to the user).  Generally, each
% option results in a single output argument.
%
% The currently available options are:
%
%     dimlength  (dimension length)
%     imagesize  (sizes of the four image dimensions)
%     vartype    (variable type, as a string)
%     attvalue   (attribute value, either scalar, vector, or string)
%     orientation(image orientation, as a string: either transverse, 
%                 coronal, or sagittal)
%
% dimlength requires one item, the dimension name.  imagesize requires 
% no items.  vartype requires the variable name.  attvalue requires
% both the variable name and attribute name, in that order.  See Examples
% below for further illumination.
% 
% Options that will most likely be added in the near future are:
%
%     dimnames
%     varnames
%     vardims
%     varatts
%     atttype
%
% One inconsistency with the standalone utility mincinfo (after which 
% miinquire is modelled) is the absence of the option "varvalues".  
% The functionality of this available in a superior way via the CMEX
% mireadvar.
%
% Minor errors such as a dimension, variable, or attribute not found
% in the MINC file will result in an empty matrix being returned.
% miinquire will abort if there is not exactly one output argument
% for every option sequence; if any option does not have all the 
% required items supplied; or if the MINC file is not found or is 
% invalid (eg. missing image variable).
%
% EXAMPLES
%
%  NumFrames = miinquire ('foobar.mnc', 'dimlength', 'time');
%
%    retrieves the length of the dimension names 'time', and stores it in
%    MATLAB as the variable NumFrames (a scalar).  Here 'dimlength' is
%    the option, and 'time' is the item associated with that option.
%
%  ImageSize = miinquire ('foobar.mnc', 'imagesize');
%
%    gets the sizes of the four image dimensions and puts them into a 
%    column vector in the order [#frames, #slices, height, width].  If
%    either the frame or slice dimension is missing, that element of
%    the vector is set to zero.  If either the height or width dimension
%    is missing, the MINC file is invalid.  Here, 'imagesize' is the
%    option string and there are no items.
%
%  ValidRange = miinquire ('foobar.mnc', 'attvalue', 'image', 'valid_range');
%
%    gets the value(s) of the attribute valid_range associated with the 
%    variable image.  (According to the MINC standard, the valid_range 
%    attribute should have two values.  This is not checked by miinquire.)
%    In this case, the option 'attvalue' requires two items: a variable
%    name ('image') and an attribute name ('valid_range').
%
%  Finally, these three calls could just as easily have been done all at once,
%  as in the following:
%
%  [NumFrames, ImageSize, ValidRange] = miinquire ('foobar.mnc', ...
%      'dimlength', 'time', 'imagesize', 'attvalue', 'valid_range');
%
