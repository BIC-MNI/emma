function viewimage (image)
% VIEWIMAGE (image) takes a vector or a square matrix
%           containing an image.  On an SGI, it will
%           display the image using Matlab's pcolor
%           function.  On any other display, it saves
%           the data in a temporary file, converts the
%           data to a PGM, and then displays the
%           result using XV.

[x,y] = size (image);


if (x==y)
    xsize = x;
else
    xsize= x^.5;
    if (y ~= 1)
        error('Image must be a vector or square.');
    end
end


% Figure out if we have a colour machine or a monochrome machine

display = getenv('DISPLAY');

if (strcmp(display,':0.0') | strcmp(display(1:4),'lear') | strcmp(display(1:4),'pria') | strcmp(display(1:4),'dunc') | strcmp(display(1:4),'port'))

    pcolor (reshape(image,xsize,xsize)'); colormap (spectral); shading flat;

else

    % Assume a monochrome X-term
    % Rotate the image and reshape it to a square

    newimage = image - min(min(image));
    newimagemax = max(max(newimage));
    if (newimagemax == 0)
       disp ('Warning: image is all zeros');
    else
        newimage = newimage .* (255/newimagemax);
    end

    eval(['newimage = reshape (newimage,' int2str(xsize) ',' int2str(xsize) ');' ]);
    newimage = fliplr (newimage);
    eval(['newimage = reshape (newimage,' int2str(x) ',' int2str(y) ');' ]);


    % Find a unique file name

    filename = tempfilename;


    % Write the data to a temporary file

    file_handle = fopen (filename, 'w');
    fwrite (file_handle, newimage, 'char');
    fclose (file_handle);

    eval(['!rawtopgm ' int2str(xsize) ' ' int2str(xsize) ' ' filename ' | xv - &'])

end


