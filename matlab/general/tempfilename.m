function fname = tempfilename

% TEMPFILENAME generate a unique temporary filename
%  
%    fname = tempfilename
%
% Requires that a directory /tmp/ exists on the current machine.

% $Id: tempfilename.m,v 1.6 1997-10-20 18:23:22 greg Rel $
% $Name:  $

rand ('seed', sum (100*clock));

timestring = sprintf ('%02d', fix (clock));
filename = sprintf ('/tmp/matimage_%s_%s.dat', ...
   timestring, int2str (rand*1e6));
file_handle = fopen (filename,'r');

% loop until we fail to open the file, ie.
% we find one that *doesn't* exist

while (file_handle ~= -1)
   if (file_handle ~= -1)

      % if file was successfully opened, close it and try another one --
      % we keep going until we find a file that *doesn't* exist

      fclose (file_handle);
      filename = sprintf ('/tmp/matimage_%s_%s.dat', ...
         timestring, int2str (rand*1e6));
      file_handle = fopen (filename, 'r');
   end
end
fname = filename;
