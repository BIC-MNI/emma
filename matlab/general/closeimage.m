function closeimage (handles)

% CLOSEIMAGE  closes image data set(s)
%
%     closeimage (handles)
%
% Closes one or more image data sets.

for handle = handles
   eval(['clear global Filename'     int2str(handle)]);
   eval(['clear global DimSizes',    int2str(handle)]);
   eval(['clear global FrameTimes'   int2str(handle)]);
   eval(['clear global FrameLengths',int2str(handle)]);
end
   
