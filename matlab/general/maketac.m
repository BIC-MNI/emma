function tac = maketac (x,y,pet)
% MAKETAC
%
%     Generate a time-activity curve from a set of data.
%
%     tac = maketac(x,y,pet)
%

if (nargin ~= 3)
  help maketac
  error ('Incorrect number of input arguments');
end

center_pixel = calpix(floor(x), floor(y));
tac_roi_loc = [center_pixel-258:center_pixel-254 ...
               center_pixel-130:center_pixel-126 ...
               center_pixel-2  :center_pixel+2   ...
               center_pixel+126:center_pixel+130 ...
               center_pixel+254:center_pixel+258];
tac = mean(pet(tac_roi_loc,:));
