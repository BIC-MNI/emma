function integral = b_curve (args, shifted_g_even, ts_even, A, midftimes)

% global shifted_g_even ts_even A flengths ftimes midftimes

% N.B.: alpha = args(1)
%        beta = args(2)
%       gamma = args(3)

% Now calculate exp (-beta * t) in the ts_even time domain, and perform
% the convolution with the *shifted* activity g(t-delta).

if (length(args) ~= 3), error ('Wrong number of fit parameters'), end;

expthing = exp(-args(2)*ts_even); 	% ts_even for the convolution
c = conv(shifted_g_even,expthing);
c = c (1:length(ts_even));		% chop off points outside of the time
					% domain we're interested in

% Calculate the two main terms of the expression, alpha * (convoluted
% stuff) and gamma * (shifted activity), and sum them.
   
i1 = args(1)*c;				% alpha * (convolution)
i2 = args(3)*shifted_g_even;		% gamma * g(t - delta)

i = i1+i2;

% Note that we don't have to explicitly integrate here, because we
% multiply by flengths to integrate, and then divide by flengths to
% normalise.

integral = lookup (ts_even,i,midftimes);
integral = integral (1:length(A));
