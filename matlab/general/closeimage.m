function closeimage (handles)

% CLOSEIMAGE  closes image data set(s)
%
%     closeimage (handles)
% 
% Closes one or more image data sets.  If the associated MINC was a
% compressed file (and therefore uncompressed by openimage), then the
% temporary file and directory used for the uncompressed data are
% deleted.

% $Id: closeimage.m,v 1.10 1997-10-20 18:23:19 greg Rel $
% $Name:  $

for handle = handles
   eval(['global Flags' int2str(handle)]);
   eval(['global Filename' int2str(handle)]);
   eval(['Flags = Flags' int2str(handle) ';']);
   eval(['Filename = Filename' int2str(handle) ';']);
   
   if (size(Flags) == [1 2])		% was it actually a compressed file?
      if (Flags(2))                     % then nuke the temp directory
	 slashes = find (Filename == '/');
	 lastslash = slashes (length (slashes));
	 unix (['/bin/rm -rf ' Filename(1:(lastslash-1))]);
      end
   else
      fprintf (2, 'closeimage: warning: invalid image handle (%d)\n', handle);
   end

   eval(['clear global Flags'        int2str(handle)]);
   eval(['clear global Filename'     int2str(handle)]);
   eval(['clear global DimSizes',    int2str(handle)]);
   eval(['clear global FrameTimes'   int2str(handle)]);
   eval(['clear global FrameLengths',int2str(handle)]);
end
