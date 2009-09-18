:- module iceserv_lib.

:- interface.
:- import_module io.
:- import_module float, list.
:- import_module ctx_specific, ctx_modality, abduction, formula.
:- import_module varset.

:- func srv_init_ctx = ctx.
:- pred srv_clear_rules(ctx::in, ctx::out) is det.
:- pred srv_load_rules_from_file(string::in, ctx::in, ctx::out, io::di, io::uo) is det.
:- pred srv_clear_facts(ctx::in, ctx::out) is det.
:- pred srv_load_facts_from_file(string::in, ctx::in, ctx::out, io::di, io::uo) is det.
:- pred srv_add_mprop_fact(mprop(ctx_modality)::in, ctx::in, ctx::out) is det.
%:- pred srv_prove_best(string::in, float::in, ctx::in, float::out, proof(ctx_modality)::out) is semidet.
:- pred srv_prove_best(proof(ctx_modality)::in, ctx::in, float::out, proof(ctx_modality)::out) is semidet.
:- pred srv_dissect_proof(proof(ctx_modality)::in, ctx::in, float::out, list(string)::out, list(string)::out) is det.
:- pred srv_print_ctx(ctx::in, io::di, io::uo) is det.
:- pred srv_proof_summary(proof(ctx_modality)::in, ctx::in, io::di, io::uo) is det.

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- pred new_proof(list(with_cost_function(mprop(ctx_modality)))::in, varset::in, proof(ctx_modality)::out) is det.

:- pred new_with_const_cost_function(mprop(ctx_modality)::in, float::in, with_cost_function(mprop(ctx_modality))::out) is det.

:- pred new_mprop(ctx_modality::in, atomic_formula::in, mprop(ctx_modality)::out, varset::in, varset::out) is det.

:- pred new_atomic_formula(string::in, list(formula.term)::in, atomic_formula::out, varset::in, varset::out) is det.
:- pred new_term(string::in, list(formula.term)::in, formula.term::out, varset::in, varset::out) is det.
:- pred new_var(string::in, formula.term::out, varset::in, varset::out) is det.

:- pred new_varset(varset::out) is det.

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- pred empty_term_list(list(formula.term)::out) is det.
:- pred cons_term_list(formula.term::in, list(formula.term)::in, list(formula.term)::out) is det.

:- pred empty_annots_list(list(with_cost_function(mprop(ctx_modality)))::out) is det.
:- pred cons_annots_list(with_cost_function(mprop(ctx_modality))::in, list(with_cost_function(mprop(ctx_modality)))::in, list(with_cost_function(mprop(ctx_modality)))::out) is det.

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- pred modality_event(ctx_modality::out) is det.
:- pred modality_info(ctx_modality::out) is det.
:- pred modality_att(ctx_modality::out) is det.

%------------------------------------------------------------------------------%

:- implementation.
:- import_module set, map, list, bool, string, pair, bag, assoc_list.
:- import_module utils.
:- import_module term, term_io, formula.
:- import_module formula_io, formula_ops, ctx_io.
:- import_module solutions, require.
:- import_module varset, costs.
:- import_module modality, stringable, context, costs.
:- import_module belief_model, lf, stf.
:- import_module lf_io.
:- import_module varset.
:- import_module abd_io.

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- pragma foreign_export("C", srv_init_ctx = out, "init_ctx").

srv_init_ctx = new_ctx.

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- pragma foreign_export("C", srv_print_ctx(in, di, uo), "print_ctx").

srv_print_ctx(C, !IO) :-
   	print_ctx(C, !IO).

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- pragma foreign_export("C", srv_clear_rules(in, out), "clear_rules").

srv_clear_rules(!Ctx) :-
	set_explicit_rules(set.init, !Ctx).

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- pragma foreign_export("C", srv_load_rules_from_file(in, in, out, di, uo), "load_rules_from_file").

