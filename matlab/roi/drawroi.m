function lineHandle = drawroi (Xi,Yi,line_color,fig)

%
%
%      lineHandle = drawroi (Xi,Yi[,line_color[,fig]])
%
%

if (nargin<2)
  help drawroi
  error('Too few arguments.');
elseif (nargin<3)
  line_color = [1 1 0];
  fig = gcf;
elseif (nargin<4)
  fig = gcf;
else
  figure(fig);
end


Xlimits = get (gca,'XLim');
Ylimits = get (gca,'YLim');

Xrange = max(Xlimits) - min(Xlimits);
Yrange = max(Ylimits) - min(Ylimits);

lx = Xi .* Xrange;
ly = Yi .* Yrange;
lz = ones(1,length(ly));

lineHandle = line (lx,ly,lz,'EraseMode','none', ...
    'Color',line_color);

