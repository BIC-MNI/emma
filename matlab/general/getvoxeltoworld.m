function xfm = getvoxel2world (volume)
% GETVOXEL2WORLD  Build the voxel-to-world coordinate transform for a volume
%
%   xfm = getvoxel2world (volume)
% 
% returns the 4x4 transformation matrix derived from the volume's step
% sizes, start coordinates, and direction cosines.  See the source for
% how this is done.
%
% The volume argument can be either an image handle or a filename.
% 
% NOTE: the transformation matrix can be applied by simple matrix
% multiplication to a 4x1 column vector of the form [vx vy vz 1]',
% containing the location of a point in voxel coordinates.  However,
% if these are slice and pixel numbers obtained within MATLAB, they
% will be one-based, whereas voxel coordinates are generally
% zero-based.  (That is, the first slice in the volume is usually
% called slice zero, whereas by EMMA conventions it is slice one.)
% Thus, you must first subtract one from each of vx, vy, and vz before
% applying the matrix returned by getvoxel2world.  Similarly, if you
% are applying the world-to-voxel transform (the inverse of the matrix
% returned by getvoxel2world), the resulting coordinates will be
% zero-based, so you should add one to the resulting x-, y-, and
% z-coordinates to obtain slice/pixel numbers usable with EMMA.
% 
% SEE ALSO
%   voxel2world, world2voxel

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
