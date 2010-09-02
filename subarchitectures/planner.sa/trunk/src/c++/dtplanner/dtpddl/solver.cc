/* Copyright (C) 2010 Charles Gretton (charles.gretton@gmail.com)
 *
 * Authorship of this source code was supported by EC FP7-IST grant
 * 215181-CogX.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * CogX ::
 *
 * Dear CogX team member :: Please email (charles.gretton@gmail.com)
 * if you make a change to this and commit that change to SVN. In that
 * email, can you please attach the source files you changed as they
 * appeared before you changed them (i.e., fresh out of SVN), and the
 * diff files (*). Alternatively, you could not commit your changes,
 * but rather post me a patch (**) which I will test and then commit
 * if it does not cause any problems under testing.
 *
 * (*) see http://www.gnu.org/software/diffutils/diffutils.html --
 * GNU-09/2009
 *
 * (**) see http://savannah.gnu.org/projects/patch -- GNU-09/2009
 *
 */

#include "solver.hh"

#include "dtp_pddl_parsing_data_problem.hh"
#include "dtp_pddl_parsing_data_domain.hh"
#include "problem_grounding.hh"

#include "planning_state.hh"

#include "action__literal.hh"
#include "action__state_transformation.hh"
#include "action__probabilistic_state_transformation.hh"

#include "state_formula__literal.hh"
#include "state_formula__disjunctive_clause.hh"
#include "state_formula__conjunctive_normal_form_formula.hh"

using namespace Planning;
using namespace Planning::Parsing;


Are_Doubles_Close Solver::are_Doubles_Close(1e-9);


std::pair<Planning::Formula::Action_Proposition, uint>
Solver::get_prescribed_action(POMDP_State* state)
{
    auto belief = state->get__belief_state();
    uint index = random() % belief.size();
    POMDP_State::Belief_Atom atom = belief[index];


    assert(dynamic_cast<State*>(atom.first));
    
    return get_prescribed_action(dynamic_cast<State*>(atom.first));//mdp_state);
}

/*Let's remove a few bugs at a time ;) */
std::pair<Planning::Formula::Action_Proposition, uint>
Solver::get_prescribed_action(State* current_state)
{
    basic_type::Runtime_Thread runtime_Thread = reinterpret_cast<basic_type::Runtime_Thread>
        (dynamic_cast<const Planning::Problem_Grounding*>(problem_Grounding.get()));

    INTERACTIVE_VERBOSER(true, 10015, "Getting prescribed action from :: "
                         <<*current_state<<std::endl);
    
    auto executable_action_indices = current_state->get__successor_Driver();
    auto _action_index = random() % executable_action_indices.size();
    auto action_index = executable_action_indices[_action_index];
    
    
    QUERY_UNRECOVERABLE_ERROR(!Formula::State_Proposition::
                              ith_exists(runtime_Thread, action_index)
                              , "Could not find a ground symbol associated with index :: "
                              << action_index);
    
    INTERACTIVE_VERBOSER(true, 10015,
                         "Got successor driver :: "<<action_index<<" "
                         <<State_Transformation::
                         make_ith<State_Transformation>
                         (runtime_Thread,
                          action_index).get__identifier()
                         <<"For atom :: "<<*current_state<<std::endl);
    
    auto symbol = State_Transformation::
        make_ith<State_Transformation>
        (runtime_Thread,
         action_index);
    auto identifier = symbol.get__identifier();
    auto id_value =  symbol.get__id();
    std::pair<Planning::Formula::Action_Proposition, uint> result(identifier, id_value);
    
    return result;
}

Observational_State* Solver::find_observation(Observational_State* observation_state)
{
    auto index = observation__space.find(observation_state);

    if(index == observation__space.end()){
        return 0;
    } else {
        return *index;
    }
}

POMDP_State* Solver::compute_successor(Observational_State* observation,
                                 uint action_index,
                                 POMDP_State* current_state)
{
    auto result =  current_state->get__successor(action_index, observation);
    
    QUERY_UNRECOVERABLE_ERROR
        (!result
         , "Unknown successor state"<<std::endl);

    return result;
}



