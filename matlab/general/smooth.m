function new_data = smooth (old_data)

% SMOOTH  do a simple spatial smoothing on a vector image
%
%        new_data = smooth (old_data)
%
%

mask = [0 0 1 0 0 ...
        0 1 1 1 0 ...
	1 1 1 1 1 ...
	0 1 1 1 0 ...
	0 0 1 0 0 ];

new_data = zeros(length(old_data));
    
for pos = 1:length(old_data-26)
    new_data (pos+13) = mean(mask .* old_data (pos:pos+25));
end
