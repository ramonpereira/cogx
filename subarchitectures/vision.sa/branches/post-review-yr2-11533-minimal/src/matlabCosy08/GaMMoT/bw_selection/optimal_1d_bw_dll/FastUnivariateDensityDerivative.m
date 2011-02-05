function [D]=FastUnivariateDensityDerivative(N,M,X,Y,h,r,epsilon)

% FastUnivariateDensityDerivative.
% Fast Evaluation.
% C++ Implementation.
% Loads FastUnivariateDensityDerivative.dll
%--------------------------------------------------------------------------
% Author     :     Vikas.C.Raykar 
% Date        :     17 September 2005
% Contact    :     vikas@cs.umd.edu
%--------------------------------------------------------------------------
% INPUTS
%--------------------------------------------------------------------------
% N                 --> number of source points.
% M                 --> number of target points.
% X                 --> 1 x N matrix of N source points.
% Y                 --> 1 x M matrix of M target points.
% h                 --> source bandwidth or scale.
% r                 --> derivative order.
% epsilon      --> error.
%--------------------------------------------------------------------------
% OUTPUTS
%--------------------------------------------------------------------------
% D                --> 1 x M vector of the density derivative
%                          evaluated at  each target point.
%--------------------------------------------------------------------------
% Direct implementation of the r^{th} kernel density derivative
% estimate based on the Gaussian kernel.
% [HANDLES ONLY UNIVARIATE CASE]
%--------------------------------------------------------------------------

