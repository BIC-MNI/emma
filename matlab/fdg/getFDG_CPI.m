function cpi = getFDG_CPI(ts_plasma, plasma, eft)

% To interpolate Ca(t), integrate, and mark for end-frame times.
%
%
%       cpi = getFDG_CPI(ts_plasma, plasma, eft)
%
%
% eft  =  a column vecter of end-frame time. 


%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Check the input arguments

if nargin~=3
  help getFDG_CPI
  error ('Incorrect number of input arguments');
end;


eft=eft(:);
eft=eft(find(eft<=max(ts_plasma) & eft>=min(ts_plasma))); 

aT=[ts_plasma; eft];

saT=sort(aT);
ssaT=saT(find((saT-shift_1(saT))~=0));
ssaT=ssaT(find(ssaT<=max(eft)));

cpi=[ssaT, lookup(ts_plasma,plasma,ssaT), ones(size(ssaT))];


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Mark the end frame times by placing a 0 in the last column

[cpind, cpinc]=size(cpi);

for i=1:1:length(eft);
  if find(ssaT==eft(i))~=[]
    cpi(find(ssaT==eft(i)),cpinc)=0;
  end;
end;