POMDP_State* Solver::take_observation(POMDP_State* current_state,
                                      Observational_State* observation,
                                      uint action_index)
{
   
    auto successor_state
        = compute_successor(observation, action_index, current_state);

    
    expand_belief_state(successor_state);

    
    return successor_state;
}

POMDP_State* Solver::take_observation(POMDP_State* current_state,
                                const Percept_List& perceptions,
                                uint action_index)
{
    basic_type::Runtime_Thread runtime_Thread = reinterpret_cast<basic_type::Runtime_Thread>
        (dynamic_cast<const Planning::Problem_Grounding*>(problem_Grounding.get()));

    Observational_State* new_observation
        = new Observational_State(problem_Grounding->get__perceptual_Propositions().size());
    
    for(auto obs = perceptions.begin()
            ; obs != perceptions.end()
            ; obs++){
        std::string _predicate_name = (*obs).first;

        NEW_referenced_WRAPPED
            (domain_Data.get()
             , Planning::Percept_Name
             , percept_name
             , _predicate_name);
        
        Planning::Constant_Arguments constant_Arguments;
        for(auto _argument = (*obs).second.begin()
                ; _argument != (*obs).second.end()
                ; _argument++){

            std::string argument = *_argument;
            
            NEW_referenced_WRAPPED
                (&problem_Data//runtime_Thread
                 , Planning::Constant
                 , constant
                 , argument);
            constant_Arguments.push_back(constant);
        }
        
        NEW_referenced_WRAPPED_deref_visitable_POINTER
            (problem_Grounding.get()
             , Formula::Perceptual_Proposition
             , __proposition
             , percept_name
             , constant_Arguments);
        auto proposition =  Formula::Perceptual_Proposition__Pointer(__proposition);
        
        auto& perceptual_Propositions
            = get__problem_Grounding()->get__perceptual_Propositions();
        if(perceptual_Propositions.find(proposition) != perceptual_Propositions.end()){
            new_observation->flip_on(proposition->get__id());
        } else {
            WARNING("Unknown perceptual proposition :: "<<proposition);
        }
    }

    auto observation = find_observation(new_observation);
    delete new_observation;
    
    
    QUERY_UNRECOVERABLE_ERROR
        (!observation
         , "Unknown observation :: "<<*observation<<std::endl);

    return take_observation(current_state, observation, action_index);
}

Solver::Solver(Planning::Parsing::Problem_Data& problem_Data)
    :problem_Data(problem_Data),
     preprocessed(false),
     starting_belief_state(0),
     null_observation(0)// ,
//      constants_Description(0),
//      constants(0)
{
}

State& report__state(State&){
    UNRECOVERABLE_ERROR("unimplemented");
}

void Solver::preprocess()
{
    if(preprocessed) return;
    
    domain_Data = problem_Data.get__domain_Data();

    VERBOSER(3101, "Got domain :: "<<*domain_Data<<std::endl);
    
    
    
    proprocess__Constants_Data();
    
    configure__extensions_of_types();
    
    problem_Grounding = CXX__PTR_ANNOTATION(Problem_Grounding)
        (new Problem_Grounding(problem_Data,
                               domain_Data,
                               constants_Description,
                               extensions_of_types));

    problem_Grounding->ground_actions();
    problem_Grounding->ground_derived_predicates();
    problem_Grounding->ground_derived_perceptions();
    problem_Grounding->ground_starting_states();
    problem_Grounding->ground_objective_function();

    assert(problem_Grounding->get__deterministic_actions().size());
    problem_Grounding->ground_observations();
    QUERY_WARNING(!problem_Grounding->get__observations().size(),
                  "Problem has no observation schemata.");
//     assert(problem_Grounding->get__observations().size());
    
    generate_starting_state();
    
    
    preprocessed = true;
}



