function out = worldtovoxel (volume, in, option)
% WORLDTOVOXEL  convert points from world to voxel coordinates
%
%   v_points = worldtovoxel (volume, w_points [, 'external'])
% or
%   wv_xfm = worldtovoxel (volume)
% 
% If the first form is used, then w_points must be a matrix with N
% columns (for N points) and either 3 or 4 rows.  (If four rows are
% supplied, then the fourth should be all ones; if only three rows are
% supplied, a fourth row of all ones will be added.)  Normally,
% worldtovoxel assumes that the the voxel coordinates are to be used as
% slice/pixel numbers within EMMA, i.e. that the coordinates should be
% 1-based.  However, if the points are to be used externally (eg.,
% written to a tag file), then they should be zero-based.  Adding the
% 'external' flag will cause this to be done.
%  
% If the second form is used then the just the world-to-voxel
% transform (a 4x4 matrix) is returned.  This is simply the inverse of
% the matrix returned by getvoxeltoworld.  Note that that applying
% this transform to points in world coordinates will give zero-based
% voxel coordinates; you must take care of adding 1 yourself if you
% need to use the voxel coordinates within MATLAB.  (See the help
% for getvoxeltoworld for more information.)
% 
% The volume parameter can be either an image handle or a filename.
%
% EXAMPLES
% 
% To use the points in a tag file with EMMA:
%
%    h = openimage ('foo.mnc');
%    w_tags = loadtagfile ('foo.tag');   % get tags in world coordinates
%    v_tags = worldtovoxel (h, w_tags');  % CHECK: is this right??!!
%
% Note that this does NOT give the same v_tags as:
% 
%    w2v = inv (getworldtovoxel ('foo.mnc'));
%    w_tags = loadtagfile ('foo.tag');
%    [num_points,n] = size (w_tags);
%    v_tags = w2v * [w_tags' ; ones(1,num_points)];
%
% because in the first case, the coordinates in v_tags will be
% one-based, and in the second case, they will be zero-based.  In
% almost all cases it is preferable to use worldtovoxel and voxeltoworld.
%
% SEE ALSO
%   getvoxeltoworld, voxeltoworld

% by Mark Wolforth; rewritten 95/3/10-12 by Greg Ward


% @COPYRIGHT  :
%             Copyright 1994 Mark Wolforth and Greg Ward, McConnell
%             Brain Imaging Centre, Montreal Neurological Institute,
%             McGill University.  Permission to use, copy, modify, and
%             distribute this software and its documentation for any
%             purpose and without fee is hereby granted, provided that
%             the above copyright notice appear in all copies.  The
%             authors and McGill University make no representations about
%             the suitability of this software for any purpose.  It is
%             provided "as is" without express or implied warranty.


%
% Check input arguments
%

if (nargin < 1 | nargin > 3)
  help worldtovoxel
  error ('Incorrect number of arguments');
end

offset = 1;                  % default value if option ~= 'external' 
if (nargin == 3)
   if (~isstr (option))
      error ('If given, option argument must be a string');
   end
   offset = ~strcmp (option, 'external');
   nargin = 2;
end

wv_xfm = getvoxeltoworld (volume);
wv_xfm = inv (wv_xfm);


% If only one argument was supplied, just return the transform

if (nargin == 1)
   out = wv_xfm;

% If exactly two arguments were supplied, the second is a matrix
% of points.  First check to see if caller supplied a three-row
% or four-row matrix; if three, we have to tack on ones to make
% the points homogeneous

elseif (nargin == 2)
   points = in;
   [m,n] = size (points);     % make sure we have points in homogeneous
   if (m == 3)                % coordinates (i.e. [x y z 1]')
      points = [points; ones(1,n)];
   elseif (m ~= 4)
      error ('If a matrix of points is supplied, it must have either three or four rows');
   end

   points = wv_xfm * points;    % perform the transformation
   points(1:3,:) = points(1:3,:) + offset;
   
   if (m == 3)                % if caller only supplied 3 rows, lose the 4th
      points = points (1:3,:);
   end
   out = points;

% Otherwise, separate vectors were supplied, so apply the matrix to them
% (This is a pain-in-the-neck special case necessitated by the way
% Mark originally wrote the function.  I'd love to get rid of it...)

%elseif (nargin == 4)
%   [mx,nx] = size (w_x); [my,ny] = size (w_y); [mz,nz] = size (w_z);
%   if (mx ~= my | mx ~= mz | nx ~= ny | nx ~= nz)
%      error ('w_x, w_y, and w_z must be the same size');
%   end
%
%   if (mx ~= 1 & my ~= 1)
%      error ('w_x, w_y, and w_z must be vectors');
%   end
%
%   if (nx == 1)           % if they passed vectors as columns
%      w_x = w_x';         % then transpose them to rows
%      w_y = w_y'; 
%      w_z = w_z';
%      num_points = my;
%   else
%      num_points = nx;
%   end
%
%   temp = [w_x; w_y; w_z; ones(1,num_points)];
%   temp = wv_xfm * temp;
%   v_x = temp(1,:);
%   v_y = temp(2,:);
%   v_z = temp(3,:);
%
%   if (nx == 1)
%      v_x = v_x';         % then transpose them to columns
%      v_y = v_y'; 
%      v_z = v_z'; 
%   end
end
