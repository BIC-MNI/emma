function integral = b_curve (args)

global g_even ts_even A flengths ftimes midftimes

expthing = exp(-args(2)*ts_even);	% ts_even for the convolution

% N.B.: alpha = args(1)
%        beta = args(2)
%       gamma = args(3)
%       delta = args(4)

% This gets g(t - delta) from g(t)

shifted_g_even = lookup ((ts_even-args(4)), g_even, ts_even);

c = conv(shifted_g_even,expthing);

i1 = args(1)*c;				% alpha * (convolution)
i2 = args(3)*shifted_g_even;		% gamma * g(t - delta)

i1 = i1(1:length(i2));			% chop off excess convoluted points
i = i1+i2;				% all are now length of g_even

i = lookup (ts_even,i,midftimes);
% i = i (ts_even < 60);			% chop off times > 60 sec to match A

% ts_interval = ts_even(2) - ts_even(1); % assumes ts_even is evenly spaced!
integral = zeros (size (A));

%for frm=1:length(A)
%   in_range = (ts_even >= ftimes(frm)) & (ts_even <= ftimes(frm+1));
%   integral (frm) = trapz (ts_even(find(in_range)), i(find(in_range)));
%   integral(frm) = sum (i (find(in_range))) * ts_interval;
%   integral (frm) = i (frm) * flengths (frm);
%end

integral = i .* flengths;
integral = integral(1:length(A)) ./ flengths(1:length(A));



