function lineHandle = drawroi (Xi,Yi,line_color,fig)

% DRAWROI - Draws a given ROI on the given figure
%
%
%      lineHandle = drawroi (Xi,Yi[,line_color[,fig]])
%
%
%  This function draws the given ROI on the current figure (or on the
%  specified figure).  It takes at least two arguments (the normalized X and
%  Y coordinates of the ROI), and can also take a line colour to use, and a
%  figure number to draw the ROI on.  The function returns the MATLAB handle
%  of the created line.
%

% @COPYRIGHT  :
%             Copyright 1993,1994 Mark Wolforth and Greg Ward, McConnell
%             Brain Imaging Centre, Montreal Neurological Institute, McGill
%             University.
%             Permission to use, copy, modify, and distribute this software
%             and its documentation for any purpose and without fee is
%             hereby granted, provided that the above copyright notice
%             appear in all copies.  The authors and McGill University make
%             no representations about the suitability of this software for
%             any purpose.  It is provided "as is" without express or
%             implied warranty.


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

