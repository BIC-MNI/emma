function cCa = cnvCa(t,Ca,b)

% To calculate cCa(t) = int_0^t Ca(u)*exp(-b(t-u))du
%
%       cCa = cnvCa(t,Ca,b)
%
% t,Ca,b are column vectors.
%
% It is recommended to interpolate Ca(t) with dt <= 0.1 min.
% When Ca is the integral of a function (Ca'), cCa (output of this code) is
% equal to the integral of convolution of Ca', if finely sampled.

Lb=length(b);
Lt=length(t);
q=tril(ones(Lt));

u=Ca*ones(1,Lb).*exp(t*b');
u2=(u+shift_1(u));
dt=(t-shift_1(t))/2;
u3=u2.*(dt*ones(1,Lb));
ui=q*u3;
cCa=ui.*exp(-t*b');
