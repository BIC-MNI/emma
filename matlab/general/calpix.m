function cp = calpix(x,y)
% CALPIX  generates the vector index of a point
%
%       cp = calpix(x,y)
%
% generates the vector index (1 to 16384) of 
% the x and y coordinates of a pixel in a 128*128 image.
% It uses the simple formula 
%
%       cp = round(x) + 128 * round(y-1)
%

cp = round(x) + 128 * round(y-1);