srv_load_rules_from_file(Filename, !Ctx, !IO) :-
	see(Filename, SeeRes, !IO),
	(SeeRes = ok -> true ; error("can't open the rule file")),

	do_while((pred(Continue::out, !.Ctx::in, !:Ctx::out, !.IO::di, !:IO::uo) is det :-
		term_io.read_term_with_op_table(init_wabd_op_table, ReadResult, !IO),
		(
			ReadResult = term(VS, Term),
			generic_term(Term),
			(if term_to_mrule(Term, MRule)
			then add_explicit_rule(vs(MRule, VS), !Ctx), Continue = yes
			else
				context(_, Line) = get_term_context(Term),
				error("Syntax error in rule file " ++ Filename
						++ " at line " ++ string.from_int(Line) ++ ".")
			)
		;
			ReadResult = error(Message, Linenumber),
			error(Message ++ " at line " ++ string.from_int(Linenumber) ++ ".")
		;
			ReadResult = eof,
			Continue = no
		)
			), !Ctx, !IO),

	seen(!IO).

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- pragma foreign_export("C", srv_clear_facts(in, out), "clear_facts").

srv_clear_facts(!Ctx) :-
	set_explicit_facts(set.init, !Ctx).

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- pragma foreign_export("C", srv_load_facts_from_file(in, in, out, di, uo), "load_facts_from_file").

srv_load_facts_from_file(Filename, !Ctx, !IO) :-
	see(Filename, SeeRes, !IO),
	(SeeRes = ok -> true ; error("can't open the rule file")),

	do_while((pred(Continue::out, !.Ctx::in, !:Ctx::out, !.IO::di, !:IO::uo) is det :-
		term_io.read_term_with_op_table(init_wabd_op_table, ReadResult, !IO),
		(
			ReadResult = term(_VS, Term),
			generic_term(Term),
			(if term_to_mprop(Term, m(Mod, Prop)), GProp = formula_to_ground_formula(Prop)
			then add_explicit_fact(m(Mod, GProp), !Ctx), Continue = yes
			else
				context(_, Line) = get_term_context(Term),
				error("Syntax error in facts file " ++ Filename
						++ " at line " ++ string.from_int(Line) ++ ".")
			)
		;
			ReadResult = error(Message, Linenumber),
			error(Message ++ " at line " ++ string.from_int(Linenumber) ++ ".")
		;
			ReadResult = eof,
			Continue = no
		)
			), !Ctx, !IO),

	seen(!IO).

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- pragma foreign_export("C", srv_add_mprop_fact(in, in, out), "add_mprop_fact").

srv_add_mprop_fact(MProp, !Ctx) :-
	MProp = m(Mod, Prop),
	GProp = det_formula_to_ground_formula(Prop),
	add_explicit_fact(m(Mod, GProp), !Ctx).

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- pragma foreign_export("C", srv_prove_best(in, in, out, out), "prove_best").

srv_prove_best(P0, Ctx, ProofCost, Proof) :-
%	VSMProp = vs(InitMProp, InitVarset),
%	vs(InitMProp, InitVarset) = det_string_to_vsmprop(GoalStr),

%	P0 = proof(vs([list.map((func(cf(MProp, Func)) = unsolved(MProp, Func)), AnnotMProps)], VS), []),

	Proofs0 = set.to_sorted_list(solutions_set((pred(Cost-P::out) is nondet :-
		Costs = costs(1.0, 1.0, 0.1),
		prove(0.0, 100.0, P0, P, Costs, Ctx),
		Cost = cost(Ctx, P, Costs)
			))),

	list.sort((pred(CA-_::in, CB-_::in, Comp::out) is det :-
		float_compare(CA, CB, Comp)
			), Proofs0, [ProofCost-Proof|_]).

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- pragma foreign_export("C", srv_dissect_proof(in, in, out, out, out), "dissect_proof").

