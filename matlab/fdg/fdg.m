function [K1_image, K_image, CMRglc_image] = fdg (filename, slices, glucose, ...
                                                  progress, ts_plasma, plasma)

% FDG  perform an analysis of FDG data
%
%
%      [K1,K,CMRglc] = fdg (filename, slice [,glucose ...
%	                    [,progress[,ts_plasma, plasma]]])
%
%
% FDG implements the weighted integral method of calculating K1,
% K, and CMRglc (in that order) for a particular slice.  It first
% reads in a great mess of data (viz., the brain activity for
% every frame of the slice, frame start times and lengths, plasma
% activity, and blood sample times).  The data supplied to FDG
% is:
%
%   filename  - The name of the file containing the FDG PET
%               images.
%   slice     - The slice (or vector of slices) to process.
%   glucose   - Optional specification of the native glucose
%               measurement (in umol/ml).
%   progress  - If 1, the function prints progress information
%               during the analysis.  If 0, the function analyzes
%               the data silently.
%   ts_plasma - The plasma measurement times.
%   plasma    - The plasma activity data.
%
% If the plasma times and activity are not supplied, this
% function will attempt to retrieve them from the BNC file
% associated with the image.


%  Copyright 1994 Mark Wolforth, McConnell Brain Imaging Centre,
%  Montreal Neurological Institute, McGill University.
%  Permission to use, copy, modify, and distribute this software
%  and its documentation for any purpose and without fee is
%  hereby granted, provided that the above copyright notice
%  appear in all copies.  The author and McGill University make
%  no representations about the suitability of this software for
%  any purpose.  It is provided "as is" without express or
%  implied warranty.



%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Check the input arguments

if (nargin < 2)
  help fdg
  error ('Insufficient number of input arguments.');
elseif (nargin == 2)
  glucose = [];
  plasma = [];
  ts_plasma = [];
  progress = 0;
elseif (nargin == 3)
  plasma = [];
  ts_plasma = [];
  progress = 0;
elseif (nargin == 4)
  plasma = [];
  ts_plasma = [];
elseif (nargin == 5)
  help fdg
  error ('Both plasma sample times AND activity must be supplied.');
end;


%%%%%%%%%%%%%%%%%%%%%%%%%%
% Default parameter values

tau = 1.1;
phi = 0.25;
Kt = 4.1;
Vd = 0.82;
v0 = 0.036;
c_time = 30;    % Circulation time in minutes

%%%%%%%%%%%%%%%%%%%%
% Get the study info

if (progress)
  disp ('Getting the image data....');
end;
  
handle = openimage(filename);

EndFTimes = (getimageinfo(handle, 'FrameTimes') + ...
    getimageinfo(handle, 'FrameLengths')) / 60;

NumFrames = length(EndFTimes);
ImageSize = getimageinfo(handle, 'ImageSize');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% If not spuulied, get the blood data from the .BNC file

if (length(plasma)==0)
  [plasma, ts_plasma] = getblooddata(handle);

  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  %Perform units correction on the blood data

  plasma = plasma ./ 10.5;
end;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Resample the blood to an even time frame.  Include the
% end of frame times in the resampled data.

if (progress)
  disp ('Preparing the blood...');
end;
[ts_new, plasma_new] = getFDGplasma (ts_plasma, plasma, EndFTimes);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Get the native glucose measurement

if (length(glucose) == 0)
  glucose = miinquire(filename, 'attvalue', ...
      'blood_analysis', 'glucose');
end;


%%%%%%%%%%%%%%%%%%%%%%%%
% As a last resort, ask!

if (length(glucose) == 0)
  glucose = input ('What is the native glucose (umol/ml):');
end;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Initialize the images to zero

if (progress)
  disp ('Initializing the images...');
end;

pixels = ImageSize(1)*ImageSize(2);

total_slices = length(slices);
K1_image = zeros(pixels,total_slices);
K_image = zeros(pixels,total_slices);
CMRglc_image = zeros(pixels,total_slices);


%%%%%%%%%%%%%%%%%%%%%%%
% Solve the FDG problem

for current_slice=1:total_slices

  if (progress)
    disp (['Doing slice ', int2str(slices(current_slice))]);
  end

  if (progress)
    disp ('Reading image information.');
  end
  
  PET = getimages(handle, slices(current_slice), 1:NumFrames, PET);

  if (progress)
    disp ('Calling solveFDG...');
  end
  
  [K1_image(:,current_slice), K_image(:,current_slice), ...
	  CMRglc_image(:,current_slice)] = ...
      solveFDG (PET, ts_new, plasma_new, EndFTimes, c_time, glucose, ...
      v0, [1 3 1 10]', [tau phi Kt Vd]);
end;

closeimage(handle);
