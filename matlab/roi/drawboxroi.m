function [roiHandle,Xi,Yi,lineHandle] = box (line_color, fig)

%
%
%     [roiHandle,Xi,Yi,lineHandle] = box (line_color, fig)
%
%


if (nargin<1)
  line_color = [1 1 0];
  fig = gcf;
elseif (nargin<2)
  fig = gcf;
else
  figure(fig);
end

eval (['global roiNumber',int2str(fig)]);
eval (['roiNumber = roiNumber',int2str(fig),';']);

if (length(roiNumber) == 0)
  roiNumber = 0;
end

roiNumber = roiNumber+1;

disp ('Click on two opposing corners of the box...');

[x,y] = getpixel(2);

lx = [x(1) x(2) x(2) x(1) x(1)];
ly = [y(1) y(1) y(2) y(2) y(1)];
lz = [1 1 1 1 1];                 % Put the line on top of the fig

lineHandle = line (lx,ly,lz,'EraseMode','none', ...
    'Color',line_color);

text (min(x(2),x(1))+abs(x(2)-x(1))/2, ...
      min(y(1),y(2))+abs(y(1)-y(2))/2, 1, ...
      num2str(roiNumber), ...
      'EraseMode','none', ...
      'Color',line_color);

roiHandle = roiNumber+(100*fig);

% Output the vertices of the ROI in normalized
% coordinates.

Xlimits = get (gca,'XLim');
Ylimits = get (gca,'YLim');

Xrange = max(Xlimits) - min(Xlimits);
Yrange = max(Ylimits) - min(Ylimits);

Xi = lx ./ Xrange;
Yi = ly ./ Yrange;

eval (['global ROIs',int2str(fig)]);
eval (['ROIs = ROIs',int2str(fig),';']);

if (length(ROIs)==0)
  ROIs = -1;
end

ROIs = [ROIs Xi Yi -1];

eval (['ROIs',int2str(fig),' = ROIs;']);

eval (['roiNumber',int2str(fig),' = roiNumber;']);
