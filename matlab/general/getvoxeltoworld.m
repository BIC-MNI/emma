function xfm = getvoxeltoworld (volume)
% GETVOXELTOWORLD  Build the voxel-to-world coordinate transform for a volume
%
%   xfm = getvoxeltoworld (volume)
% 
% returns the 4x4 transformation matrix to convert voxel coordinates to
% world coordinates.  This matrix assumes that 1) the voxel coordinates
% are zero-based (C conventions, not MATLAB), and 2) the voxel
% coordinates are in (x,y,z) order.  If this is not the case, you should
% be using voxeltoworld and worldtovoxel, as they take care of these
% issues for you.
% 
% The voxel-to-world transformation matrix is derived from the volume's
% step sizes, start coordinates, and direction cosines.  See the source
% for how this is done.
% 
% The volume argument can be either an image handle or a filename.
% 						  
% getvoxeltoworld is a fairly low-level function; usually, you should
% use voxeltoworld or worldtovoxel, which both call getvoxeltoworld and
% also handle issues of array indexing and coordinate ordering properly
% (see below).  These issues are kind of hairy, but quite important;
% usually, you can let the two higher-level functions take care of them,
% but if you really want to know the gory details, read on.
%
% First, you must consider the base index of arrays.  In C (and thus in
% most MINC utilities and applications), the first element of an array
% is element 0.  Thus, voxel coordinates start at (0,0,0); in a volume
% with all positive step sizes, this will be the inferior, posterior,
% left-most corner of the volume.  However, the convention in MATLAB is
% to number arrays starting at 1.  EMMA uses this convention, so (for
% instance) slice numbers passed to getimages or putimages start at 1,
% row/column coordinates within an image start at 1, etc.  Thus, one of
% the steps in translating world coordinates to voxel coordinates *for
% use within MATLAB* must be to add/subtract 1 to/from voxel
% coordinates.  If you use the transform matrix returned by
% getvoxeltoworld, this will *not* be done -- you should use
% voxeltoworld and/or worldtovoxel.  However, if you're performing the
% coordinate transformation for use by utilities *outside* of MATLAB,
% you should stick to the zero-based convention.  This can be done by
% using the 'external' option with voxeltoworld and worldtovoxel.  (Or,
% you can just use the matrix returned by getvoxeltoworld -- but beware
% of dimension ordering!)
%
% Next, you must consider dimension ordering.  The canonical order, and
% the way in which world coordinates are *always* specified, is (x,y,z).
% However, MINC volumes can be stored in a variety of orders; the most
% common are transverse (z,y,x), coronal (y,z,x), or sagittal (x,z,y).
% When coordinates are specified in this order, it's easy to pick out
% the slice number -- it's just the first coordinate.  Likewise, the
% "row" coordinate is the second, and the "column" coordinate is the
% last.  (These are called row and column coordinates because it makes
% anatomical sense to display volumes with one of the three standard
% orientations such that the fastest-varying dimension is horizontal on
% the screen, and the second-fastest-varying dimension is vertical.)
%
% Since you always specify coordinates in MATLAB by slice, row, and
% column (ie. in voxel order), then we obviously need something to take
% us from world order (x,y,z) to voxel order.  This is the "permutation
% matrix" P, which is simply a 4x4 identity matrix with the rows
% reordered according to the volume's dimension ordering.  The
% `permutation' option to miinquire will give you the permutation matrix
% to go from voxel order to world order.  (To go the opposite direction,
% simply invert the permutation matrix.  Actually, since it's an
% orthogonal matrix, you really only need to transpose it, if you're
% really worried about shaving off every possible clock cycle.)  
%
% Thus, to convert a point from voxel coordinates in voxel order to
% world coordinates (in world order, of course!), you would use
% something like this:
%
%      T = getvoxeltoworld (volume);
%      P = miinquire (getimageinfo (volume, 'filename'), 'permutation');
%      v = [50 30 10 1]';
%      w = T * P * v;
%
% Ignoring the issue of array indexing (here everything must be
% zero-based, as getvoxeltoworld makes no adjustment for the one-based
% MATLAB world), this is more or less what the voxeltoworld function
% does.
%
% Note that P is applied first (to reorder the point to world order),
% and then T is applied -- the transform matrix returned by
% getvoxeltoworld assumes that the voxel points will be in (x,y,z)
% order.  You could, of course, post-multiply T by P to get a
% voxel-to-world matrix that expects voxel coordinates in voxel order.
%
% SEE ALSO
%   voxeltoworld, worldtovoxel

% by Greg Ward 95/3/10

%
% Get image information - steps, starts, and direction cosines
%

if (isstr (volume))
   filename = volume;
elseif (size(volume) == [1,1])
   eval(['global Filename',int2str(volume),';']);
   eval(['filename = Filename',int2str(volume),';']);
else
   error ('volume argument must be either an image handle or a filename');
end

[xstep,ystep,zstep,xstart,ystart,zstart,xdircos,ydircos,zdircos] = ...
      miinquire (filename, ...
      'attvalue','xspace','step',...
      'attvalue','yspace','step',...
      'attvalue','zspace','step',...
      'attvalue','xspace','start',...
      'attvalue','yspace','start',...
      'attvalue','zspace','start',...
      'attvalue','xspace','direction_cosines',...
      'attvalue','yspace','direction_cosines',...
      'attvalue','zspace','direction_cosines');

if (isempty (xstep) | isempty (ystep) | isempty (zstep))
   error (['Volume ' filename ' is missing one of xstep, ystep, or zstep']);
end

% Fill in any missing values

if (isempty (xstart)), xstart = 0; end;
if (isempty (ystart)), ystart = 0; end;
if (isempty (zstart)), zstart = 0; end;
if (isempty (xdircos)), xdircos = [1 0 0]; end;
if (isempty (ydircos)), ydircos = [0 1 0]; end;
if (isempty (zdircos)), zdircos = [0 0 1]; end;

% And construct the transformation matrix

xfm = [xstep*xdircos' ...         % first column
       ystep*ydircos' ...         % second column
       zstep*zdircos' ...         % third column
       (xstart*xdircos' + ystart*ydircos' + zstart*zdircos') ];

xfm = [xfm; 0 0 0 1];
