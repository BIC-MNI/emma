function [activity, start_times, stop_times] = getblooddata (study)
%  GETBLOODDATA  - retrieve blood activity and sample times from a study
%  [activity, start_times, stop_times] = getblooddata (study)
%
%  The study variable can be a handle to an open image, or the name of 
%  a NetCDF (MNC or BNC) file containing the blood activity data for
%  the study.  If it is a handle, getblooddata will search look for the
%  associated BNC file and retrieve the data from that.  (Eventually, 
%  it will also look in the associated MNC file, but we need to write
%  a better way to inquire NetCDF files from MATLAB first.)  If it 
%  is a filename (either MNC or BNC), getblooddata will directly retrieve
%  the data from that filename only.  Eventually, we should add enough
%  smarts to make it look in both the BNC and MNC files, and NOT barf
%  if the variables are not found!  I don't feel up to today though.

% If study is a string, just use it as the filename.

if isstr (study)
	filename = study;
else

	% study is a number, so use it as a handle to access Filename#

	eval(['global Filename' int2str(study)]);
	if exist (['Filename' int2str(study)]) ~= 1
		error ('study is not the handle for an open image')
	end
	
	% copy Filename# to local variable, check if it's empty

	filename = eval(['Filename' int2str(study)]);
	if isempty (filename)
		error ('Image does not have an associated filename');
	end

	% Now we wish to strip off the extension (presumable .mnc) and tack on .bnc
	
	dot = find(filename=='.');			% location of . in filename
	if isempty(dot)						% no extension found (not too likely!)
		filename = [filename '.bnc'];
	else
		filename = [filename(1:(dot(length(dot))-1)) '.bnc'];
	end
end

% Now we have a filename to use - check that it exists, and if so read it
if exist (filename) ~= 2
	error (['File ' filename ' not found']);
end

activity = mireadvar (filename, 'corrected_activity');
start_times = mireadvar (filename, 'sample_start');
stop_times = mireadvar (filename, 'sample_stop');
