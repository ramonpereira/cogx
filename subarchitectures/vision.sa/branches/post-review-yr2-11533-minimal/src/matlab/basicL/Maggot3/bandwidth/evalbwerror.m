%%
% Originally a part of: Maggot (developed within EU project CogX)
% Author: Matej Kristan, 2009 (matej.kristan@fri.uni-lj.si; http://vicos.fri.uni-lj.si/matejk/)
% Last revised: 2009
%%
function f = evalbwerror( h, params )
%
% Error function for bandwidth estimation.
%
warning('----HACK-----> HERE should be more general!!!!') ;
% reestimate the pdf
H = getBWfromStruct( params.F{1}, h ) ;

% estimate the derivative model
derivativeModel = makeDerivativeModel( params.model_init, params.obs, ...
                                        'obs_mixing_weights', params.obs_mixing_weights,...
                                        'mix_weights', params.mix_weights,...
                                        'typeDerivative', 'useSourceBw',...
                                        'SourceBw', H,...
                                        'N_eff', params.N_eff ) ; 

% derivativeModel = augmentMixtureModelByThis( params.model_init, params.obs, H, params.obs_mixing_weights, params.mix_weights ) ;

% calculate the optimal bandwidth for that pdf
[H_opt, F_opt, h_amise] = getHoptFromThis( 'derivativeModel', derivativeModel,...  
                                           'model', params.model_init,... 
                                           'N_eff', params.N_eff,...
                                           'min_eigenvalue', params.min_eigenvalue, ...
                                           'forceDiagBwMatrix', params.forceDiagBwMatrix, ... 
                                           'forceBwMatrix', params.F, ...
                                           'typeBWoptimization', params.typeBWoptimization, ...
                                           'input_bandwidth', h ) ;                                       
                                       
f = (h - h_amise)^2 ;
