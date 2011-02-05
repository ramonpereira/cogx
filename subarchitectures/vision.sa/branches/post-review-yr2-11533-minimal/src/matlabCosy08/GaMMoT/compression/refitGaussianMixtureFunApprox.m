function f0 = refitGaussianMixtureFunApprox( f0, f_ref, maxOptIterations )

minValueAtZero = 0 ; % what is considered zero
C_atError = 1 ; % default covariance value used at singularities
maxIterationsStages = 3 ; % maximum iteration stages per component
% maxOptIterations = 1 ; 

% modify allocation of covariance matrices
d = rows(f_ref.mu) ;
l_refs = cols(f_ref.weights) ;
f_ref.covariances = reshape(f_ref.covariances',d,d,l_refs) ;

d = rows(f0.mu) ; l_mod = cols(f0.weights) ;
f0.covariances = reshape(f0.covariances',d,d,l_mod) ;

% get length of the reference_target pdf
len_target = l_refs + l_mod - 1 ; 

% allocate the covariance matrix structure
C_len = len_target ;
C_inv = zeros(d,d,C_len) ;
C_det = zeros(1,C_len) ;



% debugShowTwoPdfs( f_ref, f0 ) ; drawnow ;
for i_opt_iterations = 1 : maxOptIterations
    f0_1 = f0 ;
    for i_comp_select = 1 : l_mod

        % select the component to optimize and initialize the mean, covariance
        % and weight of the new component
        t1 = f0.mu(:,i_comp_select) ;
        C1 = f0.covariances(:,:,i_comp_select) ;
        w1 = f0.weights(i_comp_select) ;

        % costruct target pdf
        id_f0_sel = [1:i_comp_select-1, i_comp_select+1:l_mod] ;
        f_t.mu = [f_ref.mu, f0.mu(:,id_f0_sel)] ;
        f_t.covariances = cat(3,f_ref.covariances, f0.covariances(:,:,id_f0_sel)) ;
        f_t.weights = [f_ref.weights, -f0.weights(id_f0_sel)] ;

        % initialize precalculated arrays
        for i = 1 : C_len
            C_inv(:,:,i) = inv( C1 + f_t.covariances(:,:,i) ) ;
            inv_sqrt_C_det(i) = sqrt(det(C_inv(:,:,i))) ;
        end

%         debugShow( f_ref, t1, C1, w1 ) ;

        % optimize the component
        t_init = zeros(d,1) ;
        C_init = zeros(d,d) ;
        optStage = 1 ;

        for G_iteration = 1 : maxIterationsStages
            if ( optStage == -1 ) break ; end

            switch optStage
                case 1 % optimizing mean value
                    for repeat = 1 : 1

                        x0 = t_init ;
                        B_norm = C_init ;
                        for j = 1 : len_target
                            C_j_inv = C_inv(:,:,j) ;
                            k_1j = exp(-0.5*sqdist(t1,f_t.mu(:,j),C_j_inv)) ;
                            A_1j = f_t.weights(j)*C_j_inv *inv_sqrt_C_det(i) ;
                            B_norm = B_norm + A_1j*k_1j ;
                            x0 = x0 + A_1j*k_1j* f_t.mu(:,j) ;
                        end
                        t1_new = inv(B_norm)*x0 ;
                        t1 = t1_new ;

%                         debugShow( f_t, t1, C1, w1 ) ; drawnow ;
                    end
                    % switch to new stage
                    optStage = 2 ;
                case 2 % optimizing covariance
                    B_norm = C_init ;
                    B_main = C_init ;
                    for j = 1 : len_target
                        %                 C_j_inv0 = inv( C1 + f_t.covariances(:,:,j) ) ;
                        %                 inv_sqrt_C_det0 = sqrt(det(C_j_inv0)) ;

                        % read stored inverses and determinants
                        C_j_inv0 = C_inv(:,:,j) ;
                        inv_sqrt_C_det0 = inv_sqrt_C_det(j) ;

                        % evaluate other parts
                        k_1j = exp(-0.5*sqdist(t1,f_t.mu(:,j),C_j_inv0)) ;
                        A_1j = f_t.weights(j)*C_j_inv0 *inv_sqrt_C_det0 ;

                        B_norm = B_norm + A_1j*k_1j ;
                        X = (f_t.mu(:,j) - t1)*transpose(f_t.mu(:,j) - t1) ;
                        B_main = B_main + A_1j*k_1j*( f_t.covariances(:,:,j) + 2*X*C_j_inv0*C1 ) ;
                    end
                    % avoid singularities at division by zero
                    if (abs(det(B_norm)) <= minValueAtZero | abs(det(B_main)) <= minValueAtZero )
                        C1_new = C_atError ;
%                         w1 = 0 ;
%                         optStage = 1 ; 
                    else
                        % calculate new covariance matrix
                        C1_new = inv(B_norm)*B_main ;
                        optStage = 3 ;
                    end


                    %             disp('Matrices'),[B_norm, B_main, C1_new]
                    %             disp('Determinants'),[det(B_norm), det(B_main)]

                    % make sure that the matrices are positive definite
                    C1_new = chol(C1_new'*C1_new) ;


                    C1 = C1_new ;

%                     debugShow( f_t, t1, C1, w1 ) ;

                    % switch to new stage
                    
                case 3 % optimizing weight
                    w1 = 0 ;
                    for j = 1 : len_target
                        C_j_inv0 = inv( C1 + f_t.covariances(:,:,j) ) ;
                        inv_sqrt_C_det0 = sqrt(det(C_j_inv0)) ;

                        % store partial computations
                        C_inv(:,:,j) = C_j_inv0 ;
                        inv_sqrt_C_det(j) = inv_sqrt_C_det0 ;

                        k_1j = exp(-0.5*sqdist(t1,f_t.mu(:,j),C_j_inv0)) ;
                        w1 = w1 + f_t.weights(j)*k_1j*inv_sqrt_C_det0 ;
                    end
                    w1 = (w1 * sqrt(det(2*C1))) ;

                    % perhaps it would be wise to remove the component if it
                    % recieves a very small weight -- currently this seems to be
                    % the only source of the singularities

                    %             w1
%                     debugShow( f_t, t1, C1, w1 ) ;

                    % component should be removed or rejouvinated
                    if ( abs(w1) <= minValueAtZero )
                        break ;
                    end
                    % switch to new stage
                    optStage = 1 ;
            end
        end

        if ( w1 == NaN ) w1 = 0 ; C1 = eye(d,d) ; end
        f0_1.mu(:,i_comp_select) = t1 ;
        f0_1.covariances(:,:,i_comp_select) = C1 ;
        f0_1.weights(i_comp_select) = w1 ;
        debugShowTwoPdfs( f_ref, f0_1 ) ; drawnow ;
    end
    f0 = f0_1 ;
end

f0.covariances = reshape(f0.covariances, length(f0.weights),1) ;

% --------------------------------------------------------------------- %
function debugShowTwoPdfs( f_ref, f_approx1 )

clf ;
f_ref.covariances = reshape(f_ref.covariances, length(f_ref.weights),1) ;
f_approx1.covariances = reshape(f_approx1.covariances, length(f_approx1.weights),1) ;
returnBounds = showDecomposedPdf( f_ref, 'linTypeSum', 'b', 'linTypeSub', 'b--' ) ;
showDecomposedPdf( f_approx1, 'bounds', returnBounds ) ;

% --------------------------------------------------------------------- %
function debugShow( f_ref, t1, C1, w1 )

clf ;
f_ref.covariances = reshape(f_ref.covariances, length(f_ref.weights),1) ;
f_approx1.mu = t1 ;
f_approx1.weights = w1 ;
f_approx1.covariances = reshape(C1,1,1) ;
returnBounds = showDecomposedPdf( f_ref, 'linTypeSum', 'b', 'linTypeSub', 'b--' ) ;
showDecomposedPdf( f_approx1, 'bounds', returnBounds ) ;
