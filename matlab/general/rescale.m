%RESCALE - Multiply a matrix by a scalar
%
%    rescale(MATRIX,SCALAR)
%
%  This function was designed to reduce memory use problems that occur when
%  multiplying a large matrix by a scalar.  The passed matrix is multiplied
%  by the scalar without memory being re-allocated.
