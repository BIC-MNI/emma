function [Ca_even, delta] = correctblood (A, FrameTimes, FrameLengths, g_even, ts_even, options)

% CORRECTBLOOD  perform delay and dispersion corrections on blood curve
%
%  [Ca_even, delta] = correctblood (A, FrameTimes, FrameLengths, ...
%                                   g_even, ts_even, progress)
%
%  The required input parameters are: 
%      A - brain activity, averaged over all gray matter in a slice.  This
%          should be in units of decay / (gram-tissue * sec), and should
%          just be a vector - one value per frame.
%      FrameTimes - the start time of every frame, in seconds
%      FrameLengths - the length of every frame, in seconds
%      g_even - the (uncorrected) arterial input function, resampled at
%               some *evenly spaced* time domain.  Should be in units
%               of decay / (mL-blood * sec)
%      ts_even - the time domain at which g_even is resampled
%
%  The returned variables are:
%      Ca_even - g_even with dispersion and delay hopefully corrected,
%                in units of decay / (mL-blood * sec).  
%      delay - the delay time (ie. shift) in seconds
%
%  A, FrameTimes, and FrameLengths must all be vectors with the same
%  number of elements (presumably the number of frames in the study).
%  g_even and ts_even must also be vectors with the same number of
%  elements, but their size should be much larger, due to the
%  resampling at half-second intervals performed by resampleblood.
%  
%  correctblood corrects for dispersion in blood activity by
%  calculating g(t) + tau * dg/dt, where tau (the dispersion time
%  constant) is taken to be 4.0 seconds.
%
%  It then attempts to correct for delay by fitting a theoretical blood
%  curve to the observed brain activity A(t).  This curve depends
%  on the parameters alpha, beta, gamma (these correspond to K1, k2,
%  and V0, although for the entire slice rather than pixel-by-pixel) and
%  delta (which is the delay time).  correctblood steps through a series
%  of delta values (currently -5 to +10 sec), and performs a three-
%  parameter fit with respect to alpha, beta, and gamma; the value of
%  delta that results in the best fit is chosen as the delay time.
%
%  options is an entirely optional vector meant for debugging purposes.
%  If options(1) is non-zero, then correctblood will show its progress,
%  by printing out the results of progressive delay-correction fits.  If 
%  it is at least 2, then correctblood will also show progress graphically,
%  by displaying a graph of A(t) and the fits corresponding to every
%  value of delta tried.  If options(2) is zero, then no delay correction
%  will be performed; if options(3) is supplied, then it will be
%  used as delta to do delay correction without the time-consuming
%  fitting.

if ((nargin < 5) | (nargin > 6))
   help correctblood
   error ('Incorrect number of input arguments.')
end

progress = 0;                   % defaults in case of no options vector - 
do_delay = 1;                   % may be overridden below
   
if (nargin == 6)                % options vector

   if (length(options)>=1)      % options vector given; if it has an element
      progress = options(1);    % 1, that will be progress
   end

   if (length(options)>=2)      % delay correction toggle supplied
      do_delay = options(2);
   end

   if (length(options)>=3)      % value to use for delta supplied
      delta = options(3);
   else                         % no delta value given, so use 0
      delta = 0;                
   end

end

if (progress) 
   disp ('Showing progress');
end

if (~do_delay) 
   disp (['No delay-fitting will be performed; will use delta = ' ...
           int2str(delta)]);
end

MidFTimes = FrameTimes + FrameLengths/2;
first60 = find (FrameTimes < 60);           % all frames in first minute only
numframes = length(FrameTimes);

tau = 4;                                    % assumed dispersion time constant

if (progress >= 2)
   figure;
   plot (ts_even, g_even, 'y:');
   title ('Blood activity: dotted=g(t), solid=g(t) + tau*dg/dt');
   drawnow
   hold on
end

% First let's do the dispersion correction: differentiate and smooth
% g(t) by using the method of Sayers described in "Inferring
% Significance from Biological Signals."

[smooth_g_even, deriv_g] = ...
     deriv (3, length(ts_even), g_even, (ts_even(2)-ts_even(1)));
g_even = smooth_g_even + tau*deriv_g;

if (progress >= 2)
   plot (ts_even, g_even, 'r');
   drawnow
end

A = A (first60);                        % chop off stuff after 60 seconds
MidFTimes = MidFTimes (first60);        % first minute again

if (progress >= 2)
   figure;
   plot (MidFTimes, A, 'or');
   hold on
   title ('Average activity across gray matter');
   old_fig = gcf;
   drawnow;
end

%  shifted_g_even = zeros (length(g_even), 1);

% Here are the initial values of alpha, beta, and gamma, in units of:
%  alpha = (mL blood) / ((g tissue) * sec)
%  beta = 1/sec
%  gamma = (mL blood) / (g tissue)
% Note that these differ numerically from Hiroto's suggested initial
% values of [0.6, alpha/0.8, 0.03] only because of the different
% units on alpha of (mL blood) / ((100 g tissue) * min).

init = [.0001 .000125 .03];


if (do_delay)

   if (progress), fprintf ('Performing fits...\n'), end
   deltas = -5:1:10;
   rss = zeros (length(deltas), 1);     % residual sum-of-squares
   params = zeros (length(deltas), 3);  % 3 parameters per fit
   options = [0 0.1];

   for i = 1:length(deltas)
      delta = deltas (i);
      if (progress), fprintf ('delta = %.1f', delta), end

      % Get the shifted activity function, g(t - delta), by shifting g(t)
      % to the right (ie. subtract delta from its actual times, ts_even)
      % and resample at the "correct" times ts_even).  Then do the 
      % three-parameter fit to optimise the function wrt. alpha, beta,
      % and gamma.  Plot this fit.  (Could get messy, but what the hell)


      shifted_g_even = lookup ((ts_even-delta), g_even, ts_even);
      g_select = find (~isnan (shifted_g_even));

      % Be really careful with the fitting.  If the algorithm you choose makes
      % args(2) a negative value, there will be infinities in the result
      % of b_curve, which will cause the entire thing to bomb.

%      options(5) = 1;
%      [final,options,f] = leastsq ('fit_b_curve', init, options, [], ...
%                             shifted_g_even(g_select), ts_even(g_select), ...
%                             A, FrameTimes, FrameLengths);

      final = fmins ('fit_b_curve', init, options, [], ...
                     shifted_g_even (g_select), ts_even (g_select), ...
                     A, FrameTimes, FrameLengths);

      params (i,:) = final;
%     rss (i) = sum (f .^ 2) ;            % if using leastsq
      rss(i) = fit_b_curve (final, ...
                            shifted_g_even(g_select), ts_even(g_select), ...
                            A, FrameTimes, FrameLengths);

      init = final;
      if (progress)
         fprintf ('; final = [%g %g %g]; residual = %g\n', final, rss (i));

         if (progress >= 2)
            plot (MidFTimes, ...
                b_curve(final, shifted_g_even(g_select), ts_even(g_select), ...
                A, FrameTimes, FrameLengths));
            drawnow;
         end      % if graphical progress
      end      % if any progress
   end      % for delta

   [err, where] = min (rss);            % find smallest residual
   delta = deltas (where);                      % delta for best fit

end      % if do_delay

% At this point either we have performed the delay-correction fitting to 
% get delta, or the caller set options(2) to zero so that delay-correction
% was not explicitly done.  In this case, delta will have been set either
% to zero or to options(3).

Ca_even = lookup ((ts_even-delta), g_even, ts_even);

nuke = find(isnan(Ca_even));
Ca_even(nuke) = zeros(size(nuke));

