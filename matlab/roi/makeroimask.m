function mask = makeroimask (wantedROIs, fig, dim)

%
%
%        mask = makeroimask (ROIs [,fig[,dim]])
%
%


if (nargin<1)
  help makeroimask
  error('Too few input arguments.');
elseif (nargin<2)
  fig = gcf;
  
  Xlimits = get (gca,'XLim');
  Ylimits = get (gca,'YLim');
  
  Xrange = max(Xlimits) - min(Xlimits);
  Yrange = max(Ylimits) - min(Ylimits);
  xmin = min(Xlimits);
  xmax = max(Xlimits);
  ymin = min(Ylimits);
  ymax = max(Ylimits);
elseif (nargin<3)
  figure(fig);
  
  Xlimits = get (gca,'XLim');
  Ylimits = get (gca,'YLim');
  
  Xrange = max(Xlimits) - min(Xlimits);
  Yrange = max(Ylimits) - min(Ylimits);
  xmin = min(Xlimits);
  xmax = max(Xlimits);
  ymin = min(Ylimits);
  ymax = max(Ylimits);
else
  Xrange = dim(1);
  Yrange = dim(2);
  xmin = 1;
  xmax = Xrange+1;
  ymin = 1;
  ymax = Yrange+1;
end

mask = zeros(Xrange,Yrange);

eval (['global ROIs',int2str(fig)]);
eval (['ROIs = ROIs',int2str(fig),';']);
index = find(ROIs==-1);
numROIs = length(index)-1;

if (length(wantedROIs) == 0)
  wantedROIs = 1:numROIs;
end

for i=wantedROIs

  Vertices = ROIs((index(i)+1):(index(i+1)-1));
  numVertices = length(Vertices)/2;
  xi = ((Vertices(1:numVertices)).*Xrange)';
  yi = ((Vertices((numVertices+1):(length(Vertices)))).*Yrange)';

  %
  % Make sure xi and yi don't form a closed polygon
  % (closedness is implied).
  %

  n = length(xi); 
  if xi(n)==xi(1) & yi(n)==yi(1)
    xi = xi(1:n-1);
    yi = yi(1:n-1);
  end

  %
  % Transform xi,yi into pixel coordinates.  Fix-up coordinates
  % to deal with coordinates extending from pixel boundaries rather
  % than pixel centers.
  % 
  
  dx = max( (xmax-xmin)/Xrange, eps );
  dy = max( (ymax-ymin)/Yrange, eps );
  kx = (Xrange-1+dx);
  ky = (Yrange-1+dy);
  xx = max(min((xi-xmin)/(xmax-xmin)*kx+(1-dx/2),Xrange),1);
  yy = max(min((yi-ymin)/(ymax-ymin)*ky+(1-dy/2),Yrange),1);

  %
  % Coordinates of pixels
  %
  
  [u,v] = meshgrid(1:Xrange,1:Yrange);

  m = length(xx);

  %
  % Make sure polygon is traversed counter clockwise
  %
  
  [dum,i] = min(xx);
  h = rem(i+m-2,m)+1;
  j = rem(i,m)+1;
  if det([xx([h i j]) yy([h i j]) ones(3,1)]) > eps
    xx = flipud(xx(:)); 
    yy = flipud(yy(:)); 
  end

  %
  % For each triangular piece of the general polygon, find the interior
  %
  
  while m>=3,
    imin = 1; jmin = 2; hmin = 3;       % Defaults
  
    %
    % Find triangle with minimum diagonal
    %
    
    mindiag = inf;
    for i=1:m,
      h = rem(i+m-2,m)+1;
      j = rem(i,m)+1;
      if det([xx([h i j]) yy([h i j]) ones(3,1)])<eps,
        thisdiag = norm([xx(h)-xx(j) yy(h)-yy(j)]);
        if thisdiag<mindiag
          mindiag = thisdiag;
          imin = i;
          hmin = h;
          jmin = j;
        end
      end
    end
    m = m-1;
    dd = ones(Xrange,Yrange);

    for k=1:3,
      dx = xx(imin)-xx(jmin);
      dy = yy(imin)-yy(jmin);
      dd = dd & (((u-xx(imin))*dy - (v-yy(imin))*dx) <= 1);
      sav = imin;
      imin = jmin;
      jmin = hmin;
      hmin = sav;
    end
    mask = dd | mask;

    %
    % Remove vertex at imin
    %
    
    xx(imin) = []; yy(imin) = [];

  end
end
