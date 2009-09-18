% $Id$

:- module formula.

:- interface.

:- import_module term, varset, pair, list, map, string.
:- import_module costs.
:- import_module modality.

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- type atomic_formula
	--->	p(
		string,  % predicate symbol
		list(formula.term)
	).

:- type ground_atomic_formula
	--->	p(
		string,  % predicate symbol
		list(ground_term)
	).

	% prolog term <--> formula.atomic_formula
:- func atomic_formula_to_term(atomic_formula) = term.term.

:- pred term_to_atomic_formula(term.term::in, atomic_formula::out) is semidet.
:- func term_to_atomic_formula(term.term) = atomic_formula is semidet.
:- func det_term_to_atomic_formula(term.term) = atomic_formula.

	% formula <--> ground formula
:- func formula_to_ground_formula(atomic_formula) = ground_atomic_formula is semidet.
:- func ground_formula_to_formula(ground_atomic_formula) = atomic_formula.

	% predicate version of the above
:- pred ground_formula(atomic_formula, ground_atomic_formula).
:- mode ground_formula(in, out) is semidet.
:- mode ground_formula(out, in) is det.

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- type term
	--->	v(var)
	;	t(string, list(formula.term))
	.

:- type ground_term
	---> 	t(string, list(ground_term))
	.

	% prolog term <--> formula.term
:- func formula_term_to_term(formula.term) = term.term.

:- pred term_to_formula_term(term.term::in, formula.term::out) is semidet.
:- func term_to_formula_term(term.term) = formula.term is semidet.
:- func det_term_to_formula_term(term.term) = formula.term.

	% term <--> ground term
:- func term_to_ground_term(formula.term) = ground_term is semidet.
:- func ground_term_to_term(ground_term) = formula.term.

	% predicate version of the above
:- pred ground_term(formula.term, ground_term).
:- mode ground_term(in, out) is semidet.
:- mode ground_term(out, in) is det.

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- type vscope(T)
	--->	vs(
		body :: T,
		vars :: varset
	).

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- type modalized(M, T)
	--->	m(
		m :: M,
		p :: T
	).

:- type with_cost_function(T)
	--->	cf(T, cost_function).

:- type mprop(M) == modalized(list(M), atomic_formula).
:- type mrule(M) == modalized(list(M), pair(list(with_cost_function(mprop(M))), mprop(M))).

:- type mgprop(M) == modalized(list(M), ground_atomic_formula).

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- type subst == map(var, formula.term).

:- func apply_subst_to_formula(subst, atomic_formula) = atomic_formula.
:- func apply_subst_to_mprop(subst, mprop(M)) = mprop(M) <= modality(M).

:- func rename_vars_in_term(map(var, var), formula.term) = formula.term.
:- func rename_vars_in_formula(map(var, var), atomic_formula) = atomic_formula.
:- func rename_vars_in_mprop(map(var, var), mprop(M)) = mprop(M) <= modality(M).
:- func rename_vars_in_annot_mprop(map(var, var), with_cost_function(mprop(M))) = with_cost_function(mprop(M))
		<= modality(M).
:- func rename_vars_in_mrule(map(var, var), mrule(M)) = mrule(M) <= modality(M).

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- pred unify_formulas(atomic_formula::in, atomic_formula::in, subst::out) is semidet.

%------------------------------------------------------------------------------%

:- implementation.

:- import_module require.
:- import_module int.
:- import_module term_io, parser, formula_io.

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

atomic_formula_to_term(p(PredSym, Args))
		= functor(atom(PredSym), list.map(formula_term_to_term, Args), context("", 0)).

term_to_atomic_formula(T, AF) :-
	AF = term_to_atomic_formula(T).

term_to_atomic_formula(functor(atom(PredSym), TermArgs, _))
		= p(PredSym, Args) :-
	list.map(term_to_formula_term, TermArgs, Args).

det_term_to_atomic_formula(T) = AF :-
	(if AF0 = term_to_atomic_formula(T)
	then AF = AF0
	else error("error in func det_term_to_atomic_formula/1 for \"" ++ string(T) ++ "\"")
	).

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

