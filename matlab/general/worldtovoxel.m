function [x,y,z] = world2voxel (h, w_x, w_y, w_z)

%
%
%      [x,y,z] = world2voxel (handle, w_x, w_y, w_z)
%
%

% @COPYRIGHT  :
%             Copyright 1994 Mark Wolforth, McConnell Brain Imaging Centre,
%             Montreal Neurological Institute, McGill University.
%             Permission to use, copy, modify, and distribute this software
%             and its documentation for any purpose and without fee is
%             hereby granted, provided that the above copyright notice
%             appear in all copies.  The author and McGill University make
%             no representations about the suitability of this software for
%             any purpose.  It is provided "as is" without express or
%             implied warranty.


%
% Check input arguments
%

if (nargin ~= 4)
  help world2voxel
  error ('Too few input arguments');
end


%
% Get image information
%

eval(['global Filename',int2str(h),';']);
eval(['filename = Filename',int2str(h),';']);

zstep  = miinquire (filename,'attvalue','zspace','step');
zstart = miinquire (filename,'attvalue','zspace','start');
ystep  = miinquire (filename,'attvalue','yspace','step');
ystart = miinquire (filename,'attvalue','yspace','start');
xstep  = miinquire (filename,'attvalue','xspace','step');
xstart = miinquire (filename,'attvalue','xspace','start');

z = round((w_z - zstart) ./ zstep);
y = round((w_y - ystart) ./ ystep);
x = round((w_x - xstart) ./ xstep);
