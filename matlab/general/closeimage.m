function closeimage (handles)

% CLOSEIMAGE  closes image data set(s)
%
%     closeimage (handles)
% 
% Closes one or more image data sets.  If the associated MINC was a
% compressed file (and therefore uncompressed by openimage), then the
% temporary file and directory used for the uncompressed data are
% deleted.

% $Id: closeimage.m,v 1.11 2000-04-10 16:00:50 neelin Exp $
% $Name:  $

for handle = handles
   Flags = handlefield(handle, 'Flags');
   Filename = handlefield(handle, 'Filename');
   
   if (size(Flags) == [1 2])		% was it actually a compressed file?
      if (Flags(2))                     % then nuke the temp directory
	 slashes = find (Filename == '/');
	 lastslash = slashes (length (slashes));
	 unix (['/bin/rm -rf ' Filename(1:(lastslash-1))]);
      end
   else
      fprintf (2, 'closeimage: warning: invalid image handle (%d)\n', handle);
   end

   handlefield(handle, 'Free');
end
