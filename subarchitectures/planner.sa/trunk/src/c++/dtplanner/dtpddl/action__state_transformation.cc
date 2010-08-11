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

#include "action__state_transformation.hh"
#include "action__probabilistic_state_transformation.hh"

#include "planning_state.hh"

#include "state_formula__literal.hh"
#include "state_formula__disjunctive_clause.hh"
#include "state_formula__conjunctive_normal_form_formula.hh"


using namespace Planning;


Are_Doubles_Close State_Transformation::are_Doubles_Close = Are_Doubles_Close(1e-9);


State*
State_Transformation::operator()(State* in)
{
    const State_Formula::List__Literals& effects = get__effects();
    for(auto effect = effects.begin()
            ; effect != effects.end()
            ; effect++){
        /* If the effect is not satisfied in the state to which this
         * transformation is being applied.*/
        if(!(*effect)->is_satisfied(*in)){
            /* The the effect must be applied, because the parent
             * transformation was applied.*/
            (*effect)->flip_satisfaction(*in);
        }
    }

    /* If the action is not compulsory (i.e., is an agent executable
     * action), then wake up all the derivative actions.*/
    if(!get__compulsory()){
        auto listeners = Satisfaction_Listener::get__traversable__listeners();
        for(auto listener = listeners.begin()
                ; listener != listeners.end()
                ; listener++){
            (*listener).cxx_get<Satisfaction_Listener>()
                ->report__newly_satisfied(*in);
        }
    }

    /* If the actions is a compulsory derivative action, then it is no
     * longer applicable, because it was just now applied.*/
    if(get__compulsory()){
        report__newly_unsatisfied(*in);
    }
    
    
    return in;
}

const Formula::Action_Proposition& State_Transformation
::get__identifier() const
{
    return std::tr1::get<0>(contents());
}


const State_Formula::Conjunctive_Normal_Form_Formula__Pointer& State_Transformation
::get__precondition() const
{
    return std::tr1::get<1>(contents());
}


const State_Formula::List__Literals& State_Transformation
::get__effects() const
{
    return std::tr1::get<2>(contents());
}

bool State_Transformation
::get__compulsory() const
{
    return std::tr1::get<3>(contents());
}

bool State_Transformation
::get__lookup_probability() const
{
    return std::tr1::get<4>(contents());
}

double State_Transformation
::get__probability() const
{
    return std::tr1::get<5>(contents());
}

double State_Transformation
::get__probability(const State& state) const
{
    if(!get__lookup_probability()){
        return get__probability();
    } else {
        return state.get__float(std::tr1::get<6>(contents()));
    }
}


void State_Transformation
::set__satisfied(State& state)
{
    state.get__transformation__satisfaction_status().satisfy(get__id());
}

void State_Transformation
::set__unsatisfied(State& state)
{
    state.get__transformation__satisfaction_status().unsatisfy(get__id());
}

void State_Transformation
::flip_satisfaction(State& state)
{
    state.get__transformation__satisfaction_status().flip_satisfaction(get__id());
}

bool State_Transformation
::is_satisfied(const State& state) const
{
    return state.get__transformation__satisfaction_status().satisfied(get__id());
}

void State_Transformation
::increment__level_of_satisfaction(State& state)
{
    state.get__transformation__count_status().increment_satisfaction(get__id());
}

void State_Transformation
::decrement__level_of_satisfaction(State& state)
{
    state.get__transformation__count_status().decrement_satisfaction(get__id());
}

void State_Transformation
::set__level_of_satisfaction(uint level, State& state)
{
    state.get__transformation__count_status().set_satisfaction(get__id(), level);
}

uint State_Transformation
::get__level_of_satisfaction(State& state) const
{
    return state.get__transformation__count_status().get_satisfaction_level(get__id());
}




uint State_Transformation::get__number_of_satisfied_conditions(State& state) const
{
    return get__level_of_satisfaction(state);
}


void State_Transformation
::report__newly_satisfied(State& state)
{
    increment__level_of_satisfaction(state);
    
    uint satisfaction_requirement = 0;
    /*If the action has no precondition.*/
    if(get__precondition()->get__disjunctive_clauses().size() == 0){
        assert(get__compulsory());
        satisfaction_requirement = 1;
    } else
    /*If the action has a precondition.*/
    {
        /* If the action is not compulsary, then only its CNF
         * precondition has to be satisfied to make it
         * executable. Otherwise (if it is compulsary), the listener
         * action must have been executed, and the precondition CNF
         * must be satisfied.*/
        satisfaction_requirement = (get__compulsory())?2:1;
    }
    
    /*If this transformation is now executable.*/
    if(satisfaction_requirement == get__number_of_satisfied_conditions(state)){
        set__satisfied(state);
        
        if(get__compulsory()){
            
            if(get__lookup_probability()){
                WARNING("Unimplemented support for probability lookup.");
            }
            
            register double probability = get__probability(state);
            if(are_Doubles_Close(probability, 1.0)){
                state.push__compulsory_transformation(this);
            } else {
                state.push__compulsory_generative_transformation(this);
            }
            
            auto listeners = Satisfaction_Listener::get__traversable__listeners();
            for(auto listener = listeners.begin()
                    ; listener != listeners.end()
                    ; listener++){
                (*listener).cxx_get<Satisfaction_Listener>()
                    ->report__newly_satisfied(state);
            }
        } else {
            state.add__optional_transformation(this);
        }
    }
}

void State_Transformation
::report__newly_unsatisfied(State& state)
{
    decrement__level_of_satisfaction(state);

    if(is_satisfied(state)){
        set__unsatisfied(state);
        
        if(!get__compulsory()){    
            state.remove__optional_transformation(this);
        }
    }   
}

std::ostream& State_Transformation::operator<<(std::ostream&o) const
{
    o<<get__identifier()<<std::endl;

    
    o<<"PRECONDITION :: "<<get__precondition()<<std::endl;
    
    if(!get__lookup_probability()){
        o<<"Prob :: "<<get__probability()<<std::endl;
    } else {
        o<<"Prob :: LOOKUP"<<std::endl;
    }
    
    o<<"ADD :: ";
    /*add effects*/
    for(auto effect = get__effects().begin()
            ; effect != get__effects().end()
            ; effect++){
        if(!(*effect)->get__sign()){
            o<<*effect<<", ";
        }
    }
    o<<std::endl;

    
    /*delete effects*/
    
    o<<"DELETE :: ";
    /*add effects*/
    for(auto effect = get__effects().begin()
            ; effect != get__effects().end()
            ; effect++){
        if((*effect)->get__sign()){
            o<<*effect<<", ";
        }
    }
    o<<std::endl;
    o<<"{"<<get__traversable__listeners().size()<<"-LISTENERS :: ";
    for(auto listener = get__traversable__listeners().begin()
            ; listener != get__traversable__listeners().end()
            ; listener++){
        if(listener->test_cast<basic_type>()){
            listener->cxx_get<basic_type>()->operator<<(o);
        }
        
        
//         if(listener->test_cast<State_Transformation>()){
//             listener->cxx_get<State_Transformation>()->operator<<(o);
//         } else if (listener->test_cast<Probabilistic_State_Transformation>()) {
//             o<<*(listener->cxx_get<Probabilistic_State_Transformation>());//->operator<<(o);
//         }
    }
    o<<"}"<<std::endl;
    
    return o;
}

namespace std
{
    std::ostream& operator<<(std::ostream&o
                             , const Planning::State_Transformation&in)
    {
        return in.operator<<(o);
    }
    
}


