#ifndef CLASSICAL_PLANNER_HH
#define CLASSICAL_PLANNER_HH

#include "CAST_SCAT/cast_scat.hh"

/* Header is automatically generated by ICE (Internet Communications
 * Engine -- ZeroC, Inc.) package \program{slice2cpp}.*/
#include "PCogX.hpp"

#ifndef CLASSICAL_PLANNER_DESIGNATION
#define CLASSICAL_PLANNER_DESIGNATION ""
#endif

using CAST_SCAT::Designator;
using CAST_SCAT::Designators;

class Classical_Planner :
    public CAST_SCAT::procedure_implementation<Classical_Planner>,
    public CAST_SCAT::procedure_call<>
{
public:
    typedef CAST_SCAT::procedure_implementation<Classical_Planner> Implement;
    typedef CAST_SCAT::procedure_call<> Call;

    Classical_Planner();
    Classical_Planner(const Designator& name);
    
    
//     explicit Classical_Planner(Designator&& name = CLASSICAL_PLANNER_DESIGNATION);
    
    void implement__distinctPlanner(PCogX::distinctPlannerPtr&);
    void implement__readPropositionIdentifiers(PCogX::readPropositionIdentifiersPtr&);
    void implement__postSTRIPSAction(PCogX::postSTRIPSActionPtr&);
    void implement__postActionDefinition(PCogX::postActionDefinitionPtr&);
    void implement__postTypes(PCogX::postTypesPtr&);
    void implement__postXXsubtypeofYY(PCogX::postXXsubtypeofYYPtr&);
    
    void runComponent();
protected:
    void start();
};

#endif
