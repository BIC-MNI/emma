% PET image analysis
%
% High-level MINC I/O functions
%   openimage     - Open an image stored in a MINC file.
%   newimage      - Create a new MINC file.
%   getimageinfo  - Get information about an opened MINC file.
%   getimages     - Get images from an open MINC file.
%   putimages     - Put an image into a MINC file created with newimage.
%   getimglines   - Get a contiguous range of horizontal lines from an image.
%   getblooddata  - Get blood data from a data set.
%   getmask       - Interactively calculate a threshold mask.
%   resampleblood - Get resampled blood data from a data set.
%   closeimage    - Close an image data set.
%
% Low-level (CMEX or standalone executables) MINC I/O functions
%
%   mireadimages  - Read images from a MINC file (used by getimages).
%   mireadvar     - Read a hyperslab from any NetCDF variable.
%   micreate      - Create a new MINC file from scratch.
%   micreateimage - Create the MIimage variable in a new MINC file.
%   miwriteimages - Write images to a MINC file (used by putimages).
%   miwritevar    - Write a hyperslab to any NetCDF variable.
%   miinquire     - Get NetCDF variable, dimension, or attribute information.
%
%     Note: these seven functions should not generally be called by 
%     general purpose image analysis applications.  Use the high-
%     level functions instead.
%
% General utility functions
%   calpix        - Generate a vector index for a point in an image vector.
%   getpixel      - Use this instead of ginput.
%   viewimage     - View an image.
%   spectral      - Generate the RGB numbers for a spectral colourmap.
%   hotmetal      - Generate the RGB numbers for a hotmetal colourmap.
%   ntrapz        - Fast CMEX function for trapezoidal integration.
%   lookup        - Fast CMEX function for linear interpolation.
%   nframeint     - Fast CMEX integration across frames.
%   deriv         - Calculate the derivative of a numerical function.
%   smooth        - Perform a simple spatial smoothing on an image.
%
% rCBF analysis functions
%   rcbf1         - Performs a single compartment rCBF analysis.
%   rcbf2         - Performs a full two compartment rCBF analysis, with
%                   blood dispersion and delay correction.

