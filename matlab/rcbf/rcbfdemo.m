function rcbfdemo(slice_number, frame_number)
% RCBFDEMO Demonstrate the EMMA blood analysis package.
%
%   rcbfdemo(slice_number, frame_number)
%
% Hard-coded to use the yates_19445 data
% file, but allows input of a slice and
% frame to display.


if (nargin ~= 2)
  help rcbfdemo
  error ('Incorrect number of input arguments.');
end

if (length(slice_number) ~= 1)
  help rcbfdemo
  error ('<slice_number> must be a scalar.');
end

if (length(frame_number) ~= 1)
  help rcbfdemo
  error ('<frame_number> must be a scalar.');
end
  

disp ('Opening yates_19445 via openimage');
h = openimage ('/usr/local/matlab/toolbox/local/examples/yates_19445.mnc');
nf = getimageinfo(h,'NumFrames');
ns = getimageinfo(h,'NumSlices');
disp (['Image has ' int2str(nf) ' frames and ' int2str(ns) ' slices.']);

if ((slice_number>ns) | (slice_number<1))
  help rcbfdemo
  error ('<slice_number> was out of range.');
end

if ((frame_number>nf) | (frame_number<1))
  help rcbfdemo
  error ('<frame_number> was out of range.');
end

disp (['Reading all images for slice ' int2str(slice_number)]);
pet = getimages (h, slice_number, 1:nf);
set (0, 'DefaultFigurePosition', [100 550 560 420]);
viewimage (pet (:,frame_number));
title (['Here is frame ' int2str(frame_number) ' of slice ' int2str(slice_number)]);


frame_lengths = getimageinfo (h, 'FrameLengths');
frame_times = getimageinfo (h, 'FrameTimes');
summed = pet * frame_lengths;
set (0, 'DefaultFigurePosition', [700 550 560 420]);
figure (gcf+1);
viewimage (summed);
title (['Here is the integrated image: all frames of slice ' int2str(slice_number)]);
drawnow


disp ('TAC generation: Click in the fruit salad to quit.');

current_figure = gcf;
set (0, 'DefaultFigurePosition', [750 250 300 200]);
figure (current_figure+1);
title ('Time-activity curve');
x=100;y=100;

while ((x>20) & (y>20))
  disp ('Now, pick a pixel and I will make a time activity curve');
  figure(current_figure);
  [x,y] = getpixel(1);
  activity = maketac (x,y,pet);
  figure(current_figure+1);
  plot (frame_times, activity);
  drawnow
end

closeimage (h);
delete(gcf);

disp (['Now calculating K1, k2, and V0 images for slice' int2str(slice_number)]);
set (0, 'DefaultFigurePosition', [100 550 560 420]);
figure;
cpustart = cputime;
tic;

[K1, k2, V0, delay] = rcbf2('/usr/local/matlab/toolbox/local/examples/yates_19445.mnc', ...
                            slice_number, 2, 1);
cpu_elapsed = cputime - cpustart;
user_elapsed = toc;

disp (['That took ' int2str(cpu_elapsed) ' seconds of CPU time while ']);
disp ([int2str(user_elapsed) ' seconds elapsed in "reality".']);

set (0, 'DefaultFigurePosition', [100 50 560 420]);
figure (gcf+1);

viewimage (K1);
title ('Here is the K1 image as calculated within MATLAB');







