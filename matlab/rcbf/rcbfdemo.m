function demo(slice_number, frame_number)
% DEMO
%       Demontrate the PAMI blood analysis package.
%       Hard-coded to use the arnaud_20547 data
%       file, but allows input of a slice and
%       frame to display.
%
%    Usage: demo(slice_number, frame_number)
%


if (nargin ~= 2)
  help demo
  error ('Incorrect number of input arguments.');
end

if (length(slice_number) ~= 1)
  help demo
  error ('<slice_number> must be a scalar.');
end

if (length(frame_number) ~= 1)
  help demo
  error ('<frame_number> must be a scalar.');
end
  

disp ('Opening arnaud_20547 via openimage');
h = openimage ('/usr/people/wolforth/matlab/images/arnaud_20547.mnc');
nf = getimageinfo(h,'NumFrames');
ns = getimageinfo(h,'NumSlices');
disp (['Image has ' int2str(nf) ' frames and ' int2str(ns) ' slices.']);

if ((slice_number>ns) | (slice_number<1))
  help demo
  error ('<slice_number> was out of range.');
end

if ((frame_number>nf) | (frame_number<1))
  help demo
  error ('<frame_number> was out of range.');
end

disp (['Reading all images for slice ' int2str(slice_number)]);
pet = getimages (h, slice_number, 1:nf);

viewimage (pet (:,frame_number));
title (['Here is frame ' int2str(frame_number) ' of slice ' int2str(slice_number)]);
set (gcf, 'Position', [100 550 560 420]);

frame_lengths = getimageinfo (h, 'FrameLengths');
frame_times = getimageinfo (h, 'FrameTimes');
summed = pet * frame_lengths;
figure (gcf+1);
set (gcf, 'Position', [700 550 560 420]);
viewimage (summed);
title (['Here is the integrated image: all frames of slice ' int2str(slice_number)]);
drawnow


disp ('TAC generation: Click in the fruit salad to quit.');

current_figure = gcf;
figure (current_figure+1);
title ('Time-activity curve');
x=100;y=100;

while ((x>20) & (y>20))
  disp ('Now, pick a pixel and I will make a time activity curve');
  figure(current_figure);
  [x,y] = ginput (1);
  activity = maketac (x,y,pet);
  figure(current_figure+1);
  plot (frame_times, activity);
  drawnow
end

closeimage (h);

disp (['Now calculating K1 and k2 images for slice' int2str(slice_number)]);
cpustart = cputime;
tic;

[K1, k2] = rcbf1 ('/usr/people/wolforth/matlab/images/arnaud_20547.mnc', slice_number);
cpu_elapsed = cputime - cpustart;
user_elapsed = toc;

disp (['That took ' int2str(cpu_elapsed) ' seconds of CPU time while ']);
disp ([int2str(user_elapsed) ' seconds elapsed in "reality".']);

figure (gcf+1);
set (gcf, 'Position', [100 50 560 420]);
viewimage (K1);
title ('Here is the K1 image as calculated within MATLAB');

disp ('Finally, reading the previously calculated K1 image...');
disp ('(which, you may recall, takes 5 min/slice to calculate on the VAX)');

h2 = openimage ('/usr/people/wolforth/matlab/images/arnaud_20547.k1.mnc');
k12 = getimages (h2, slice_number);

figure(gcf+1);
set (gcf, 'Position', [700 50 560 420]);
viewimage (k12);
title (['This is the previously calculated K1 for slice ' int2str(slice_number)]);

closeimage (h2);
