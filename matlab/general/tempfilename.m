function fname = tempfilename

% TEMPFILENAME generate a unique temporary filename
%  
%         fname = tempfilename
%
% Requires that a directory /tmp/ exists on the current machine.

now = clock;
filename = sprintf ('/tmp/matimage%d%d%d%d%d%s.dat', now(1:5),int2str(now(6)));
file_handle = fopen (filename,'r');

% loop until we fail to open the file, ie.
% we find one that *doesn't* exist

while (file_handle ~= -1)
   if (file_handle ~= -1)

	% if file was successfully opened, close it

      	fclose (file_handle);
	filename = sprintf ('/tmp/matimage%d%d%d%d%d%s%s.dat', now(1:5),int2str(now(6)), int2str(1e6*rand));
	file_handle = fopen (filename, 'r');
   end
end
fname = filename;
