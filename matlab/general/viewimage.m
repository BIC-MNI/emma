function [fig_handle, image_handle, bar_handle] = viewimage (img, colourbar)
% VIEWIMAGE  displays a PET image from a vector or square matrix.
%
%    [fig_handle, image_handle, bar_handle] = viewimage (img, colourbar_flag)
%
%  viewimage (img) sets the colourmap to spectral (the standard PET
%  colourmap) and uses MATLAB's image function to display the image.
%  Works on either SGI's or Xterminals, with colours dithered
%  to black and white on the Xterms.
%
%  Images are scaled so its high points are white and its low points black.
%
%  viewimage (img, colourbar_flag) turns the colourbar on or off.
%  The default is on, but by specifying colourbar flag = 0, the
%  colourbar will be turned off.
%


[x,y] = size (img);

if (x==y)
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

if (nargin==1)
  colourbar = 1;
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


% img is now square, xsize by xsize.  Set the colourmap, and 
% shift/scale img so that it maps onto 1..length(colourmap)

if (strcmp (get (0, 'BlackAndWhite'), 'off'))
   colormap (spectral);
else
   colormap (gray .^ 1.5);           % darken it a little
end
num_colors = length (colormap);

img = ((img - lo) * ((num_colors-1) / (hi-lo))) + 1;

% Now display it, and fix the y-axis to normal (rather than reverse) dir.

fig_handle = gcf;

image_handle = subplot(1,2,1);
image (img');
axis('xy','square');

% For some reason, Matlab didn't want to change the position
% of the figure at this point, so I moved that to the end.....

% Draw a colourbar beside the image

if (colourbar)
  bar_handle = subplot(1,2,2);
  image((1:num_colors)');
  axis('xy');
  set(gca,'Xticklabels', []);
  yticks = linspace (0, num_colors, 9);
  set(gca,'Ytick',yticks);

  lab = linspace(lo, hi, 9);
  for i=1:9, labels = str2mat(labels,num2str(lab(i))); end;
  labels(1,:)=[];
  set(gca,'Yticklabels', labels);
  set(gca,'Position', [.85, .1, .1, .8]);
end

% Make the main image a reasonable size

subplot(1,2,1);
set(gca,'Position', [.1, .1, .6, .75]);


