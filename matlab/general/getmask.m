function mask = getmask (image)
% GETMASK returns a mask that is the same size as the passed image.
%         The mask consists of 0's and 1's, and is created interactively
%         by the user.  Currently, a threshold algorithm is used, 
%         based on the input argument image: the user selects a 
%         threshold using a slider (the default starting value is 
%         1.8), and getmask selects all points in image greater than
%         the mean value of the entire image multiplied by threshold 
%         the threshold.  It then displays image as masked by
%         that threshold value, so the user can refine the threshold
%         to his/her satisfaction.

if (nargin ~= 1)
   help getmask;
   error ('Incorrect number of input arguments.');
end

img = image;                            % copy because it is made global
mu = mean(mean(img)); 			% in case it happens to be square
threshold = 1.8; 			% initial value (from Hiroto)
temp = img > threshold * mu; 		% "binary" temp - all 1's and 0's
[fh, iah] = viewimage (temp .* img); 	% returns handles: figure, image axes

pos = get (iah, 'Position');
pos (2) = pos(2) + 1.05*pos (4);	% assuming normalised units
pos (4) = .05;

tobj = text('units', 'normal', 'position', [.5 1.15], 'string', num2str(threshold));

slider_cmd = ['global slider mu tobj temp img threshold;'...
              'threshold = get (slider, ''value'');'...
              'temp = img > threshold*mu;'...
	      'viewimage (img .* temp, 0);'...
	      'tobj = text(''units'', ''normal'', ''position'', [.5, 1.15], ''string'', num2str(threshold));'];
slider = uicontrol ('Style', 'slider', 'units', 'normal', 'Position', pos,...
      'min', 1, 'max', 3, 'value', threshold, 'CallBack', slider_cmd);

global slider mu tobj temp img threshold
drawnow;
input ('Press [Enter] when done.');
delete (slider); 
delete (tobj);
mask = temp;
clear global slider mu tobj img temp threshold
