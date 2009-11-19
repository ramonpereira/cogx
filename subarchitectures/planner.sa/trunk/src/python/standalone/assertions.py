from collections import defaultdict
import itertools
import config

import mapl_new as mapl
import state_new as state
import plans

log = config.logger("assertions")

def get_observable_functions(sensors):
    result=[]
    for s in sensors:
        pred_types = [a.getType() for a in s.get_term().args]
        if s.is_boolean():
            pred_types.append(s.get_value().getType())
        else:
            pred_types.append(s.get_term.getType())

        #Unify predicates with same/compatible types
        exists = False
        remove = []
        for function, types in result:
            if function != s.get_term().function:
                continue

            is_supertype = False
            is_subtype = False
            compatible = True
            for newtype, oldtype in zip(pred_types, types):
                if newtype == arg.getType():
                    continue
                elif newtype.isSubtypeOf(oldtype):
                    is_subtype = True
                elif newtype.isSupertypeOf(oldtype):
                    is_supertype = True
                else:
                    compatible = False
                    break

            #some parameters are subtypes, some are supertypes of the existing predicate
            #Those can't be unified
            if is_supertype and is_subtype:
                compatible = False
            #Remove existing predicate if current one is a strict superype
            if compatible and is_supertype:
                remove.append((function, types))
            elif is_equal:
                exists = True
                break

        for rem in remove:
            result.remove(rem)

        if not exists:
            result.append((s.get_term().function, pred_types))
        
    observable = defaultdict(list)
    for func, types in result:
        observable[func].append(types)
    return observable

def get_nonstatic_functions(domain):
    def effectVisitor(eff, results):
        if isinstance(eff, mapl.effects.SimpleEffect):
            if eff.predicate in mapl.predicates.assignmentOps:
                function = eff.args[0].function
            elif eff.predicate in mapl.predicates.mapl_modal_predicates:
                return []
            else:
                function = eff.predicate
            return [function]
        else:
            return sum(results, start=[])
                
    result = set()
    for a in domain.actions:
        for eff in a.effects:
            result |= set(eff.visit(effectVisitor))
    return result

def get_static_functions(domain):
    nonstatic = get_nonstatic_functions(domain)
    result = set()
    for func in itertools.chain(domain.predicates, domain.functions):
        if func not in nonstatic:
            result.add(func)

    return result
        
def is_observable(observables, term, value=None):
    if term.function not in observables:
        return False
    
    ttypes = [a.getType() for a  in term.args]
    if value:
        ttypes.append(value.getType())
    else:
        ttypes.append(term.function.type)

    for types in observables[term.function]:
        if all(map(lambda t1,t2: t1.equalOrSubtypeOf(t2), ttypes, types)):
            return True
    return False
        
def to_assertion(action, domain):
    if not action.precondition:
        return None
    
    observable = get_observable_functions(domain.sensors)
    agent = action.agents[0]

    new_indomain = set()

    def assertionVisitor(cond, results=[]):
        if cond.__class__ == mapl.conditions.LiteralCondition:
            if cond.predicate == mapl.predicates.knowledge:
                return None, cond
            if cond.predicate != mapl.predicates.equals or \
                    not is_observable(observable, cond.args[0], cond.args[1]) :
                return cond, None
            
            k_cond = mapl.conditions.LiteralCondition(mapl.predicates.knowledge, [mapl.predicates.VariableTerm(agent), cond.args[0]])
            id_cond = mapl.conditions.LiteralCondition(mapl.predicates.indomain, cond.args[:], negated = cond.negated)
            #print cond.pddl_str()

            return id_cond, k_cond
        elif isinstance(cond, mapl.conditions.JunctionCondition):
            condparts = []
            replanparts = []
            newcond = newreplan = None
            for pre, replan in results:
                if pre: condparts.append(pre)
                if replan: replanparts.append(replan)
            if condparts:
                newcond = cond.__class__(condparts)
            if replanparts:
                newreplan = cond.__class__(replanparts)
                    
            return newcond, newreplan
        elif cond.__class__ == mapl.conditions.QuantifiedCondition:
            newcond = newreplan = None
            if results[0][0]:
                newcond = cond.__class__(self.args, condparts)
            if results[0][1]:
                newreplan = cond.__class__(self.args, replanparts)
            return newcond, newreplan
        else:
            assert False

    condition, replan = action.precondition.visit(assertionVisitor)
    if not replan:
        return None

    assertion = mapl.actions.Action("assertion_"+action.name, action.agents, action.args, action.vars, condition, replan, action.effects, domain)
    assertion = assertion.copy()
    return assertion

def cluster_predecessors(cluster, plan):
    result = set()
    for node in cluster:
        result |= set(pred for pred in plan.predecessors_iter(node) if not isinstance(pred, plans.DummyAction))
    return result - cluster

def cluster_successors(cluster, plan):
    result = set()
    for node in cluster:
        result |= set(pred for pred in plan.successors_iter(node) if not isinstance(pred, plans.DummyAction))
    return result - cluster

