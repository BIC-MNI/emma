function y = igrate(t,x)

% IGRATE performs a piecewise linear integration
%
%
%         y = igrate (t,x) 
%
%
% integration of each of the columns of X along the
% vector T.  If X is M-by-N then T must be M. IGRATE
% returns a vector Y containing cumulative integrals
% at each element of T.
%
% The time spacing must be even.

if length(t) ~= length(x)
  error('t and x must have the save length.');
end


dt = t(2) - t(1);
bins = x .* dt;

y(1) = bins(1);
for i=2:length(t)
  y(i) = y(i-1) + bins(i);
end
