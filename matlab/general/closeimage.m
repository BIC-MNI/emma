function closeimage (handles)

% CLOSEIMAGE  closes image data set(s)
%
%     closeimage (handles)
% 
% Closes one or more image data sets.  If the associated MINC was a
% compressed file (and therefore uncompressed by openimage), then the
% temporary file and directory used for the uncompressed data are
% deleted.

for handle = handles
   eval(['global Flags' int2str(handle)]);
   eval(['global Filename' int2str(handle)]);
   eval(['Flags = Flags' int2str(handle) ';']);
   eval(['Filename = Filename' int2str(handle) ';']);
   
   if (size(Flags) == [1 2])		% make sure it was actually an open
      if (Flags(2))			% compressed file?
	 delete (Filename);
	 slashes = find (Filename == '/');
	 lastslash = slashes (length (slashes));
	 delete (Filename(1:(lastslash-1)));
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
