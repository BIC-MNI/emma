function new_data = smooth (old_data)

% SMOOTH  do a simple spatial smoothing on a vector image
%
%        new_data = smooth (old_data)
%
%


if (nargin ~= 1)
   help smooth
   error ('Incorrect number of input arguments.');
end


mask = [0 0 1 0 0; ...
        0 1 1 1 0; ...
	1 1 1 1 1; ...
	0 1 1 1 0; ...
	0 0 1 0 0  ];

[x,y] = size (old_data);

if (x==y)
    xsize = x;
else
    xsize= x^.5;
    if (xsize ~= floor (xsize))
        error('Image must be square.');
    end
    if (y ~= 1)
        error('Image must be a vector if not square.');
    end
    old_data = reshape (old_data, xsize, xsize);
end

%
%  Now old_data is a square image matrix
%

new_data = zeros(xsize, xsize);
norm = length (find (mask));          % Number of 1's in the mask

for i=3:xsize-2
    for j=3:xsize-2
        new_data(i,j) = sum (sum (old_data(i-2:i+2, j-2:j+2) .* mask)) / norm;
    end
end

new_data = reshape (new_data, xsize^2, 1);
