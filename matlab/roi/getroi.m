function [Xi,Yi] = getroi (handle)

%
%
%      [Xi,Yi] = getroi (handle)
%
%

setHandle = floor(handle/100);

eval (['global ROIs',int2str(setHandle)]);
eval (['ROIs = ROIs',int2str(setHandle),';']);

roiNumber = handle - (100*setHandle);

index = find(ROIs==-1);

Vertices = ROIs((index(roiNumber)+1):(index(roiNumber+1)-1));
numVertices = length(Vertices)/2;

Xi = Vertices(1:numVertices);
Yi = Vertices((numVertices+1):(length(Vertices)));