formula_term_to_term(v(Var)) = variable(Var, context("", 0)).
formula_term_to_term(t(F, Args)) = functor(atom(F), list.map(formula_term_to_term, Args), context("", 0)).

term_to_formula_term(T, FT) :-
	FT = term_to_formula_term(T).

term_to_formula_term(variable(Var, _)) = v(Var).
term_to_formula_term(functor(atom(Functor), TermArgs, _)) = t(Functor, Args) :-
	list.map(term_to_formula_term, TermArgs, Args).

det_term_to_formula_term(T) = FT :-
	(if FT0 = term_to_formula_term(T)
	then FT = FT0
	else error("error in func det_term_to_formula_term/1 for \"" ++ string(T) ++ "\"")
	).

%------------------------------------------------------------------------------%

apply_subst_to_mprop(Subst, m(M, Prop)) = m(M, apply_subst_to_formula(Subst, Prop)).

apply_subst_to_formula(Subst, p(F, Args)) = p(F, SubstArgs) :-
	SubstArgs0 = list.map((func(Arg) = SubstArg :-
		(
			Arg = t(Functor, TermArgs),
			SubstArg = t(Functor, TermArgs)
		;
			Arg = v(Var),
			(if Value = Subst^elem(Var)
			then SubstArg = Value
			else SubstArg = Arg
			)
		)), Args),

	(if SubstArgs0 = Args
	then SubstArgs = SubstArgs0
	else p(_, SubstArgs) = apply_subst_to_formula(Subst, p(F, SubstArgs0))
	).

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

rename_vars_in_term(Renaming, v(Var)) = v(map.lookup(Renaming, Var)).
rename_vars_in_term(Renaming, t(F, Args)) = t(F, SubstArgs) :-
	SubstArgs = list.map(rename_vars_in_term(Renaming), Args).

rename_vars_in_formula(Renaming, p(PS, Args)) = p(PS, SubstArgs) :-
	SubstArgs = list.map(rename_vars_in_term(Renaming), Args).

rename_vars_in_mprop(Renaming, m(M, Prop)) = m(M, rename_vars_in_formula(Renaming, Prop)).

rename_vars_in_annot_mprop(Renaming, cf(MProp, F)) = cf(rename_vars_in_mprop(Renaming, MProp), F).

rename_vars_in_mrule(Renaming, m(M, Ante-Succ)) =
		m(M, list.map(rename_vars_in_annot_mprop(Renaming), Ante)-rename_vars_in_mprop(Renaming, Succ)).

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

unify_formulas(A, B, U) :-
	unify_term(atomic_formula_to_term(A), atomic_formula_to_term(B), init, TermU),
	U = map.map_values((func(_, TermTgt) = det_term_to_formula_term(TermTgt)), TermU).

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

formula_to_ground_formula(p(PredSym, Args)) = p(PredSym, GroundArgs) :-
	list.map((pred(T::in, GT::out) is semidet :-
		GT = term_to_ground_term(T)
			), Args, GroundArgs).

ground_formula_to_formula(p(PredSym, GroundArgs)) = p(PredSym, list.map(ground_term_to_term, GroundArgs)).

:- pragma promise_equivalent_clauses(ground_formula/2).

ground_formula(ground_formula_to_formula(GroundFormula)::out, GroundFormula::in).
ground_formula(Formula::in, formula_to_ground_formula(Formula)::out).

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

term_to_ground_term(t(Functor, Terms)) = t(Functor, GroundTerms) :-
	list.map((pred(T::in, GT::out) is semidet :-
		GT = term_to_ground_term(T)
			), Terms, GroundTerms).

ground_term_to_term(t(Functor, GroundTerms)) = t(Functor, list.map(ground_term_to_term, GroundTerms)).

:- pragma promise_equivalent_clauses(ground_term/2).

ground_term(ground_term_to_term(GroundTerm)::out, GroundTerm::in).
ground_term(Term::in, term_to_ground_term(Term)::out).
