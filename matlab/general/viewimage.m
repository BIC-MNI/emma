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
disp (['Img min is ' int2str(min(min(img)))]);
disp (['Img max is ' int2str(max(max(img)))]);

% Now display it, and fix the y-axis to normal (rather than reverse) dir.
image (img');
set (gca, 'YDir', 'normal');
