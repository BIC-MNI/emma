function [no,xo] = gettaggedhist (handle,tags,bins)

%
%
%        [no,xo] = gettaggedhist (handle, tags [,bins])
%
%
% Assumes transverse orientation
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
% Check the input arguments
%

if (nargin < 2)
  help gettaggedhist
  error ('Too few input arguments.');
elseif (nargin == 2)
  bins = 50;
end

%
% Get the image information
%

slices = getimageinfo(handle,'NumSlices');

if (length(bins) == 1)
  eval(['global Filename',int2str(handle),';']);
  eval(['filename = Filename',int2str(handle),';']);
  mins=mireadvar(filename,'image-min');
  min_val = min(mins);
  maxs=mireadvar(filename,'image-max');
  max_val = max(maxs);
  
  %
  % Calculate the bin limits
  %
  
  binwidth = (max_val - min_val) ./ bins;
  xx = min_val + binwidth*[0:bins];
  xx(length(xx)) = max_val;
  xo = xx(1:length(xx)-1) + binwidth/2;
else
  xo = bins;
end

%
% Initialize no
%

no = zeros (1,length(xo));

%
% Get the voxel coordinates of the tagged points
%

[x_v,y_v,z_v] = world2voxel(handle,tags(:,1),tags(:,2),tags(:,3));

if (min(z_v)<1 | max(z_v)>slices)
  error('Z coordinate is out of range!');
end

%
% Get the histograms
%

disp (['Working on slices ',int2str(min(z_v)),' through ',...
	int2str(max(z_v))]);

for i=min(z_v):max(z_v)

  disp (['Calculating hist for slice ', int2str(i)]);
  
  index = find(z_v==i);
  yi = y_v(index);
  xi = x_v(index);
  mask = zeros(256,256);
  for j=1:length(xi);
    mask(xi(j),yi(j)) = 1;
  end
  MRI = getimages(handle,i);
  [n,x] = hist(reshape(mask,length(MRI),1).*MRI,xo);
  no = no+n;
end

%
% If there are no output arguments, then plot
% the bar graph.
%

if (nargout == 0)
  bar (xo,no);
end
  
