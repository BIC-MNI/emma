function [fig_handle, image_handle, bar_handle] = viewimage (img, update, colourbar)
% VIEWIMAGE  displays a PET image from a vector or square matrix.
%
%    [fig_handle, image_handle, bar_handle] = viewimage (img, update,
%                                                        colourbar_flag)
%
%  viewimage (img) displays an image using the MATLAB image function.
%  Works on either colour or monochrome displays.
%
%  Images are scaled so that the high points are white and the low
%  points black.
%
%  viewimage (img, update) chooses the update mode.  The default is to erase
%  all elements of the figure window, and recreate everything.  By
%  specifying update = 1, only the image itself will be changed.
%
%  viewimage (img, [], colourbar_flag) turns the colourbar on or off.
%  The default is on, but by specifying colourbar flag = 0, the
%  colourbar will be turned off.
%

%  Copyright 1993,1994 Mark Wolforth and Greg Ward, McConnell Brain
%  Imaging Centre, Montreal Neurological Institute, McGill
%  University.
%  Permission to use, copy, modify, and distribute this
%  software and its documentation for any purpose and without
%  fee is hereby granted, provided that the above copyright
%  notice appear in all copies.  The authors and McGill University
%  make no representations about the suitability of this
%  software for any purpose.  It is provided "as is" without
%  express or implied warranty.


if (nargin < 1)
  help viewimage
  error ('Too few input arguments.');
elseif (nargin == 1)
  update = 0;
  colourbar = 1;
elseif (nargin == 2)
  colourbar = 1;
  if (length (update) == 0)
    update = 0;
  end;
end;

% Reshape the image appropriately
[x,y] = size (img);

if ((x > 1) & (y > 1))
    xsize = x;
else
    xsize= x^.5;
    if (xsize ~= floor (xsize))
        error('Image must be square.');
    end
    if (y ~= 1)
        error('Image must be a vector if not square.');
    end
    img = reshape (img, xsize, xsize);
end

% If any NaN's or infinities are present in the image, find the min/max
% of the image *without* them, and assign them all to the minimum -- that 
% way they will display as black.

nuke = (isnan (img) | isinf (img));
if any (any (nuke))
  lo = min(img(~nuke));
  hi = max(img(~nuke));
  disp ('viewimage warning: image contains NaN''s and/or infinities');
  nuke = find(nuke);
  img(nuke) = zeros (size (nuke));
else
  lo = min(min(img));
  hi = max(max(img));
end


% Set the default colourmap, and shift/scale img so that it maps onto
% 1..length(colourmap).  We have different setups depending on whether
% or not we are diplaying in colour.

if (~update)

  % Clean everything off the current figure
  delete(get(gcf,'Children'));
  
  if (get (0, 'ScreenDepth') > 1)
    default_colormap = ['colormap (spectral)'];
    
    b = ['colormap(spectral)'];
    uicontrol('Units','normal','Position',[.12 0.87 .09 .04], ...
	'String','Spectral','callback',b)
    h = ['colormap(hotmetal)'];
    uicontrol('Units','normal','Position',[.22 0.87 .09 .04], ...
	'String','Hot','callback',h)
    g = ['colormap(gray)'];
    uicontrol('Units','normal','Position',[.32 0.87 .09 .04], ...
	'String','Gray','callback',g)
    u = ['brighten(0.3)'];
    uicontrol('Units','normal','Position',[.42 0.87 .09 .04], ...
	'String','Bright','callback',u)
    l = ['brighten(-0.3)'];
    uicontrol('Units','normal','Position',[.52 0.87 .09 .04], ...
	'String','Dark','callback',l)
  else
    default_colormap = ['colormap (gray .^ 1.5)'];
    
    u = ['brighten(0.3)'];
    uicontrol('Units','normal','Position',[.12 0.87 .09 .04], ...
	'String','Bright','callback',u)
    l = ['brighten(-0.3)'];
    uicontrol('Units','normal','Position',[.22 0.87 .09 .04], ...
	'String','Dark','callback',l)
    uicontrol('Units','normal','Position',[.32 0.87 .09 .04], ...
	'String','Default','callback',default_colormap)
  end
  eval(default_colormap);
end

num_colors = length (colormap);
img = ((img - lo) * ((num_colors-1) / (hi-lo))) + 1;

% Now display it, and fix the y-axis to normal (rather than reverse) dir.

fig_handle = gcf;

% Draw a colourbar beside the image
  
if (colourbar)

  if (~update)
    bar_handle = subplot(1,2,2);
    image((1:num_colors)');
    axis('xy');
    yticks = linspace (0, num_colors, 9);
    set(bar_handle,'Xticklabels',[],'Ytick',yticks, ...
	'Position',[.85,.1,.1,.8],'UserData','ColourBar');
  else
    children = get (gcf,'children');
    for i=1:length(children)
      ident_tag = get(children(i), 'UserData');
      if (length(ident_tag) > 0)
	
	% Right now, if the tag exists, then assume that
	% this is the colourbar
	
	bar_handle = children(i);
      end
    end
  end

  lab = linspace(lo, hi, 9);
  for i=1:9, labels = str2mat(labels,num2str(lab(i))); end;
  labels(1,:)=[];
  set(bar_handle,'Yticklabels', labels);
end

% We draw the image last so that it will be the
% current axis

image_handle = subplot(1,2,1);
image (img');

% Set the direction of the axes to what we're used to, and
% make the aspect ratio square
axis('xy','square');

% Make the main image a reasonable size
set(image_handle,'Position', [.1, .1, .6, .75]);
