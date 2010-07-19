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



#include "problem_grounding.hh"


#include "dtp_pddl_parsing_data_domain.hh"

using namespace Planning;


Problem_Grounding::Problem_Grounding(Parsing::Problem_Data& problem_Data,
                                     CXX__PTR_ANNOTATION(Parsing::Domain_Data) domain_Data)
    :problem_Data(problem_Data),
     domain_Data(domain_Data)
{
//     QUERY_UNRECOVERABLE_ERROR
//         (State_Proposition::indexed__Traversable_Collection->end()
//          == State_Proposition::indexed__Traversable_Collection->find(&problem_Data),
//          "Could not find propositions associated with problem :: "
//          <<get__problem_Name()<<std::endl
//          );
//     propositions = *indexed__Traversable_Collection->find(&problem_Data);
    
}


void Problem_Grounding::groud_actions()
{
    for(auto action = domain_Data->get__action_Schemas().begin()
            ; action != domain_Data->get__action_Schemas().end()
            ; action ++){
        ground_action_schema(*action);
        
    }
}

void Problem_Grounding::ground_action_schema(const Planning::Action_Schema& actionSchema)
{
    
}

