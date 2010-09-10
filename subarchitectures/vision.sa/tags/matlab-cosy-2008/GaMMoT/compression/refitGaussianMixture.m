function ptf1 = refitGaussianMixture( f1, varargin )
%
% Matej Kristan (2007)
%
% re-fit Gaussian distribution through function approximation
% input:
%   f1              ... reference distribution
%   f_init          ... intial rbf set for approximation. If this is [], then
%                       intial is constructed from f1.
%   maxIterations   ... maximum number of iterations. If this is [], then
%                       default value 50 is used.
%   options         ... optimization options. If [] then default is used.
%   showIntermediate... show intermediate results. 0 means "no", 1 means "yes"
%   maxIterations, options0, showIntermediate

global pdf_ref ptf1 ptX ptY showInterm opt_weights0 ref_weights k alpha fignum;
fignum = 3 ;  
maxIterationsDefault = 50 ; 
type_of_least_squares = 'nonlinear' ; % 'nonlinear', 'linear'
type_of_minimization = 'constrained' ; % 'constrained', 'unconstrained'
type_of_fineOptimization = 'leastSquares' ; % 'LevenbergMarquadt', 'leastSquares'
debugmode =  'off' ;
%  TolX = 1e-8 ; TolFun = 1e-8 ;
 %  
 TolX = 1e-8 ; TolFun = 1e-8 ; 
 
 % TolX = 1e-3 , TolFun = 1e-8  ;


  
if isequal(debugmode,'off') DerivativeCheck = 'off' ;
else DerivativeCheck = 'on' ;
end

k = 3 ;     % number of control points per half-dimension
alpha = 6 ; % scale of control points

typeOperation = 'accurate' ; % 'fast', 'accurate'
reportProgress = 1 ;
modify = 0 ;
pdf_ref = f1 ;
f_init = [] ;
options = [] ;
showInterm = 0 ;
opt_weights = 1 ;
maxIterations = maxIterationsDefault ;
gradient_opt = 0 ;
args = varargin;
nargs = length(args);
for i=1:2:nargs
    switch args{i}
        case 'maxIterations', maxIterations = args{i+1} ;
        case 'f_init', f_init = args{i+1};
        case 'options', options = args{i+1};
        case 'showIntermediate', showInterm = args{i+1};
        case 'gradient', gradient_opt = args{i+1};
        case 'modify', modify = args{i+1} ;
        case 'type_of_minimization', type_of_minimization = args{i+1};
        case 'reportProgress', reportProgress = args{i+1} ;
        case 'typeOperation', typeOperation = args{i+1} ;
        otherwise, ;
    end
end

%  if gradient_opt == 1
%     TolX = 1e-8 ; TolFun = 1e-8 ;
%  else
%      TolX = 1e-12 ; TolFun = 1e-8 ;
%  end


if gradient_opt == 1
    TolX = 1e-8;1e-3 ; TolFun = 1e-8 ;
else
    TolX = 1e-8 ; TolFun = 1e-8 ; 
end

if isequal(typeOperation,'fast')
    TolX = 1e-3 ; TolFun = 1e-8 ; 
end

if nargin < 1 
    error('At least one argument is required!') ;
end

if isempty(f_init)
   f_init = createInitialRBFset( f1, 'modify', modify ) ; 
end

% increase number of control points if required
k0 = round(k*length(f_init.weights)/length(f1.weights)+0.5) ;
if k0 > k k = k0 ; end

ptf1 = f_init ;
opt_weights0 = opt_weights ;
ref_weights = f_init.weights ;

[X, numSigPoints ] = getPointsOnDistribution( f1, k, alpha ) ;
Y = evaluateDistributionAt( f1.mu, f1.weights, f1.covariances, X ) ;
% showme(f1, X, Y) ;

ptX = X ;
ptY = Y ;
x0 = distributionToVec( ptf1 ) ;

if reportProgress == 1
   reportProgress = 'iter' ;
else
    reportProgress = 'off' ;
end

if isempty(options) 
    options = optimset( 'TolX',TolX,'TolFun',TolFun, 'LevenbergMarquardt', 'on','LargeScale', 'off',...
                        'MaxIter',maxIterations, 'GradObj','on', ...
                        'DerivativeCheck', DerivativeCheck ,'Display', reportProgress ) ;    
end

