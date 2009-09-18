:- module world_model.

:- interface.
:- import_module string, int, map, set.
:- import_module lf, ontology.

:- type world_id(Index)
	--->	this
	;	u(int)  % unnamed
	;	i(Index)
	.

:- type world_model(Index, Sort, Rel).

:- func worlds(world_model(I, S, R)) = map(I, S) <= isa_ontology(S).
:- func reach(world_model(I, S, R)) = set({R, world_id(I), world_id(I)}) <= isa_ontology(S).
:- func props(world_model(I, S, R)) = map(world_id(I), set(proposition)) <= isa_ontology(S).

:- type world_model == world_model(string, string, string).

:- func init = world_model(I, S, R).

	% add_lf(M0, LF, M)
	% True iff
	%   M0 \/ model-of(LF) = M
	%
	% i.e. add LF to M0 so that it is consistent, fail if not possible.
	%
:- pred add_lf(world_model(I, S, R), lf(I, S, R), world_model(I, S, R)) <= isa_ontology(S).
:- mode add_lf(in, in, out) is semidet.

	% satisfies(M, LF)
	% True iff
	%   M |= LF
	%
:- pred satisfies(world_model(I, S, R), lf(I, S, R)) <= isa_ontology(S).
:- mode satisfies(in, in) is semidet.

	% Reduced model is a model for which it holds that for every reachability
	% relation R and worlds w1, w2, w3, it is true that
	%
	%   (w1, w2) \in R & (w1, w3) \in R -> w2 = w3
	%
	% i.e. every reachability relation is a (partial) function W -> W
	% rather than W -> pow(W)

	% reduced(M) = RM
	% True iff RM is a functionally reduced version of M.
	%
:- func reduced(world_model(I, S, R)::in) = (world_model(I, S, R)::out) is semidet <= isa_ontology(S).
:- func det_reduced(world_model(I, S, R)) = world_model(I, S, R) <= isa_ontology(S).


:- func lfs(world_model(I, S, R)) = set(lf(I, S, R)) <= isa_ontology(S).

%------------------------------------------------------------------------------%

:- implementation.
:- import_module solutions, require.
:- import_module list.

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%
	
:- type world_model(Index, Sort, Rel)
	--->	wm(
		worlds :: map(Index, Sort),  % sort
		reach :: set({Rel, world_id(Index), world_id(Index)}),  % reachability
		props :: map(world_id(Index), set(proposition)),  % valid propositions
		next_unnamed :: int  % lowest unused index for unnamed worlds
	).

%------------------------------------------------------------------------------%

init = wm(map.init, set.init, map.init, 0).

%init = wm(map.init, set.init, map.init, 0).

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- pred new_unnamed_world(int::out, world_model(I, S, R)::in, world_model(I, S, R)::out) is det.

new_unnamed_world(Id, WM0, WM) :-
	Id = WM0^next_unnamed,
	WM = WM0^next_unnamed := Id + 1.

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- pred rename_merge_world(world_id(I)::in, world_id(I)::in, world_model(I, S, R)::in,
		world_model(I, S, R)::out) is det <= isa_ontology(S).

rename_merge_world(Old, New, WM0, WM) :-
	Reach = set.map((func({Rel, OldIdA, OldIdB}) = {Rel, NewIdA, NewIdB} :-
		(OldIdA = Old -> NewIdA = New ; NewIdA = OldIdA),
		(OldIdB = Old -> NewIdB = New ; NewIdB = OldIdB)
			), WM0^reach),

	(if map.search(WM0^props, Old, OldsProps0)
	then OldsProps = OldsProps0
	else OldsProps = set.init
	),
	(if map.search(WM0^props, New, NewsProps0)
	then NewsProps = set.union(NewsProps0, OldsProps)
	else NewsProps = OldsProps
	),
	DelProps = map.delete(WM0^props, Old),
	(if NewsProps = set.init
	then Props = DelProps
	else Props = map.set(DelProps, New, NewsProps)
	),

	WM = wm(WM0^worlds, Reach, Props, WM0^next_unnamed).

%------------------------------------------------------------------------------%

add_lf(!.WM, LF, !:WM) :-
	add_lf0(this, _, LF, !WM).

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

:- pred add_lf0(world_id(I)::in, world_id(I)::out, lf(I, S, R)::in,
		world_model(I, S, R)::in, world_model(I, S, R)::out) is semidet <= isa_ontology(S).

add_lf0(Cur, Cur, at(of_sort(WName, Sort), LF), WM0, WM) :-
	% add the referenced world
	(if map.search(WM0^worlds, WName, OldSort)
	then
		% we've already been there
		NewSort = more_specific(OldSort, Sort)
	else
		% it's a new one
		NewSort = Sort
	),
	WM1 = WM0^worlds := map.set(WM0^worlds, WName, NewSort),
	add_lf0(i(WName), _, LF, WM1, WM).

