function [out] = voxel2world (volume, in, option)
% VOXEL2WORLD  convert points from world to voxel coordinates
%
%   w_points = voxel2world (volume, v_points [, 'external'])
% or
%   w2v_xfm = voxel2world (volume)
% 
% If the first form is used, then v_points must be a matrix with N
% columns (for N points) and either 3 or 4 rows.  (If four rows are
% supplied, then the fourth should be all ones; if only three rows are
% supplied, a fourth row of all ones will be added.)  Normally,
% voxel2world assumes that the the input voxel coordinates originate
% from slice/pixel numbers within EMMA, i.e. that the coordinates are
% 1-based.  In this case, the coordinates must be made zero-based by
% subtracting one before applying the voxel-to-world transform.
% However, if the points have an origin outside of the EMMA
% environment, they are most likely zero-based.  Adding the 'external'
% flag will cause the subtraction of one to be suppressed.
%  
% If the second form is used then just the voxel-to-world transform (a
% 4x4 matrix) is returned.  This is simply the matrix returned by
% getvoxel2world.  Note that to apply this transform to voxel
% coordinates, they must already be zero-based, so you will have to
% subtract one yourself.  (See the help for getvoxel2world for more
% information.)
% 
% The volume parameter can be either an image handle or a filename.
%
% EXAMPLES
%
% SEE ALSO
%   getvoxel2world, world2voxel

% by Greg Ward 95/3/12


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
  help voxel2world
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

v2w = getvoxel2world (volume);


% If only one argument was supplied, just return the transform

if (nargin == 1)
   out = v2w;

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

   points(1:3,:) = points(1:3,:) - offset;
   points = v2w * points;     % perform the transformation
   
   if (m == 3)                % if caller only supplied 3 rows, lose the 4th
      points = points (1:3,:);
   end
   out = points;

end
