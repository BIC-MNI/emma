% PET image analysis
%
% MINC I/O functions
%   openimage     - Open an image stored in a MINC file.
%   newimage      - Create a new data set for image storage.
%   getimageinfo  - Get information about a data set.
%   getimages     - Get images from an open data set.
%   putimages     - Put an image into a data set.
%   getimglines   - Get a contiguous range of horizontal lines from an image.
%   getblooddata  - Get blood data from a data set.
%   resampleblood - Get resampled blood data from a data set.
%   closeimage    - Close an image data set.
%
% General utility functions
%   calpix        - Generate a vector index for a point in an image vector.
%   viewimage     - View an image.
%   spectral      - Generate the RGB numbers for a spectral colourmap.
%   C_trapz       - Fast CMEX function for trapezoidal integration.
%   lookup        - Fast CMEX function for linear interpolation.
%   nframeint     - Fast CMEX integration across frames.
%   deriv         - Calculate the derivative of a function
%   smooth        - Perform a simple spatial smoothing on an image.
%
% rCBF analysis functions
%   rcbf1         - Performs a single compartment rCBF analysis.
%   rcbf2         - Performs a full two compartment rCBF analysis, with
%                   blood dispersion and delay correction.

