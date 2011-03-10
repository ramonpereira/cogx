#include "cyclic_cg_heuristic.h"

#include "domain_transition_graph.h"
#include "globals.h"
#include "operator.h"
#include "state.h"
#include "search_space.h"

#include <algorithm>
#include <cassert>
#include <vector>
using namespace std;

/*
TODO: The responsibilities between the different classes need to be
      divided more clearly. For example, the LocalProblem
      initialization code contains some stuff (related to initial node
      initialization) that may better fit into the LocalProblemNode
      class. It's probably best to have all the code in the
      CyclicCGHeuristic class and have everything else be PODs.
      This would also get rid of g_HACK.
 */

CyclicCGHeuristic *g_HACK = 0;

bool debug = false;

inline void CyclicCGHeuristic::add_to_heap(LocalProblemNode *node) {
    int bucket_no = node->priority();
    if(bucket_no >= buckets.size()) {
        buckets.resize(max<size_t>(bucket_no + 1, 2 * buckets.size()));
        std::cout << "resize: " << buckets.size() << std::endl;
    }
    // std::cout << "push: " << bucket_no << " - " << heap_size+1  << std::endl;
    buckets[bucket_no].push_back(node);
    ++heap_size;
}

LocalTransition::LocalTransition(
    LocalProblemNode *source_, LocalProblemNode *target_,
    const ValueTransitionLabel *label_, int action_cost_, double action_prob_) {
    source = source_;
    target = target_;
    label = label_;
    action_cost = action_cost_;
    action_prob = action_prob_;

    // if (label->op)
    //     std::cout << "local transition: "<< label->op->get_name() << " " << action_cost << std::endl;
    // else
    //     std::cout << "local transition: (no op) " << action_cost << std::endl;

    // Set the following to explicitly invalid values.
    // They are initialized in on_source_expanded.
    target_cost = -1;
    target_action_costs = -1;
    target_prob = -1;
    unreached_conditions = -1;
}

inline void LocalTransition::try_to_fire() {
    if(!unreached_conditions && target_cost < target->cost) {
        target->action_cost = target_action_costs;
        target->prob = target_prob;
        target->cost = target->action_cost + (1-target->prob) * g_reward + 0.5;
        target->reached_by = this;
        g_HACK->add_to_heap(target);
    }
}

void LocalTransition::on_source_expanded(const State &state) {
    /* Called when the source of this transition is reached by
       Dijkstra exploration. Tries to compute cost for the target of
       the transition from the source cost, action cost, and set-up
       costs for the conditions on the label. The latter may yet be
       unknown, in which case we "subscribe" to the waiting list of
       the node that will tell us the correct value. */

    assert(source->cost >= 0);
    assert(source->cost < LocalProblem::QUITE_A_LOT);

    target_action_costs = source->action_cost + action_cost;
    target_prob = source->prob * action_prob;
    target_cost = target_action_costs + (1-target_prob) * g_reward + 0.5;
    // target_cost = source->cost + action_cost + (1-action_prob) * g_reward * g_multiplier;

    if(target->cost <= target_cost) {
        // Transition cannot find a shorter path to target.
        return;
    }

    unreached_conditions = 0;
    const vector<LocalAssignment> &precond = label->precond;
    if (debug && label->op) {
        std::cout << "open:" << label->op->get_name()  << " - " << target_cost << " / " << target_prob << std::endl;
        if (source->reached_by) {
            const ValueTransitionLabel* l = source->reached_by->label;
            std::cout << "reached by:" << l->op->get_name()  << " - " << source->cost << " / " << source->prob << std::endl;
        }
    }

    vector<LocalAssignment>::const_iterator
        curr_precond = precond.begin(),
        last_precond = precond.end();

    short *children_state = &source->children_state.front();
    vector<vector<LocalProblem *> > &problem_index = g_HACK->local_problem_index;
    int *parent_vars = &*source->owner->causal_graph_parents->begin();

    for(; curr_precond != last_precond; ++curr_precond) {
        int local_var = curr_precond->local_var;
        int current_val = children_state[local_var];
        // double current_prob = ((double) children_state[local_var].prob) / (1 << 15);
        int precond_value = curr_precond->value;
        int precond_var_no = parent_vars[local_var];
        
        if(current_val == precond_value) {
            continue;
        }

        LocalProblem *&child_problem = problem_index[precond_var_no][current_val];
        if(!child_problem) {
            child_problem = new LocalProblem(precond_var_no);
            g_HACK->local_problems.push_back(child_problem);
        }

        if(!child_problem->is_initialized())
            child_problem->initialize(source->priority(), current_val, state);
        LocalProblemNode *cond_node = &child_problem->nodes[precond_value];
        if(cond_node->expanded) {
            // target_cost = target_cost + target_prob * (cond_node->cost + (1-cond_node->prob) * g_multiplier * g_reward);
            target_action_costs += cond_node->action_cost;
            if (debug && label->op) {
                double new_target_prob = target_prob * cond_node->prob;
                int new_target_cost = target_action_costs + (1-target_prob) * g_reward +0.5;
                const ValueTransitionLabel* l = cond_node->reached_by->label;
                std::cout << "already expanded:" << l->op->get_name()  << " - " << cond_node->cost << " / " << cond_node->prob << std::endl;
                std::cout << "now:" << label->op->get_name()  << " - " << target_cost << " / " << target_prob << " ---> " << new_target_prob << " / " << new_target_cost  << std::endl;
            }
            target_prob *= cond_node->prob;
            target_cost = target_action_costs + (1-target_prob) * g_reward + 0.5;

            if(target->cost <= target_cost) {
                // Transition cannot find a shorter path to target.
                return;
            }
        } else {
            cond_node->add_to_waiting_list(this);
            ++unreached_conditions;
        }
    }
    try_to_fire();
}

