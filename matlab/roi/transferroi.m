function transferroi (child_fig, parent_fig, line_color)

%
%
%      transferroi (child_fig [,parent_fig[,line_color]])
%
%


if (nargin<1)
  help transferroi
  error ('Too few input arguments.');
elseif (nargin<2)
  parent_fig = gcf;
  line_color = [1 1 0];
elseif (nargin<3)
  figure (parent_fig);
  line_color = [1 1 0];
end

eval (['global ROIs',int2str(parent_fig)]);
eval (['ROIs = ROIs',int2str(parent_fig),';']);
index = find(ROIs==-1);
numROIs = length(index)-1;

for i=1:numROIs
  Vertices = ROIs((index(i)+1):(index(i+1)-1));
  numVertices = length(Vertices)/2;
  Xi = Vertices(1:numVertices);
  Yi = Vertices((numVertices+1):(length(Vertices)));
  drawroi (Xi,Yi,line_color,child_fig);
end
