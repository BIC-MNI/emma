function [K1_image, K_image, CMRglc_image] = fdg(ts_plasma, plasma, filename, slice)

% FDG  perform an analysis of FDG data
%
%
%      [K1, K, CMRglc] = fdg(ts_plasma, plasma, filename, slice)
%
%
%  filename is the name of the file containing the FDG
%  PET images.
%

%%%%%%%%%%%%%%%%%%%%
% Get the image data

fprintf('\nGetting the image data.\n')
handle = openimage(filename);

MidFTimes = getimageinfo(handle, 'MidFrameTimes')/60;
NumFrames = length(find(MidFTimes<=30));              % We only want the first 30 minutes
MidFTimes = MidFTimes(1:NumFrames);
FrameTimes = getimageinfo(handle, 'FrameTimes')/60;
FrameTimes = FrameTimes(1:NumFrames);
FrameLengths = getimageinfo(handle, 'FrameLengths')/60;
FrameLengths = FrameLengths(1:NumFrames);
PET = getimages(handle, slice, 1:NumFrames);
ImageSize = getimageinfo(handle, 'ImageSize');

closeimage(handle);
pixels = ImageSize(1)*ImageSize(2);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Resample the blood to an even time frame (every 3 seconds).

EndLastFrame = FrameTimes(NumFrames) + FrameLengths(NumFrames);
samples = ceil((EndLastFrame*60 - (ts_plasma(1)*60))/3);
ts_even = linspace (ts_plasma(1), EndLastFrame, samples)';
Ca_even = lookup (ts_plasma, plasma, ts_even);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Find the magic weighting function.

w2 = findweight(ts_even, Ca_even);
MidFWeight2 = MidFTimes + (w2(1) - ts_even(1));


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Set up the beta lookup table

beta1 = (0.5:0.02:5);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Calculate some useful lookup tables

fprintf('Calculating the conv. ints.\n')
[conv_int1, conv_int2] = findconvints_fdg(ts_even, Ca_even, beta1, 1, w2);


%%%%%%%%%%%%%%%%%%%%%%
% Calculate the images

fprintf('\nCalculating the images (initial setup).\n')
V0 = 0.036;
Ca_int1 = V0 .* ntrapz (ts_even, Ca_even);
Ca_int2 = ntrapz(ts_even, igrate(ts_even, Ca_even));
ImLen = size(PET,1);
PET_int1 = ntrapz (MidFTimes, PET')' - Ca_int1;
PET_int2 = ntrapz (MidFTimes, PET' .* (MidFWeight2 * ones(1,ImLen)))';

mask = find(PET_int1>mean(PET_int1));

tau = 1.1;
phi = 0.3;
Kt = 4.8;
Vd = 0.78;

Ca = input('Native glucose measurement: ');

K1_image = zeros(pixels,1);
K_image = zeros(pixels,1);

fprintf('\nCalculating the images (here we go!)\n');
fprintf('Working on %d pixels.\n', length(mask));


for pixel = 1:pixels
    fprintf('.');

    K1_K = PET_int2(pixel) ./ conv_int2;
    K = (PET_int1(pixel) - ((K1_K).*conv_int1))/Ca_int2;
    K1 = K1_K + K;
    
	beta2 = (K1 ./ (K1_K)) .* (K1+((tau*K)./(phi+((tau-phi)*(K./K1)))).*(Ca/Kt))*(1/Vd);

	[y, location] = min(abs(beta1-beta2));
    
	K1_image(pixel) = K1(location);
	K_image(pixel) = K(location);

end;

% CMRglc_image = Ca ./ ((phi./K) + (tau-phi)./K1);