add_lf0(Cur, i(WName), i(of_sort(WName, Sort)), WM0, WM) :-
	(
	 	Cur = this,
		fail  % should we perhaps allow this?
	;
		Cur = i(WName),
		map.search(WM0^worlds, WName, OldSort),
		NewSort = more_specific(OldSort, Sort),
		WM = WM0^worlds := map.set(WM0^worlds, WName, NewSort)
	;
		Cur = u(Num),
		(if map.search(WM0^worlds, WName, OldSort)
		then NewSort = more_specific(OldSort, Sort)
		else NewSort = Sort
		),
		WM1 = WM0^worlds := map.set(WM0^worlds, WName, NewSort),
		rename_merge_world(u(Num), i(WName), WM1, WM)
	).

add_lf0(Cur, Cur, r(Rel, LF), WM0, WM) :-
	new_unnamed_world(Num, WM0, WM1),
	WM2 = WM1^reach := set.insert(WM0^reach, {Rel, Cur, u(Num)}),
	add_lf0(u(Num), _, LF, WM2, WM).

add_lf0(Cur, Cur, p(Prop), WM0, WM) :-
	(if OldProps = map.search(WM0^props, Cur)
	then NewProps = set.insert(OldProps, Prop)
	else NewProps = set.make_singleton_set(Prop)
	),
	WM = WM0^props := map.set(WM0^props, Cur, NewProps).
	
add_lf0(Cur0, Cur, and(LF1, LF2), WM0, WM) :-
	add_lf0(Cur0, Cur1, LF1, WM0, WM1),
	add_lf0(Cur1, Cur, LF2, WM1, WM).

%------------------------------------------------------------------------------%

satisfies(M, LF) :-
	satisfies0(this, M, LF).

:- pred satisfies0(world_id(I)::in, world_model(I, S, R)::in, lf(I, S, R)::in) is semidet <= isa_ontology(S).

satisfies0(_WCur, M, at(of_sort(Idx, Sort), LF)) :-
	satisfies0(i(Idx), M, i(of_sort(Idx, Sort))),
	satisfies0(i(Idx), M, LF).

satisfies0(i(Idx), M, i(of_sort(Idx, LFSort))) :-
	map.search(M^worlds, Idx, Sort),
	isa(Sort, LFSort).

satisfies0(WCur, M, r(Rel, LF)) :-
	set.member({Rel, WCur, WReach}, M^reach),
	satisfies0(WReach, M, LF).

satisfies0(WCur, M, p(Prop)) :-
	set.member(Prop, map.search(M^props, WCur)).

satisfies0(WCur, M, and(LF1, LF2)) :-
	satisfies0(WCur, M, LF1),
	satisfies0(WCur, M, LF2).

%------------------------------------------------------------------------------%
	
:- type mergable_result(I)
	--->	merge(world_id(I), world_id(I))  % merge 2nd into 1st
	;	none
	.

reduced(WM) = RWM :-
	first_mergable_worlds(set.to_sorted_list(WM^reach), Res),
	(
	 	Res = merge(W2, W3),
		rename_merge_world(W3, W2, WM, WM0),
		RWM = reduced(WM0)
	;
		Res = none,
		RWM = WM
	).

det_reduced(M) = RM :-
	(if RM0 = reduced(M)
	then RM = RM0
	else error("model irreducible in func det_reduced/1")
	).

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%

	% Fails when a reachability relation that cannot be reduced to a partial
	% function is found.
:- pred first_mergable_worlds(list({R, world_id(I), world_id(I)})::in, mergable_result(I)::out) is semidet.

first_mergable_worlds([], none).
first_mergable_worlds([{R, W1, W2} | Rest], Result) :-
	(if
		list.find_first_map((pred({Rx, W1x, W3x}::in, W3x::out) is semidet :-
			Rx = R, W1x = W1), Rest, W3)
	then
		(W2 = u(_), W3 = u(_) -> Result = merge(W2, W3) ;
		(W2 = i(_), W3 = u(_) -> Result = merge(W2, W3) ;
		(W2 = u(_), W3 = i(_) -> Result = merge(W3, W2) ;
		fail  % W2 and W3 are both indexed by nominals => non-reducible
		)))
	else
		first_mergable_worlds(Rest, Result)
	).

%------------------------------------------------------------------------------%

lfs(M) = LFs :-
	LFs = solutions_set((pred(LF::out) is nondet :-
		(
			map.member(M^props, W, SetProps),
			W = i(Idx), M^worlds^elem(Idx) = Sort,
			set.member(Prop, SetProps),
			LF = at(of_sort(Idx, Sort), p(Prop))
		;
			set.member({Rel, W1, W2}, M^reach),
			W1 = i(Idx1), M^worlds^elem(Idx1) = Sort1,
			W2 = i(Idx2), M^worlds^elem(Idx2) = Sort2,
			LF = at(of_sort(Idx1, Sort1), r(Rel, i(of_sort(Idx2, Sort2))))
		)
			)).