srv_dissect_proof(Proof, _Ctx, Cost, Assumed, Asserted) :-
%	Costs = costs(1.0, 1.0, 0.1),
	Cost = 0.0,

	LastGoal = last_goal(Proof),
	Assumed = list.map((func(cf(m(Mod, GProp), _AssFunc)) = String :-
		String = mprop_to_string(varset.init, m(Mod, ground_formula_to_formula(GProp)))
			), bag.to_list(goal_assumptions(LastGoal))),

	Asserted = [],

	(if Asserted = []
	then true %error("conversion error!")
	else true
	).

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- pragma foreign_export("C", srv_proof_summary(in, in, di, uo), "proof_summary").

srv_proof_summary(Proof, Ctx, !IO) :-
	Costs = costs(1.0, 1.0, 0.1),
	LastGoal = last_goal(Proof),

	print_ctx(Ctx, !IO),

	format("proof cost = %f\n\n", [f(cost(Ctx, Proof, Costs))], !IO),
	print("proven goal:\n  " ++ goal_to_string(LastGoal) ++ "\n", !IO),
	nl(!IO),

	print("assumptions:\n", !IO),
	print("  " ++ assumptions_to_string(Ctx, goal_assumptions(LastGoal)) ++ "\n", !IO),
	nl(!IO),

	print("assertions:\n", !IO),
	print("  " ++ assertions_to_string(Ctx, goal_assertions(LastGoal)) ++ "\n", !IO),
	nl(!IO),

	print_proof_trace(Ctx, Proof, !IO).

%------------------------------------------------------------------------------%

:- pragma foreign_export("C", new_proof(in, in, out), "new_proof").

new_proof(Annots, VS, proof(vs([list.map((func(cf(MProp, Func)) = unsolved(MProp, Func)), Annots)], VS), [])).

:- pragma foreign_export("C", new_mprop(in, in, out, in, out), "new_mprop").

new_mprop(Mod, F, MProp, !VS) :-
	MProp = m([Mod], F).
%	trace [io(!IO)] (print("MProp = " ++ string(MProp), !IO), nl(!IO) ).

:- pragma foreign_export("C", new_atomic_formula(in, in, out, in, out), "new_atomic_formula").

new_atomic_formula(PredSym, Args, F, !VS) :-
	F = p(PredSym, Args).
%	trace [io(!IO)] (print("Atomic formula = " ++ string(F), !IO), nl(!IO) ).

:- pragma foreign_export("C", new_term(in, in, out, in, out), "new_term").

new_term(Functor, Args, T, !VS) :-
	T = t(Functor, Args).
%	trace [io(!IO)] (print("Term = " ++ string(T), !IO), nl(!IO) ).

:- pragma foreign_export("C", new_var(in, out, in, out), "new_var").

new_var(Name, T, !VS) :-
	varset.create_name_var_map(!.VS, VarNames),
	(if map.search(VarNames, Name, Var0)
	then Var = Var0
	else varset.new_named_var(!.VS, Name, Var, !:VS)
	),
	T = v(Var).
%	trace [io(!IO)] (print("Var = " ++ string(T), !IO), nl(!IO) ).

:- pragma foreign_export("C", new_varset(out), "new_varset").

new_varset(varset.init).

:- pragma foreign_export("C", new_with_const_cost_function(in, in, out), "new_with_const_cost_function").

new_with_const_cost_function(MProp, Cost, cf(MProp, const(Cost))).

%------------------------------------------------------------------------------%

:- pragma foreign_export("C", empty_term_list(out), "empty_term_list").
:- pragma foreign_export("C", cons_term_list(in, in, out), "cons_term_list").

empty_term_list([]).
cons_term_list(H, T, [H|T]).

:- pragma foreign_export("C", empty_annots_list(out), "empty_annots_list").
:- pragma foreign_export("C", cons_annots_list(in, in, out), "cons_annots_list").

empty_annots_list([]).
cons_annots_list(H, T, [H|T]).

%------------------------------------------------------------------------------%

:- pragma foreign_export("C", modality_event(out), "modality_event").
:- pragma foreign_export("C", modality_info(out), "modality_info").
:- pragma foreign_export("C", modality_att(out), "modality_att").

modality_event(e(now)).
modality_info(i).
modality_att(a(com)).
