function [Ca_even, delta, A] = correctblood (pet_images, ftimes, flengths, g_even, ts_even, progress)

% CORRECTBLOOD  perform delay and dispersion corrections on blood curve


error (nargchk (5, 6, nargin));
if (nargin < 6), progress = 0; end

if (progress) disp ('Showing progress'), end

midftimes = ftimes + flengths/2;
first60 = find (ftimes < 60);		% all frames in first minute only
numframes = length(ftimes);

if (progress)
   plot (ts_even, g_even, ':');
   drawnow
   hold on
end

% First let's do the dispersion correction: differentiate g (t) by
% smoothing it and taking differences, add subtract tau * dg/dt to g.

% smoothed will have length = length(g_even) + 4; want to lose first two
% and last two values to make it "match up" to g_even.
smoothed = conv (g_even, [-3 12 17 12 -3]); 
smoothed = smoothed (3:length(smoothed)-2) / 35;
deriv = diff (smoothed) ./ diff (ts_even);
g_even = g_even(1:length(g_even)-1) + 4 * deriv;
z = find (g_even < 0);                       % nuke negatives from g_even
g_even (z) = zeros (size (z));
ts_even = ts_even(1:length(ts_even)-1);      % cut ts_even down to size to
                                             % match g_even

if (progress)
   plot (ts_even, g_even);
   drawnow
   figure;
end

% Now g_even is shortened by one element, but has been corrected for
% dispersion (assuming dispersion time constant tau = 4 sec).

% Find a mask to get just gray matter.

summed = pet_images * flengths;
mask = summed>(1.8*mean(summed)); % dumb but quick
%mask = getmask(summed);                % better but interactive
clf					% clear figure 

A = mean (pet_images (mask, :))' * (1.05^2) / 37;   % fix units
A = A (first60); 			     % chop off stuff frames after
midftimes = midftimes (first60);	     % first minute

if (progress)
   plot (midftimes, A, 'or');
   hold on
   title ('Average activity across gray matter');
   old_fig = gcf;
   drawnow;
end

shifted_g_even = zeros (length(g_even), 1);

% Here are the initial values of alpha, beta, and gamma, in units of:
%  alpha = (mL blood) / ((g tissue) * sec)
%   beta = 1/sec
%  gamma = (mL blood) / (g tissue)
% Note that these differ numerically from Hiroto's suggest initial 
% values of [0.6, alpha/0.8, 0.03] only because of the different
% units on alpha of (mL blood) / ((100 g tissue) * min).

init = [.0001 .000125 .03];

if (progress), fprintf ('Performing fits...\n'), end
deltas = -5:1:10;
rss = zeros (length(deltas), 1);	% residual sum-of-squares
params = zeros (length(deltas), 3);	% 3 parameters per fit
options = [0 0.1];

for i = 1:length(deltas)
   delta = deltas (i);
   if (progress), fprintf ('delta = %.1f', delta), end
   
   % Get the shifted activity function, g(t - delta), by shifting g(t)
   % to the right (ie. subtract delta from its actual times, ts_even)
   % and resampling at the "correct" times ts_even.  Then do the 
   % three-parameter fit to optimise the function wrt. alpha, beta,
   % and gamma.  Plot this fit.  (Could get messy, but what the hell)

   shifted_g_even = lookup ((ts_even-delta), g_even, ts_even);
%   [final,options,f] = leastsq ('fit_b_curve', init, options, [], ...
%	 shifted_g_even, ts_even, A, midftimes);
   final = fmins ('fit_b_curve', init, options, [], ...
	 shifted_g_even, ts_even, A, midftimes);

   fit_b_curve (final, shifted_g_even, ts_even, A, midftimes);
   params (i,:) = final;
%   rss (i) = sum (f .^ 2) ;
   rss(i) = fit_b_curve (final, shifted_g_even, ts_even, A, midftimes);
   if (progress)
      fprintf ('; final = [%g %g %g]; error = %g\n', ...
	    final, rss (i));
      plot (midftimes, ...
            b_curve(final, shifted_g_even, ts_even, A, midftimes));
      drawnow

   end
            
end

[err, where] = min (rss);		% find smallest residual
delta = deltas (where);			% delta for best fit
Ca_even = lookup ((ts_even-delta), g_even, ts_even);
%fit_params = params (where,:);		% alpha, beta, gamma for best fit
%rss = err;				% return just the minimum residual