def get_relevant_effects(cluster, plan):
    successors = cluster_successors(cluster, plan)
    predecessors = cluster_predecessors(cluster, plan)

    read_later = set()
    written = set()
    for pnode in cluster:
        written |= set(svar for svar,val in pnode.effects)
    for pnode in successors:
        read_later |= set(svar for svar,val in itertools.chain(pnode.preconds, pnode.replanconds))

    result = written & read_later 
    for pnode in predecessors:
        readonly = set(svar for svar,val in itertools.chain(pnode.preconds, pnode.replanconds)) - set(svar for svar,val in pnode.effects)
        threat_vars = readonly & written
        result |= threat_vars
    return result


def make_clusters(plan, domain):
    log = config.logger("assertions.clustering")
    results = set()
    initial_nodes = []
    static_functions = get_static_functions(domain)

    for node in plan.nodes_iter():
        if any(isinstance(pred.action, mapl.sensors.Sensor) for pred in plan.predecessors_iter(node)):
            initial_nodes.append(node)

            
    for node in initial_nodes:
        results |= add_to_cluster(set([node]), plan, static_functions, domain)
        

    log.info("Number of resulting clusters: %d", len(results))
    return results
    

def add_to_cluster(cluster, plan, static, domain):
    log = config.logger("assertions.clustering")
    
    def check_consistency(cluster, node):
        if any(plan.pred_closure(n) & cluster for n in plan.pred_closure(node) - cluster):
            return False
        if any(plan.succ_closure(n) & cluster for n in plan.succ_closure(node) - cluster):
            return False
        return True

    def get_svars(cluster, only_nonstatic=False, filter_func=None):
        if only_nonstatic:
            filter_func = lambda svar: svar.function not in static and svar.modality is None
        read = set()
        write = set()
        for pnode in plan.topological_sort():
            if pnode not in cluster:
                continue
            read |= set(svar for svar,val in itertools.chain(pnode.preconds, pnode.replanconds) if filter_func(svar)) - write
            write |= set(svar for svar,val in pnode.effects if filter_func(svar))
            
        return read, write, read|write

    def get_sensor_pres(cluster):
        read, _, _ = get_svars(cluster, filter_func=lambda svar: svar.modality == mapl.predicates.direct_knowledge)
        return read
    
    candidates = set()
    for a in cluster:
        candidates |= set(plan.predecessors(a) + plan.successors(a)) - cluster
    
    candidates.discard(plan.init_node)
    candidates.discard(plan.goal_node)

    add_always_candidates = set()
    branch_candidates = set()

    nonstatic_pre, nonstatic_eff, nonstatic_svars = get_svars(cluster, only_nonstatic=True)
    sensor_pre = get_sensor_pres(cluster)
    relevant_effs = get_relevant_effects(cluster, plan)

    log.debug("cluster: %s", map(str, cluster))
    
    for a in candidates:
        log.debug("considering %s ...", a)
        if not check_consistency(cluster, a):
            log.debug("inconsistent")
            continue

        nonstatic_pre2, nonstatic_eff2, nonstatic_svars2 = get_svars(cluster | set([a]), only_nonstatic=True)
        sensor_pre2 = get_sensor_pres(cluster | set([a]))
        relevant_effs2 = get_relevant_effects(cluster | set([a]), plan)

        log.debug("sensor preconditions: %d, %d", len(sensor_pre), len(sensor_pre2))
        sensors_added = len(sensor_pre2 - sensor_pre)
        sensors_removed = len(sensor_pre - sensor_pre2)

        nonstatic_pre_added = len(nonstatic_pre2 - nonstatic_pre - sensor_pre2)
        nonstatic_pre_removed = len(nonstatic_pre - nonstatic_pre2 - sensor_pre)

#        relevant_eff_added = len(relevant_effs2 - relevant_effs)
#        relevant_eff_removed = len(relevant_effs - relevant_effs2)

#        svars_added = len(relevant_effs2 | nonstatic_pre2 - (relevant_effs | nonstatic_pre))
#        svars_removed = len(relevant_effs | nonstatic_pre2 - (relevant_effs2 | nonstatic_pre2))

        #only non-negative traits:
#        if sensors_removed == 0 and nonstatic_pre_added == 0 and relevant_eff_added == 0:
        if sensors_removed == 0 and nonstatic_pre_added == 0:
            add_always_candidates.add(a)
            log.debug("added")
#        elif sensors_added - sensors_removed >= 0 and nonstatic_pre_removed - nonstatic_pre_added > 0:
        elif sensors_added - sensors_removed + nonstatic_pre_removed - nonstatic_pre_added > 0:
#        elif sensors_added - sensors_removed + svars_removed - svars_added > 0:
            branch_candidates.add(a)
            log.debug("branched")
        else:
            log.debug("not added")

        

    results = set()

    cluster |= add_always_candidates;
        
    if add_always_candidates:
        results |= add_to_cluster(cluster.copy(), plan, static, domain)
    elif len(cluster) > 1:
        results.add(frozenset(cluster))
        
    for c in branch_candidates:
        branch = cluster.copy()
        branch.add(c)
        results |= add_to_cluster(branch, plan, static, domain)

    return results
