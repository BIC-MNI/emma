function handle = newimage (NewFile, DimSizes, ParentFile, ...
                            ImageType, ValidRange, DimOrder)
% NEWIMAGE  create a new MINC file, possibly descended from an old one
%
%  handle = newimage (NewFile, DimSizes, ParentFile, ...
%                     ImageType, ValidRange, DimOrder)
%
% creates a new MINC file.  NewFile and DimSizes must always be given,
% although the number of elements required in DimSizes varies
% depending on whether ParentFile is given (see below).  All other
% parameter are optional, and, if they are not included or are
% empty, default to values sensible for PET studies at the MNI.
%
% The optional arguments are:
%
%   ParentFile - the name of an already existing MINC file.  If this
%                is given, then a number of items are inherited from
%                the parent file and included in the new file; note
%                that this can possibly change the defaults of all
%                following optional arguments.
%
%   DimSizes   - a vector containing the lengths of the image
%                dimensions.  If ParentFile is not given, then all
%                four image dimensions (in the order frames, slices,
%                height, and width) must be specified.  Either or both
%                of frames and slices may be zero, in which case the
%                corresponding MINC dimension (MItime for frames, and
%                one of MIzspace, MIyspace, or MIxspace for slices)
%                will not be created.  If ParentFile is given, then
%                only the number of frames and slices are required; if
%                the height and width are not given, they will default
%                to the height/width of the parent MINC file.  In no
%                case can the height or width be zero -- these two
%                dimensions must always exist in a MINC file.  See
%                below, under "DimOrder", for details on how slices,
%                width, and height are mapped to MIzspace, MIyspace,
%                and MIxspace for the various conventional image
%                viewpoints.
%
%   ImageType  - a string, containing a C-like type dictating how the
%                image is to be stored.  Currently, this may be one of
%                'byte', 'short', 'long', 'float', or 'double'; plans
%                are afoot to add 'signed' and 'unsigned' options for
%                the three integer types.  Currently, 'byte' images will
%                be unsigned and 'short' and 'long' images will be
%                signed.  If this option is empty or not supplied, it
%                will default to 'byte'.  NOTE: this parameter is currently
%                ignored.
%                
%   ValidRange - a two-element vector describing the range of possible 
%                values (which of course depends on ImageType).  If
%                not provided, ValidRange defaults to the maximum
%                range of ImageType, eg. [0 255] for byte, [-32768
%                32767] for short, etc.  NOTE: this parameter is currently
%                ignored.
%
%   DimOrder   - a string describing the orientation of the images,
%                one of 'transverse' (the default), 'sagittal', or
%                'coronal'.  Transverse images are the default if
%                DimOrder is not supplied.  Recall that in the MINC
%                standard, zspace, yspace, and xspace all have
%                definite meanings with respect to the patient: z
%                increases from inferior to superior, x from left to
%                right, and y from posterior to anterior.  However,
%                the concepts of slices, width, and height are
%                relative to a set of images, and the three possible 
%                image orientations each define a mapping from
%                slices/width/height to zspace/yspace/xspace as
%                follows:
%
%                    Orientation  Slice dim    Height dim   Width dim
%                     transverse   MIzspace     MIyspace     MIxspace
%                     sagittal     MIxspace     MIzspace     MIyspace
%                     coronal      MIyspace     MIzspace     MIxspace


% ------------------------------ MNI Header ----------------------------------
%@NAME       : newimage
%@INPUT      : 
%@OUTPUT     : 
%@RETURNS    : handle - a handle to the new image created in MATLAB
%@DESCRIPTION: Creates the appropriate variables for accessing an image
%              data set from within MATLAB, and creates an 
%              associated MINC file.
%@METHOD     : 
%@GLOBALS    : reads/increments: ImageCount
%              creates: Filename#, DimSizes#, FrameTimes#, FrameLengths#
%              (with the latter two empty)
%@CALLS      : (if a MINC filename is supplied) micreate, micreateimage
%@CREATED    : June 1993, Greg Ward & Mark Wolforth
%@MODIFIED   : 6-17 Aug 1993 - totally overhauled (GPW).
%              18 Aug 1993   - fixed up argument parsing code
%-----------------------------------------------------------------------------


% Check validity of input arguments

if (nargin < 2)
   error ('You must supply at least a new filename and dimension sizes');
end

if (nargin > 6)
   error ('Too many input arguments');
end

% If at least the parent file was given, let's open it so we can override
% the defaults on the other arguments with values from the parent file.

