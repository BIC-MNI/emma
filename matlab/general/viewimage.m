function viewimage (img)
% VIEWIMAGE  displays a PET image from a vector or square matrix.
%  viewimage (img) sets the colormap to spectral (the standard PET
%  colormap) and uses MATLAB's image function to display the image.
%  Works on either SGI's or Xterminals, with colours dithered
%  to black and white on the Xterms.
%
%  Note that the colour scaling is currently a little overenthusiastic:
%  any image, regardless of the magnitude of activity in it, will
%  be scaled so its high points are white and its low points black.

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

% img is now square, xsize by xsize.  Set the colormap, and 
% shift/scale img so that it maps onto 1..length(colormap)

colormap (spectral);
num_colors = length (colormap);

lo = min(min(img));
hi = max(max(img));
img = ((img - lo) * ((num_colors-1) / (hi-lo))) + 1;

% Now display it, and fix the y-axis to normal (rather than reverse) dir.

subplot(1,2,1);
image (img');
axis('xy','square');

% For some reason, Matlab didn't want to change the position
% of the figure at this point, so I moved that to the end.....

% Draw a colorbar beside the image

subplot(1,2,2);
image((1:num_colors)');
axis('xy');
set(gca,'Xticklabels', []);
set(gca,'Ytick',0:8:64);
lab = linspace(lo, hi, 9);
for i=1:9, labels = str2mat(labels,num2str(lab(i))); end;
labels(1,:)=[];
set(gca,'Yticklabels', labels);
set(gca,'Position', [.85, .1, .1, .8]);

% Make the main image a reasonable size

subplot(1,2,1);
set(gca,'Position', [.1, .1, .6, .75]);


