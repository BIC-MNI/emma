function closeimage (handle)

eval(['clear global Filename'     int2str(handle)]);
eval(['clear global NumFrames',   int2str(handle)]);
eval(['clear global NumSlices',   int2str(handle)]);
eval(['clear global ImageSize',   int2str(handle)]);
eval(['clear global PETimages'    int2str(handle)]);
eval(['clear global FrameTimes'   int2str(handle)]);
eval(['clear global FrameLengths',int2str(handle)]);
eval(['clear global AvailFrames', int2str(handle)]);
eval(['clear global AvailSlices', int2str(handle)]);
eval(['clear global CurLine',     int2str(handle)]);