if gradient_opt == 1
    x0 = distributionToVec( ptf1 )' ;
    if isequal(type_of_minimization,'unconstrained')
        x = fminunc(@(x)evaluatepdfOptErf(x),x0,options) ;
    elseif isequal(type_of_minimization,'constrained')
        [lb, ub] = getBounds( ptf1, opt_weights0 ) ;
        c = ub ; c(find(ub~=1)) = 0 ; Aeq = [c] ;
        beq = [1] ;
        
        x = fmincon(@(x)evaluatepdfOptErf(x),x0,[],[],Aeq,beq,lb, ub, [], options) ;
%        x = fmincon(@(x)evaluatepdfOptErf(x),x0,[],[],[], [],lb, ub, @nonlinearConstraint, options) ;
    else
        error(['Unknown optimization type: ', type_of_minimization]) ;
    end
    
%     figure(2); title(sprintf('Sum: %f',sum(ptf1.weights))) ; drawnow ;
    ptf1 = vecToDistribution( x' ) ;
    ptf1.weights = abs(ptf1.weights)/sum(abs(ptf1.weights)) ;
    ptf1.covariances = abs(ptf1.covariances)  ;
else
    if isequal(type_of_fineOptimization,'LevenbergMarquadt')
        options = optimset( 'LevenbergMarquardt', 'on','LargeScale', 'off',...
                            'MaxIter',maxIterations, 'GradObj','on', 'Display', reportProgress  );
        x0 = distributionToVec( ptf1 )' ;
        [lb, ub] = getBounds( ptf1, opt_weights0 ) ;
        x = fmincon(@(x)evaluatepdfOptErf(x),x0,[],[],[],[],lb, ub, [], options) ;
        ptf1 = vecToDistribution( x' ) ;
    elseif isequal(type_of_fineOptimization,'leastSquares')
        options = optimset( 'TolX',TolX,'TolFun',TolFun, 'MaxIter',maxIterations,  'Display', reportProgress  );
        x0 = distributionToVec( ptf1 ) ;
        [lb, ub] = getBounds( ptf1, opt_weights0 ) ; warning off ;
        if isequal(type_of_least_squares, 'nonlinear')
            x = lsqnonlin(@evaluatepdfOpt_nllsq,x0,lb,ub,options) ;
        elseif isequal(type_of_least_squares, 'linear')
            x = lsqcurvefit(@evaluatepdfOpt_lsq,x0,X,Y,lb,ub,options) ;
        else
            error(['Unknown type of ls optimization: ', type_of_least_squares]) ;
        end
        warning on ;
        ptf1 = vecToDistribution( x ) ;
    end
    ptf1.weights = abs(ptf1.weights)/sum(abs(ptf1.weights)) ;
    ptf1.covariances = abs(ptf1.covariances)  ;
end
 
% ----------------------------------------------------------------------- %
function [lb, ub] = getBoundsUnconstr( ptf1, opt_weights )

l_mu_mnd = -inf*ones(1,cols(ptf1.mu)) ;
u_mu_mnd =  inf*ones(1,cols(ptf1.mu)) ;
l_c_mnd = -inf*ones(1,cols(ptf1.mu)) ;
u_c_mnd = inf*ones(1,cols(ptf1.mu)) ;
l_w_mnd = 0*ones(1,cols(ptf1.mu)) ;
u_w_mnd = 1*ones(1,cols(ptf1.mu)) ;

if opt_weights == 0
    l_w_mnd = [] ;
    u_w_mnd = [] ;
elseif opt_weights == -1
     l_mu_mnd = [] ;
     u_mu_mnd = [] ;
     l_c_mnd = [] ;
     u_c_mnd = [] ;
end

lb = [l_mu_mnd, l_c_mnd, l_w_mnd] ;
ub = [u_mu_mnd, u_c_mnd, u_w_mnd] ;

function [lb, ub] = getBounds( ptf1, opt_weights )

l_mu_mnd = -inf*ones(1,cols(ptf1.mu)) ;
u_mu_mnd =  inf*ones(1,cols(ptf1.mu)) ;
l_c_mnd = 0*ones(1,cols(ptf1.mu))+0.000001 ;
u_c_mnd = inf*ones(1,cols(ptf1.mu)) ;
l_w_mnd = 0*ones(1,cols(ptf1.mu)) ;
u_w_mnd = ones(1,cols(ptf1.mu)) ;

if opt_weights == 0
    l_w_mnd = [] ;
    u_w_mnd = [] ;
elseif opt_weights == -1
     l_mu_mnd = [] ;
     u_mu_mnd = [] ;
     l_c_mnd = [] ;
     u_c_mnd = [] ;
end

lb = [l_mu_mnd, l_c_mnd, l_w_mnd] ;
ub = [u_mu_mnd, u_c_mnd, u_w_mnd] ;

% ----------------------------------------------------------------------- %
function [Aeq, beq, A, b, lb, ub] = getConstraints( ptf1, opt_weights ) 

%[lb, ub] = getBoundsUnconstr( ptf1, opt_weights ) ;

[lb, ub] = getBounds( ptf1, opt_weights ) ;
len = length(ptf1.weights) ;

A = [] ; b = [] ;
if opt_weights == 0
   Aeq = [] ; deq = [] ; 
else
   mu = zeros(1,len) ;
   weights = ones(1,len) ;
   covariances = zeros(1,len)  ;

   Aeq = [];[ mu, covariances, weights ] ;
   beq = []; 1 ;
end


% ----------------------------------------------------------------------- %
function [Aeq,beq] = getEqualities( ptf1, opt_weights ) 

if opt_weights == 0
   Aeq = [] ; deq = [] ; return ;
end

len = length(ptf1.weights) ;
mu = zeros(1,len) ;
weights = ones(1,len) ;
covariances = zeros(1,len)  ;

Aeq = [ mu, covariances, weights ] ;
beq = 1 ;


% ----------------------------------------------------------------------- %
function f1 = vecToDistribution( x )
global opt_weights0 ptf1 ;
global ref_weights ;

len = length(x) ;
if opt_weights0 == 0
    t1 = 1 ; t2 = len/2 ;
    f1.mu = x(:,t1:t2) ;
    t1 = t2+1 ; t2 = len;
    f1.covariances = abs(x(:,t1:t2)') ;
    f1.weights = ref_weights ;
elseif opt_weights0 == -1
    f1 = ptf1 ;
    f1.weights = x ;
else
    t1 = 1 ; t2 = len/3 ;
    f1.mu = x(:,t1:t2) ;
    t1 = t2+1 ; t2 = t1 + len/3 - 1;
    f1.covariances = ((x(:,t1:t2)')) ;  
    t1 = t2+1 ; t2 = t1 + len/3 -1 ;
    f1.weights = x(:,t1:t2) ;
end

% in case of unconstrained optimization
%  f1.covariances = exp(f1.covariances) ;
% f1.weights = log(f1.weights) ;

% f1.weights = abs(f1.weights) ;
% f1.weights = f1.weights / sum(f1.weights) ;

% ----------------------------------------------------------------------- %
function x = distributionToVec( f1 )
global opt_weights0 ;

mu = f1.mu ;
weights = f1.weights ;
covariances = f1.covariances' ;

if opt_weights0 == 0
   weights = [] ;
elseif opt_weights0 == -1
    covariances = [] ; mu = [] ;
end

% prepare for unconstrained optimization
%  covariances = log(covariances) ;
% weights = exp(weights) ;

x = [ mu, (covariances), weights ] ;



% ----------------------------------------------------------------------- %
function err = evaluatepdfOpt_nllsqXX( x )
% optimize mu, covariances, weights
global ptf1 ptX ptY showInterm k alpha pdf_ref;

ptf1 = vecToDistribution( x ) ;

% generate additional points
%[X_cur, numSigPoints ] = getPointsOnDistribution( ptf1, 1, 1 ) ;
%y_ref = evaluateDistributionAt( pdf_ref.mu, pdf_ref.weights, pdf_ref.covariances, X_cur) ;
Y_p1 = evaluateDistributionAt( ptf1.mu, ptf1.weights, ptf1.covariances, ptX ) ;


%y_cur0 = evaluateDistributionAt( ptf1.mu, ptf1.weights, ptf1.covariances, ptX) ;
err = Y_p1 - ptY ;
          
if showInterm == 1
    showme(ptf1, ptX, ptY) ; 
end

% ----------------------------------------------------------------------- %
function err = evaluatepdfOpt_nllsq( x )
% optimize mu, covariances, weights
global ptf1 ptX ptY showInterm k alpha pdf_ref;

ptf1 = vecToDistribution( x ) ;

% generate additional points
[X_cur, numSigPoints ] = getPointsOnDistribution( ptf1, 1, 1 ) ;
y_ref = evaluateDistributionAt( pdf_ref.mu, pdf_ref.weights, pdf_ref.covariances, X_cur) ;
Y_p1 = evaluateDistributionAt( ptf1.mu, ptf1.weights, ptf1.covariances, [ptX, X_cur]) ;

%y_cur0 = evaluateDistributionAt( ptf1.mu, ptf1.weights, ptf1.covariances, ptX) ;
y_cur0 = -11 ;

err = Y_p1 - [ptY, y_ref];
          
if showInterm == 1
    showme(ptf1, [ptX,X_cur], [ptY,y_ref]) ; 
end

function y = evaluatepdfOpt_lsq( x, xdata )
% optimize mu, covariances, weights
global ptf1 ptX ptY showInterm k alpha;

ptf1 = vecToDistribution( x ) ;
if showInterm == 1
    showme(ptf1, ptX, ptY) ; 
end

% generate additional points
% [X, numSigPoints ] = getPointsOnDistribution( ptf1, k, alpha ) ;

y = evaluateDistributionAt( ptf1.mu, ptf1.weights, ptf1.covariances, xdata ) ;

% ----------------------------------------------------------------------- %
function [c,ceq, g, geq] = nonlinearConstraint(x)
l = length(x) ;
d = l/3 ;
c = 0 ; g = 0*x ; 
ceq = 1 - sum(x(l-d+1:l)) ; 
geq = x*0 ; geq(l-d+1:l) = -1 ;

% ----------------------------------------------------------------------- %
function [er g] = evaluatepdfOptErf( x )
% optimize mu, covariances, weights
global pdf_ref ptX ptY showInterm;
global opt_weights0 ;
 
x = x' ;
pdf_current = vecToDistribution( x ) ;  
if showInterm == 1
    showme(pdf_current, ptX, ptY) ; 
end
% disp('Error eval')
% tic
% y = evaluateDistributionAt( pdf_current.mu, pdf_current.weights, pdf_current.covariances, ptX ) ;
% normalizer = 1/length(ptX) ;
% er = sqrt(sum((ptY-y).^2))*normalizer ;

er = getL2error( pdf_ref, pdf_current ) ;
% toc
% disp('Gradient eval')
% tic

g = [] ;
if nargout > 1
    g = getCostderivativeAt( pdf_ref, pdf_current ) ;
%     [g_analitic, err_analitic]= getAnalyticDerivativeAt( pdf_ref, pdf_current ) ;
%     [g_analitic, g(3), er, err_analitic]
end
% toc
 
% ----------------------------------------------------------------------- %
function er = evaluatepdfOptErfOld( x, f_ref )
% optimize mu, covariances, weights
global showInterm;

ptf1 = vecToDistribution( x' ) ;
if showInterm == 1
    showmePdfs(f_ref, ptf1) ; 
end
%k = 1 ;er = suHellinger( f_ref, ptf1, k ) ; 

er = getL2error( f_ref, ptf1 ) ;

% ----------------------------------------------------------------------- %
function showmePdfs(pdf1, pdf2)
global showInterm fignum ;

if showInterm ~= 1 return ; end
figure(fignum); clf ; hold on ; 

b1 = sqrt(max([pdf1.covariances;pdf2.covariances])) ;
bmin = min([pdf1.mu,pdf2.mu]) - b1*5 ;
bmax = max([pdf1.mu,pdf2.mu]) + b1*5 ;
bounds = [bmin,bmax] ;

showPdf( bounds, 100, pdf1.mu, pdf1.covariances, pdf1.weights, 'g' ) ;
showPdf( bounds, 100, pdf2.mu, pdf2.covariances, pdf2.weights, 'r--' ) ;
title('Current optimization result.')
drawnow ;
% ----------------------------------------------------------------------- %
function showme(f1_mix, X, Y)
global showInterm fignum  ;

if showInterm ~= 1 return ; end
figure(fignum); 
clf ; 
hold on ; 

b1 = sqrt(max([f1_mix.covariances])) ;
bmin = min([f1_mix.mu]) - b1*5 ;
bmax = max([f1_mix.mu]) + b1*5 ;
bounds = [bmin,bmax] ;
%bounds = [-4, 40] ;
showPdf( bounds, 100, f1_mix.mu, f1_mix.covariances, f1_mix.weights, 'g' ) ;
plot(X, Y, '+r') ; title('Current optimization result.')
drawnow ;

% ----------------------------------------------------------------------- %
function y_evals = showPdf( bounds, N,centers, covariances, weights, color )
x_evals = [bounds(1):abs(diff(bounds))/N:bounds(2)] ;
y_evals = evaluateDistributionAt( centers, weights, covariances, x_evals ) ;

plot ( x_evals, y_evals, color )