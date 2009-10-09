function pdf = getOptimalBatchKDE( X, varargin )

scaleSilver = 1 ;0.75 ;
typeBandwidth = 'plugin' ; % 'silverman'
args = varargin;
nargs = length(args);
for i=1:2:nargs
    switch args{i}
        case 'typeBandwidth', typeBandwidth = args{i+1} ;
    end
end

if ( isequal(typeBandwidth,'plugin') )
    h = slow_univariate_bandwidth_estimate_STEPI(length(X),X) ;
elseif ( isequal(typeBandwidth,'silverman') )     
    h = getSilver( X, scaleSilver ) ;
end
pdf.mu = X ;
pdf.covariances = ones(length(X),1)*h^2 ;
pdf.weights = ones(1,length(X)) ;
pdf.weights = pdf.weights / sum(pdf.weights) ;


% --------------------------------------------------------------- %
function h = getSilver( X, scaleSilver )

sg = cov(X) ;
n_sg = length(X) ;

C0 = scaleSilver^2*(sqrt(sg)*(4/(3*n_sg))^(1/5))^2 ; % silverman rule-of-thumb 
h = sqrt(C0) ;