void LocalTransition::on_condition_reached(int cost, double prob) {
    assert(unreached_conditions);
    --unreached_conditions;
    target_action_costs += cost;
    // target_cost = target_cost + target_prob * (cost + (1-prob) * g_multiplier * g_reward);
    if (debug && label->op) {
        double new_target_prob = target_prob * prob;
        int new_target_cost = target_action_costs + (1-new_target_prob) * g_reward + 0.5;
        std::cout << "condition reached: " << label->op->get_name() << " - " << cost << " / " << prob << std::endl;
        std::cout << target_action_costs << " - " << new_target_prob << std::endl;
        std::cout << "update: " << target_cost << " / " << target_prob << " ---> " << new_target_cost << " / " << new_target_prob  << std::endl;
    }
    target_prob *= prob;
    target_cost = target_action_costs + (1-target_prob) * g_reward + 0.5;
    // std::cout << "target " << std::endl;
    try_to_fire();
}

LocalProblemNode::LocalProblemNode(LocalProblem *owner_,
                                   int children_state_size) {
    owner = owner_;
    action_cost = -1;
    cost = -1;
    prob = 1.0;
    expanded = false;
    reached_by = 0;
    children_state.resize(children_state_size, -1);
}

void LocalProblemNode::add_to_waiting_list(LocalTransition *transition) {
    waiting_list.push_back(transition);
}

void LocalProblemNode::on_expand() {
    expanded = true;
    // Set children state unless this was an initial node.
    if(reached_by) {
        LocalProblemNode *parent = reached_by->source;
        children_state = parent->children_state;
        const vector<LocalAssignment> &precond = reached_by->label->precond;
        for(int i = 0; i < precond.size(); i++)
            children_state[precond[i].local_var] = precond[i].value;
        const vector<LocalAssignment> &effect = reached_by->label->effect;
        for(int i = 0; i < effect.size(); i++)
            children_state[effect[i].local_var] = effect[i].value;
        if(parent->reached_by)
            reached_by = parent->reached_by;
    }
    for(int i = 0; i < waiting_list.size(); i++)
        waiting_list[i]->on_condition_reached(action_cost, prob);
    waiting_list.clear();
}

void LocalProblem::build_nodes_for_variable(int var_no) {
    DomainTransitionGraph *dtg = g_transition_graphs[var_no];

    causal_graph_parents = &dtg->ccg_parents;

    int num_parents = causal_graph_parents->size();
    for(int value = 0; value < g_variable_domain[var_no]; value++)
        nodes.push_back(LocalProblemNode(this, num_parents));

    // Compile the DTG arcs into LocalTransition objects.
    for(int value = 0; value < nodes.size(); value++) {
        LocalProblemNode &node = nodes[value];
        const ValueNode &dtg_node = dtg->nodes[value];
        for(int i = 0; i < dtg_node.transitions.size(); i++) {
            const ValueTransition &dtg_trans = dtg_node.transitions[i];
            int target_value = dtg_trans.target->value;
            LocalProblemNode &target = nodes[target_value];
            for(int j = 0; j < dtg_trans.ccg_labels.size(); j++) {
                const ValueTransitionLabel &label = dtg_trans.ccg_labels[j];
                // int action_cost = dtg->is_axiom ? 0 : label.op->get_cost() + (g_reward * 0.5 * label.op->get_p_cost()) / g_multiplier;
                int action_cost = dtg->is_axiom ? 0 : label.op->get_cost();
                // std::cout << "build node: " << label.op->get_name() << " cost: " << label.op->get_cost() << " + " <<  g_reward << " * 0.5 * " << label.op->get_p_cost() << " = " << action_cost << std::endl;
                double action_prob = dtg->is_axiom ? 1.0 : label.op->get_prob();
                LocalTransition trans(&node, &target, &label, action_cost, action_prob);
                node.outgoing_transitions.push_back(trans);
            }
        }
    }
}