void Solver::domain_constants__to__problem_objects()
{
    VERBOSER(3001, "Adding domain constants to the problem description.");
    
//     const Constants_Data& problem__Constants_Data = problem_Data;
    const Constants_Data& domain__Constants_Data = *domain_Data;
    auto domain__constants_Description = domain__Constants_Data.get__constants_Description();
    
    for(auto _constant = domain__constants_Description.begin()
            ; _constant != domain__constants_Description.end()
            ; _constant++){

        const Constant& constant = _constant->first;
        
        VERBOSER(3001, "Adding domain constant :: "<<constant<<std::endl
                 <<"As problem object. "<<std::endl);
        
        problem_Data
            .add__constant(constant.get__name());
        
        auto types = domain__Constants_Data.get__constantx_types(constant);

        QUERY_UNRECOVERABLE_ERROR
            (!types.size(),
             "No types were specified for domain constant :: "<<constant<<std::endl);
        
        for(auto type = types.begin()
                ; type != types.end()
                ; type++){

            QUERY_UNRECOVERABLE_ERROR(domain_Data->get__types_description().find(*type)
                                      == domain_Data->get__types_description().end(),
                                      "Thread :: "<<domain_Data->get__types_description().begin()->first.get__runtime_Thread()<<std::endl
                                      <<"Got query from thread :: "<<type->get__runtime_Thread()<<std::endl
                                      <<"For domain at :: "<<reinterpret_cast<basic_type::Runtime_Thread>(domain_Data.get())<<std::endl);
            
            assert(type->get__runtime_Thread() == reinterpret_cast<basic_type::Runtime_Thread>(domain_Data.get()));
            assert(domain_Data->get__types_description().find(*type) != domain_Data->get__types_description().end());
            
            problem_Data
                .add__type_of_constant(type->get__name());
        }
        
        problem_Data
            .add__constants();
    }

    constants_Description = problem_Data.get__constants_Description();
    
}

void Solver::configure__extensions_of_types()
{
    /* For each problem constant. */
    for(auto constant_Description = constants_Description.begin()
            ; constant_Description != constants_Description.end()
            ; constant_Description++){
        auto types = constant_Description->second;
        auto constant = constant_Description->first;


        QUERY_UNRECOVERABLE_ERROR(types.size() > 1
                                  , "Each object is supposed to be of exactly one type."<<std::endl
                                  <<"However :: "<<constant<<" was declared with type :: "<<types<<std::endl);
        QUERY_UNRECOVERABLE_ERROR(types.size() == 0
                                  , "Each object is supposed to be of exactly one type."<<std::endl
                                  <<"However :: "<<constant<<" was declared without a type."<<std::endl);
        
        for(auto type = types.begin()
                ; type != types.end()
                ; type++){

            auto types_description = domain_Data->get__types_description();


//             std::string for_debug;
//             {
//                 std::ostringstream oss;
//                 oss<<*domain_Data<<std::endl;
//                 for(auto thing = types_description.begin()
//                         ; thing != types_description.end()
//                         ; thing++){
//                     oss<<thing->first<<" "<<thing->second<<std::endl;
//                 }
//                 for_debug = oss.str();  
//             }
            
            QUERY_UNRECOVERABLE_ERROR(types_description.find(*type) == types_description.end(),
                                      "Unable to find :: "<<*type<<std::endl
                                      <<"In the domain type hierarchy :from : "<<*domain_Data<<std::endl);

            if(extensions_of_types.find(*type) == extensions_of_types.end()){
                extensions_of_types[*type] = Constants();
            }

            VERBOSER(3001, "Adding :: "<<constant<<" of type ::"<<*type<<std::endl);
            
            extensions_of_types[*type].insert(constant);
            
            for(auto super_type = types_description.find(*type)->second.begin()
                    ; super_type != types_description.find(*type)->second.end()
                    ; super_type++){
                extensions_of_types[*super_type].insert(constant);
            }
        }
    }
}

void Solver::proprocess__Constants_Data()
{
    domain_constants__to__problem_objects();
    
    
}



bool Solver::sanity() const
{
    if(!preprocessed) {
        WARNING("Tested sanity on :: "<<problem_Data.get__problem_Name()
                <<"before preprocessing."<<std::endl);
        return false;
    }

    return true;
}



CXX__PTR_ANNOTATION(Problem_Grounding)  Solver::get__problem_Grounding()
{
    return problem_Grounding;
}
