function [points1, points2] = loadtagfile(tagfile);
% loadtagfile: Load a tag file
%
% [points1, points2] = loadtagfile(tagfile);
%
% Input : tagfile - name of input tag file
%
% Output : points1 - list of first set of homogeneous coordinates 
%                    (stored as rows)
%          points2 - list of second set of homogeneous coordinates
%
junkname='tagtoxfm_junk';
junkdir ='/tmp/';
junkfile = [junkdir,junkname];

% Test for existence of the file
if (~exist(tagfile))
   error(['File ',tagfile,' not found']);
end

% Load the points
eval(['! sed ''s/;//g'' ',tagfile,' | ', ...
   'awk ''( \!/^MNI Tag Point File$/ && \!/^Volumes/ && \!/^Points/ && ', ...
   '\!/^#/ && \!/^%/ && \!/^$/) {print $1,$2,$3,$4,$5,$6}''  > ',junkfile]);
eval(['load -ascii ',junkfile]);
eval(['pts = ',junkname,';']);
eval(['delete ',junkfile]);

% Extract the points
[m,n] = size(pts);

if ((n ~= 3) & (n ~= 6))
   error('Wrong number of coordinate fields in file');
end
points1 = [pts(:,1:3) ones(m,1)];

if (n == 6) 
   points2 = [pts(:,4:6) ones(m,1)];
else
   points2 = [];
end