if (nargin >= 3) 
   if (~isempty (ParentFile))
      Parent = openimage (ParentFile);
   else
      ParentFile = '-';           % indicates that no parent file opened
      Parent = -1;                % so does this 
   end
else
   ParentFile = '-';           % indicates that no parent file opened
   Parent = -1;                % so does this 
end


% Now check all the other arguments, in descending order.  If any are
% not supplied, use the defaults.  NOTE!!!  This code should check if
% ParentFile was opened, and if so use the orientation (could be tricky),
% valid range, and image type from it!!

if (nargin <= 6)
   % Do nothing -- all arguments are supplied
end

if (nargin <= 5)                  % DimOrder not supplied, set to default
   DimOrder = 'transverse';
end

if (nargin <= 4)                  % ValidRange not supplied either
   ValidRange = [];                   % will be set after we know the type
end

if (nargin <= 3)                  % ImageType not supplied
   ImageType = 'byte';
end

% Now go through all the optional arguments (except ParentFile) and if 
% they were empty, give them defaults.  Again, this should let ParentFile
% override the defaults.

if (isempty (ImageType))
   ImageType = 'byte';
end

% The default valid ranges here are taken from /usr/include/limits.h

if (isempty (ValidRange))
   if (strcmp (ImageType, 'byte'))
      ValidRange = [0 255];
   elseif (strcmp (ImageType, 'short'))
      ValidRange = [-32768 32767];
   elseif (strcmp (ImageType, 'long'))
      ValidRange = [-2147483648 2147483647];
   elseif (strcmp (ImageType, 'float'))
      ValidRange = [1.17549435e-38 3.40282347e+38];
   elseif (strcmp (ImageType, 'double'))
      ValidRange = [2.2250738585072014e-308 1.7976931348623157e+308];
   else
      error (['Invalid image type: ' ImageType]);
   end
end

if (isempty (DimOrder))
   DimOrder = 'transverse';
end

s = size(DimSizes);
if (min(s) ~= 1) | ( (max(s) ~= 2) & (max(s) ~= 4) )
   error ('DimSizes must be a vector with either 2 or 4 elements');
end

if (max (s) == 2)
   if (isempty (ParentFile)) | (Parent == -1)
      error ('Must supply all 4 dimension sizes if parent file is not given');
   else
      DimSizes (3) = getimageinfo(Parent,'ImageHeight');
      DimSizes (4) = getimageinfo(Parent,'ImageWidth');
   end
end

% At this point, we have a four-element DimSizes, and if ParentFile was
% given then Parent contains the handle to the open MINC file.  Also, if
% ParentFile was not given, ImageType, ValidRange, and DimOrder are
% set.  So let's create the new MINC file, copying the patient, study
% and acquisition variables if possible.

%disp (['New file: ' NewFile]);
%disp (['Old file: ' ParentFile]);    % will be set to '' if none given by caller
%disp ('DimSizes: ');
%disp (DimSizes);
%disp (['Image type: ' ImageType]);
%disp ('Valid range:');
%disp (ValidRange);
%disp (['Orientation: ', DimOrder]);

execstr = sprintf ('micreate %s %s patient study acquisition', ...
                   ParentFile, NewFile);
[result,output] = unix (execstr);
if (result ~= 0)
   error (['Error running ' execstr ' to create file ' NewFile]);
end

% Now create the image variable in the new MINC file, using the current
% values of DimSizes and DimOrder.  Note that the sprintf implicitly
% assumes that DimSizes has exactly four elements!

execstr = sprintf ('micreateimage %s %d %d %d %d %s', ...
                   NewFile, DimSizes, DimOrder);
[result,output] = unix (execstr);
if (result ~= 0)
   error (['Error running ' execstr ' to create image in file ' NewFile]);
end

closeimage (Parent);


% Figure out what the handle to return should be

global ImageCount

if exist ('ImageCount') == 1
   ImageCount = ImageCount + 1;
else
   ImageCount = 1;
end

% MINC file is now created (if applicable), so we must create the
% MATLAB variables that will be used by putimages

eval(['global Filename'     int2str(ImageCount)]);
eval(['global DimSizes'     int2str(ImageCount)]);
eval(['global FrameTimes'   int2str(ImageCount)]);
eval(['global FrameLengths',int2str(ImageCount)]);

eval(['Filename'     int2str(ImageCount) ' = NewFile;']);
eval(['DimSizes'     int2str(ImageCount) ' = DimSizes;']);
eval(['FrameTimes'   int2str(ImageCount) ' = [];']);
eval(['FrameLengths' int2str(ImageCount) ' = [];']);

handle = ImageCount;