void LocalProblem::build_nodes_for_goal() {
    // TODO: We have a small memory leak here. Could be fixed by
    // making two LocalProblem classes with a virtual destructor.
    causal_graph_parents = new vector<int>;
    for(int i = 0; i < g_goal.size(); i++)
        causal_graph_parents->push_back(g_goal[i].first);

    for(int value = 0; value < 2; value++)
        nodes.push_back(LocalProblemNode(this, g_goal.size()));

    vector<LocalAssignment> goals;
    for(int goal_no = 0; goal_no < g_goal.size(); goal_no++) {
        int goal_value = g_goal[goal_no].second;
        goals.push_back(LocalAssignment(goal_no, goal_value));
    }
    vector<LocalAssignment> no_effects;
    ValueTransitionLabel *label = new ValueTransitionLabel(0, goals, no_effects);
    LocalTransition trans(&nodes[0], &nodes[1], label, 0, 1.0);
    nodes[0].outgoing_transitions.push_back(trans);
}

LocalProblem::LocalProblem(int var_no) {
    base_priority = -1;
    if(var_no == -1)
        build_nodes_for_goal();
    else
        build_nodes_for_variable(var_no);
}

void LocalProblem::initialize(int base_priority_, int start_value,
                              const State &state) {
    assert(!is_initialized());
    base_priority = base_priority_;

    for(int to_value = 0; to_value < nodes.size(); to_value++) {
        nodes[to_value].expanded = false;
        nodes[to_value].cost = QUITE_A_LOT;
        nodes[to_value].action_cost = QUITE_A_LOT;
        nodes[to_value].prob = 1.0;
        nodes[to_value].waiting_list.clear();
    }

    LocalProblemNode *start = &nodes[start_value];
    // start->prob = start_prob;
    start->cost = 0;
    start->action_cost = 0;
    start->prob = 1.0;
    for(int i = 0; i < causal_graph_parents->size(); i++)
        start->children_state[i] = state[(*causal_graph_parents)[i]];

    g_HACK->add_to_heap(start);
}

void LocalProblemNode::mark_helpful_transitions(const State &state) {
    assert(cost >= 0 && cost < LocalProblem::QUITE_A_LOT);
    if(reached_by) {
        if(reached_by->target_action_costs == reached_by->action_cost) {
            // Transition applicable, all preconditions achieved.
            const Operator *op = reached_by->label->op;
            assert(!op->is_axiom());
            assert(op->is_applicable(state));
            if (debug)
                std::cout << "helpful: " << op->get_name() << std::endl;
            g_HACK->set_preferred(op);
        } else {
            if (debug && reached_by->label->op)
                std::cout << "indirectly helpful: " << reached_by->label->op->get_name() << std::endl;

            // Recursively compute helpful transitions for precondition variables.
            const vector<LocalAssignment> &precond = reached_by->label->precond;
            int *parent_vars = &*owner->causal_graph_parents->begin();
            for(int i = 0; i < precond.size(); i++) {
                int precond_value = precond[i].value;
                int local_var = precond[i].local_var;
                int precond_var_no = parent_vars[local_var];
                if(state[precond_var_no] == precond_value)
                    continue;
                LocalProblemNode *child_node = &g_HACK->get_local_problem(
                    precond_var_no, state[precond_var_no])->nodes[precond_value];
                child_node->mark_helpful_transitions(state);
            }
        }
    }
}

