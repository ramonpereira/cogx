%%
% Originally a part of: Maggot (developed within EU project CogX)
% Author: Matej Kristan, 2009 (matej.kristan@fri.uni-lj.si; http://vicos.fri.uni-lj.si/matejk/)
% Last revised: 2009
%%
function [out_kde, subindicator] = regularizeKDEInBandwidth( input_kde, varargin )
% takes an input_kde , analyzes its bandwidth and if the bandwidth is below
% the value "practicallyZero" in certain directions of the subspace, those
% directions are regularized to "practicallyZero".

minVal = 1e-10 ;
practicallyZero = [] ;
% process arguments
args = varargin;
nargs = length(args);
for i = 1:2:nargs
    switch args{i}        
        case 'practicallyZero', practicallyZero = args{i+1} ; 
    end
end

if isempty(practicallyZero)
    practicallyZero = 1e-10 ;
end

out_kde = input_kde ;
subindicator = 0 ;
% get bandwidht
H = readOutTheKernel( input_kde.pdf ) ;

% regularize bandwidth
[U,S,V] = svd(H) ;
s = diag(S) ;
% e = s / sum(s) ;
% if min(e) < practicallyZero   
%    defaultval = mean(s( e > practicallyZero ))*1e-3 ;
%    s( e <= practicallyZero ) = defaultval ;
   
if min(s) < minVal  
   id_ok = find(s > minVal) ;
   id_notok = find(s <= minVal) ;
   if sum(id_ok) > 0
       defaultval = mean(s( id_ok ))*1e-3 ;
   else
       defaultval = minVal ;
   end
  
   s( id_notok ) = defaultval ;  
   
   H = U*diag(s)*U' ;
   % reset the bandwidth     
   out_kde.pdf = readjustKernels( input_kde.pdf, H );
   subindicator = 1 ;
else
    out_kde = input_kde ;
end

if nargout < 2
    subindicator = [] ;
end



