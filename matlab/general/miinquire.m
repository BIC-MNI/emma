% MIINQUIRE   find out various things about a MINC file from MATLAB
%
%   info = miinquire ('minc_file' [, 'option' [, 'item']], ...)
%
% miinquire has a rather involved syntax, so pay attention.  The first
% argument is always the name of a MINC file.  Following the filename
% can come any number of "option sequences", which consist of the option
% (a string) followed by zero or more items (more strings).
%
% Any number of option sequences can be included in a single call to 
% miinquire, as long as enough output arguments are provided (this is
% checked mainly as a debugging aid to the user).  Generally, each option
% results in a single output argument.
%
% The currently available options are:
%
%     dimlength
%     imagesize
%     vartype
% 
% Options that will most likely be added in the near future are:
%
%     dimnames
%     varnames
%     vardims
%     varatts
%     atttype
%     attvalue
%
% One inconsistence with the standalone utility mincinfo (after which 
% miinquire is modelled) is the absence of the option "varvalues".  
% The functionality of this available in a superior way via the CMEX
% mireadvar.
%
% EXAMPLES
%
%  [ImSize, NumFrames, ImType] = ...
%    miinquire ('foobar.mnc', 'imagesize', ...
%               'dimlength', 'time', ...
%               'vartype', 'image');
%
% puts the four-element vector of image dimension sizes into ImSize; 
% the length of the time dimension into the scalar NumFrames; and the
% type of the image variable into the string ImType.