void LocalProblemNode::compute_probability(const State &state, std::set<const Operator *>& ops) {
    assert(cost >= 0 && cost < LocalProblem::QUITE_A_LOT);
    if(reached_by) {
        const Operator *op = reached_by->label->op;
        if (op && ops.find(op) == ops.end())
            ops.insert(op);

        if(reached_by->target_cost == reached_by->action_cost) {
            // Transition applicable, all preconditions achieved.
        } else {
            // Recursively compute helpful transitions for precondition variables.
            const vector<LocalAssignment> &precond = reached_by->label->precond;
            int *parent_vars = &*owner->causal_graph_parents->begin();
            for(int i = 0; i < precond.size(); i++) {
                int precond_value = precond[i].value;
                int local_var = precond[i].local_var;
                int precond_var_no = parent_vars[local_var];
                if(state[precond_var_no] == precond_value)
                    continue;
                LocalProblemNode *child_node = &g_HACK->get_local_problem(
                    precond_var_no, state[precond_var_no])->nodes[precond_value];
                child_node->compute_probability(state, ops);
            }
        }
    }
}

CyclicCGHeuristic::CyclicCGHeuristic() {
    assert(!g_HACK);
    g_HACK = this;
    goal_problem = 0;
    goal_node = 0;
    heap_size = -1;
}

CyclicCGHeuristic::~CyclicCGHeuristic() {
    delete goal_problem;
    for(int i = 0; i < local_problems.size(); i++)
        delete local_problems[i];
}

void CyclicCGHeuristic::initialize() {
    assert(goal_problem == 0);
    cout << "Initializing cyclic causal graph heuristic..." << endl;

    int num_variables = g_variable_domain.size();

    goal_problem = new LocalProblem;
    goal_node = &goal_problem->nodes[1];

    local_problem_index.resize(num_variables);
    for(int var_no = 0; var_no < num_variables; var_no++) {
        int num_values = g_variable_domain[var_no];
        local_problem_index[var_no].resize(num_values, 0);
    }
}

int CyclicCGHeuristic::compute_heuristic(const State &state) {
    initialize_heap();
    goal_problem->base_priority = -1;
    for(int i = 0; i < local_problems.size(); i++)
        local_problems[i]->base_priority = -1;

    goal_problem->initialize(0, 0, state);

    int heuristic = compute_costs(state);

    if(heuristic != DEAD_END && heuristic != 0) {
        double p_init = 1.0;
        if (einfo != NULL) {
            p_init = einfo->get_p();
        }
        goal_node->mark_helpful_transitions(state);
        std::set<const Operator *> ops;
        goal_node->compute_probability(state, ops);
        double p = 1.0;
        std::set<const Operator *>::const_iterator i = ops.begin();
        for(; i != ops.end(); i++) {
            p *= (*i)->get_prob();
        }
        heuristic += p_init * (1-p) * g_reward;
    }

    if (debug) {
        cout << "h = " << heuristic << endl;
        exit(0);
    }

    return heuristic;
}

void CyclicCGHeuristic::initialize_heap() {
    /* This was just "buckets.clear()", but it may be advantageous to
       keep the empty buckets around so that there are fewer
       reallocations. At least changing this from buckets.clear() gave
       a significant speed boost (about 7%) for depots #10 on alfons.
    */
    for(int i = 0; i < buckets.size(); i++)
        buckets[i].clear();
    heap_size = 0;
}

int CyclicCGHeuristic::compute_costs(const State &state) {
    double p_init = 1.0;
    if (einfo != NULL) {
        p_init = einfo->get_p();
    }
    for(int curr_priority = 0; heap_size != 0; curr_priority++) {
        assert(curr_priority < buckets.size());
        for(int pos = 0; pos < buckets[curr_priority].size(); pos++) {
            // std::cout << curr_priority  << " - " << pos << " - " << heap_size << std::endl;
            LocalProblemNode *node = buckets[curr_priority][pos];
            if (debug) {
                if (node->reached_by && node->reached_by->label->op )
                    std::cout << node->reached_by->label->op->get_name()  << " - " << node->cost << " / " << node->prob << std::endl;
                else
                    std::cout << " - " << node->cost << " / " << node->prob << std::endl;
            }
            assert(node->owner->is_initialized());
            if(node->priority() < curr_priority)
                continue;
            if(node == goal_node) {
                if (debug) {
                    cout << "cea:" << node->prob << "    " << node->cost << endl;
                }
                if (node->cost == 0)
                    return 0;
                
                return node->action_cost;// + p_init * (1-node->prob) * g_reward;
                // return 1 + (1-node->prob) * g_reward;
            }

            assert(node->priority() == curr_priority);
            node->on_expand();
            for(int i = 0; i < node->outgoing_transitions.size(); i++)
                node->outgoing_transitions[i].on_source_expanded(state);
        }
        heap_size -= buckets[curr_priority].size();
        buckets[curr_priority].clear();
    }
    return DEAD_END;
}
