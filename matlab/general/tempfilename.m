function fname = tempfilename

% TEMPFILENAME generate a unique temporary filename
%  
%         fname = tempfilename
%
% Requires that a directory /tmp/ exists on the current machine.

now = clock;
filename = ['/tmp/matimage', int2str(now), '.dat'];
file_handle = fopen (filename,'r');

while (file_handle ~= -1)			% loop until we fail to open the file, ie.
											% we find one that *doesn't* exist
   if (file_handle ~= -1)			% if file was successfully opened, close it
      fclose (file_handle);		% since we don't really want it open
		filename = ['/tmp/matimage', int2str(now) int2str(1e6*rand) '.dat'];
		file_handle = fopen (filename, 'r');
   end
end
fname = filename;
