function closeimage (handles)

% CLOSEIMAGE  closes image data set(s)
%
%     closeimage (handles)
%
% Closes one or more image data sets.

for handle = handles
   eval(['global Flags' int2str(handle)]);
   eval(['global Filename' int2str(handle)]);
   eval(['Flags = Flags' int2str(handle) ';']);
   eval(['Filename = Filename' int2str(handle) ';']);
   
   if (Flags(2)) 				  % compressed file?
      delete (Filename);
   end
   
   eval(['clear global Flags'        int2str(handle)]);
   eval(['clear global Filename'     int2str(handle)]);
   eval(['clear global DimSizes',    int2str(handle)]);
   eval(['clear global FrameTimes'   int2str(handle)]);
   eval(['clear global FrameLengths',int2str(handle)]);
end
   
