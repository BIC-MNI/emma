function info = getimageinfo (handle, whatinfo)
% GETIMAGEINFO   retrieve helpful trivia about an open image
%
%     info = getimageinfo (handle, whatinfo)
% 
% Get some information about an open image.  handle refers to a MINC
% file previously opened with openimage or created with newimage.
% whatinfo is a string that describes what you want to know about.
% The possible values of this string are numerous and ever-expanding.
% 
% The first possibility is the name of one of the standard MINC image
% dimensions: "time", "zspace", "yspace", or "xspace".  If these are
% supplied, getimageinfo will return the length of that dimension from
% the MINC file, or 0 if the dimension does not exist.  Note that
% requesting "time" is equivalent to requesting "NumFrames"; also,
% the three spatial dimensions also have equivalences that are
% somewhat more complicated.  For the case of transverse images,
% zspace is equivalent to NumSlices, yspace to ImageHeight, and xspace
% to ImageWidth.  See the help for newimage (or the MINC standard
% documentation) for details on the relationship between image
% orientation (transverse, sagittal, or coronal) and the MINC spatial
% image dimensions.
% 
% The other possibilities for whatinfo, and what they cause
% getimageinfo to return, are as follows:
%
%     Filename     - the name of the MINC file (if applicable)
%                    as supplied to openimage or newimage; will be
%                    empty if data set has no associated MINC file.
%
%     NumFrames    - number of frames in the study, 0 if non-dynamic
%                    study (equivalent to "time")
%
%     NumSlices    - number of slices in the study (0 if no slice
%                    dimension)
%
%     ImageHeight  - the size of the second-fastest varying spatial 
%                    dimension in the MINC file.  For transverse
%                    images, this is just the length of MIyspace.
%                    Also, when an image is displayed with viewimage,
%                    the dimension that is "vertical" on your display
%                    is the image height dimension.  (Assuming
%                    viewimage is working correctly.)
%
%     ImageWidth   - the size of the fastest varying spatial
%                    dimension, which is MIxspace for transverse
%                    images.  When an image is displayed with
%                    viewimage, the image width is the horizontal
%                    dimension on your display.
%
%     ImageSize    - a two-element vector containing ImageHeight and
%                    ImageWidth (in that order).  Useful for viewing 
%                    non-square images, because viewimage needs to know
%                    the image size in that case.
%
%     DimSizes     - a four-element vector containing NumFrames, NumSlices,
%                    ImageHeight, and ImageWidth (in that order)
%
%     FrameLengths - vector with NumFrames elements - duration of
%                    each frame in the study, in seconds.  This is
%                    simply the contents of the MINC variable
%                    "time-width"; if this variable does not exist in
%                    the MINC file, then getimageinfo will return an
%                    empty matrix.
%
%     FrameTimes   - vector with NumFrames elements - start time of
%                    each frame, relative to start of study, in
%                    seconds.  This comes from the MINC variable
%                    "time"; again, if this variable is not found,
%                    then getimageinfo will return an empty matrix.
%
%     MidFrameTimes - time at the middle of each frame (calculated by
%                     FrameTimes + FrameLengths/2) in seconds
%      
% If the requested data item is invalid or the image specified by handle
% is not found (ie. has not been opened), then the returned data will
% be an empty matrix.  (You can test whether this is the case with
% the isempty() function.)
%
% SEE ALSO  openimage, newimage, getimages

% ------------------------------ MNI Header ----------------------------------
%@NAME       : getimageinfo
%@INPUT      : handle - handle to an opened MATLAB image set
%              whatinfo - character string describing what is to be returned
%                 for currently supported values, type "help getimageinfo"
%                 in MATLAB
%@OUTPUT     : 
%@RETURNS    : info - the appropriate image data, either from within
%              MATLAB or read from the associated MINC file
%@DESCRIPTION: Read and return various data about an image set.
%@METHOD     : 
%@GLOBALS    : Filename#, DimSizes#, FrameLengths#, FrameTimes#
%@CALLS      : mireadvar (CMEX), miinquire (CMEX)
%@CREATED    : 93-6-17, Greg Ward
%@MODIFIED   : 93-6-17, Greg Ward: added standard MINC dimension names,
%              spruced up help
%              93-7-6, Greg Ward: added this header
%              93-8-18, Greg Ward: massive overhaul (see RCS log for details)
%-----------------------------------------------------------------------------

if nargin ~= 2
   error ('Incorrect number of arguments');
end

if length(handle) ~= 1
   error ('handle must be a scalar');
end

if ~isstr(whatinfo)
   error ('whatinfo must be a string');
end

lwhatinfo = lower (whatinfo);

% Make global the three image-size variables, and also the (possibly)
% named value.  This may be redundant, eg. if whatinfo=='FrameTimes'.  But
% it assures that everything we could possibly need global is global
% before calling exist or eval with it.

eval(['global Filename' int2str(handle)]);
eval(['global DimSizes' int2str(handle)]);
eval(['global FrameTimes' int2str(handle)]);
eval(['global FrameLengths' int2str(handle)]);
eval(['global ' whatinfo int2str(handle)])

eval(['filename = Filename' int2str(handle) ';']);
eval(['dimsizes = DimSizes' int2str(handle) ';']);

% If "whatinfo" is one of the MINC image dimension names, just do 
% an miinquire on the MINC file for the length of that dimension.
% If miinquire returns an empty matrix, that means the dimension 
% doesn't exist, so getimageinfo will return 0.

if (strcmp (lwhatinfo, 'time') | ...
    strcmp (lwhatinfo, 'zspace') | ...
    strcmp (lwhatinfo, 'yspace') | ...
    strcmp (lwhatinfo, 'xspace'))

   info = miinquire (filename, 'dimlength', lwhatinfo);
   if (isempty (info))
      info = 0;
   end


% Now check if it's one of NumSlices, NumFrames, ImageHeight, or 
% ImageWidth -- ie. an element of DimSizes.

elseif (strcmp (lwhatinfo, 'numframes'))
   info = dimsizes (1);

elseif (strcmp (lwhatinfo, 'numslices'))
   info = dimsizes (2);

elseif (strcmp (lwhatinfo, 'imageheight'))
   info = dimsizes (3);

elseif (strcmp (lwhatinfo, 'imagewidth'))
   info = dimsizes (4);

elseif (strcmp (lwhatinfo, 'imagesize'))
   info = dimsizes (3:4);

elseif (strcmp (lwhatinfo, 'dimsizes'))
   info = dimsizes;

% Now check if it's an option calculated from other options (currently
% this is only MidFrameTimes).

elseif (strcmp (lwhatinfo, 'midframetimes'))
   info = eval(['FrameTimes' int2str(handle) ' + FrameLengths' int2str(handle) ' / 2']);

% Finally check to see if 'whatinfo' with the handle number tacked on
% exists as a variable.  Note that this will work because of the 
% 'global whatinfo#' eval'd above; also, this bit is STILL case
% sensitive because of the mixed case of the global variables created
% by openimage or newimage.  Maybe that should be changed...?

elseif (exist ([whatinfo int2str(handle)]) == 1)
   info = eval([whatinfo int2str(handle)]);
else
   error (['Unknown option: ' whatinfo]);
end
