function index = pixelindex (img,x,y)
% PIXELINDEX  generates the vector index of a point
%
%       index = pixelindex (img,x,y)
% 
% generates the vector index of the x and y coordinates of a pixel in
% an image.  img is either an image handle or a 2x1 vector describing
% the size of each image of the form returned by getimageinfo's 
% `imagesize' option.  That is, the first number is the number of
% pixels in the image "height" dimension, and the second number is
% the number of pixels in the "width" dimension.  ("Height" and "width"
% are respectively the second-fastest and fastest-varying dimensions
% in the volume.)
% 
% EXAMPLES
% 
% If the image is 128x128 pixels, then the vector index will be from
% 1..16384; pixel (x,y) = (37,24) will translate as follows:
%
%     index = width*(y-1) + x = 128*23 + 37 = 2981
% 
% where width = img(2).

% by Greg Ward 95/3/16 (from the obsolete calpix.m)

error (nargchk (3, 3, nargin));

if (size(img) == [1 1])
   imgsize = getimageinfo (img, 'imagesize');
elseif (size(img) == [1 2] | size(img) == [2 1])
   imgsize = img;
else
   error ('img must be either an image handle or a 2x1 vector');
end

index = imgsize(2) * round(y-1) + round(x);
