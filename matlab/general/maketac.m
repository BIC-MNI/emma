function tac = maketac (x,y,pet)
% MAKETAC
%
%     tac = maketac(x,y,pet)
%
%     Generate a time-activity curve from a set of data.

if (nargin ~= 3)
  help maketac
  error ('Incorrect number of input arguments');
end

center_pixel = calpix(floor(x), floor(y));
line_length  = length(pet) ^ .5;
if (line_length ~= floor(line_length))
   error ('Image must be square.');
end


tac_roi_loc = [center_pixel-(2*line_length+2):center_pixel-(2*line_length-2) ...
               center_pixel-(line_length+2):center_pixel-(line_length-2) ...
               center_pixel-2  :center_pixel+2   ...
               center_pixel+(line_length-2):center_pixel+(line_length+2) ...
               center_pixel+(2*line_length-2):center_pixel+(2*line_length+2)];
tac = mean(pet(tac_roi_loc,:));
