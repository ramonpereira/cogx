#! /usr/bin/env python
# -*- coding: latin-1 -*-
import itertools

import parser
import mapltypes
import scope
import predicates, conditions, actions, effects, domain

from mapltypes import *
from parser import ParseError, UnexpectedTokenError
from actions import Action

def product(*iterables):
    for el in iterables[0]:
        if len(iterables) > 1:
            for prod in product(*iterables[1:]):
                yield (el,)+prod
        else:
            yield (el,)

class Problem(domain.MAPLDomain):
    def __init__(self, name, objects, init, goal, _domain, optimization=None, opt_func=None):
        domain.MAPLDomain.__init__(self, name, _domain.types, _domain.constants, _domain.predicates, _domain.functions, [], [], [])
        self.actions = [a.copy(self) for a in _domain.actions]
        self.sensors = [s.copy(self) for s in _domain.sensors]
        self.axioms = [a.copy(self) for a in _domain.axioms]
        self.stratifyAxioms()
        self.name2action = None
        
        for o in objects:
            self.add(o)
            
        self.domain = _domain
        self.requirements = set(self.domain.requirements)

        self.objects = set(o for o in objects)
        self.init = [l.copy(self) for l in init]
        self.goal = None
        if goal:
            self.goal = goal.copy(self)
        self.optimization = optimization
        self.opt_func = opt_func

    def copy(self):
        return Problem(self.name, self.objects, self.init, self.goal, self.domain, self.optimization, self.opt_func)

    def addObject(self, object):
        if object.name in self:
            self.objects.remove(self[object.name])
        self.objects.add(object)
        self.add(object)

    def getAll(self, type):
        if isinstance(type, FunctionType):
            for func in self.functions:
                if func.builtin:
                    continue
                #print func.name, types.FunctionType(func.type).equalOrSubtypeOf(type)
                if FunctionType(func.type).equalOrSubtypeOf(type):
                    combinations = product(*map(lambda a: list(self.getAll(a.type)), func.args))
                    for c in combinations:
                        #print FunctionTerm(func, c, self.problem)
                        yield predicates.FunctionTerm(func, c, self)
        else:
            for obj in itertools.chain(self.objects, self.constants):
                if obj.isInstanceOf(type):
                    yield obj
        

    @staticmethod
    def parse(root, domain):
        it = iter(root)
        it.get("define")
        j = iter(it.get(list, "(problem 'problem identifier')"))
        j.get("problem")
        probname = j.get(None, "problem identifier").token.string
        
        j = iter(it.get(list, "domain identifier"))
        j.get(":domain")
        domname = j.get(None, "domain identifier").token

        if domname.string != domain.name:
            raise ParseError(domname, "problem requires domain %s but %s is supplied." % (domname.string, domain.name))

        problem = None
        objects = set()
        
        for elem in it:
            j = iter(elem)
            type = j.get("terminal").token

            
            if type == ":objects":
                olist = mapltypes.parse_typelist(j)
                for key, value in olist.iteritems():
                    if value.string not in domain.types:
                        raise ParseError(value, "undeclared type")

                    objects.add(TypedObject(key.string, domain.types[value.string]))

                problem = Problem(probname, objects, [], None, domain)

            elif type == ":init":
                for elem in j:
                    if elem.isTerminal():
                        raise UnexpectedTokenError(elem.token, "literal or fluent assignment")
                        
                    init_elem = Problem.parseInitElement(iter(elem), problem)
                    problem.init.append(init_elem)
                    
            elif type == ":goal":
                cond = j.get(list, "goal condition")
                problem.goal = conditions.Condition.parse(iter(cond),problem)

            elif type == ":metric":
                opt = j.get("terminal", "optimization").token
                if opt.string not in ("minimize", "maximize"):
                    raise UnexpectedTokenError(opt, "'minimize' or 'maximize'")
                problem.optimization = opt.string
                
                problem.functions.add(predicates.total_time)
                func = predicates.Term.parse(j,problem)
                problem.functions.remove(predicates.total_time)

                j.noMoreTokens()

                if not isinstance(func.getType(), FunctionType):
                    raise ParseError(elem.token, "Optimization function can't be a constant.")
                if not func.getType().equalOrSubtypeOf(numberType):
                    raise ParseError(elem.token, "Optimization function must be numeric.")
                
                problem.opt_func = func

            else:
                raise ParseError(type, "Unknown section identifier: %s" % type.string)

        return problem

    @staticmethod
    def parseInitElement(it, scope):
        first = it.get("terminal").token
        if first.string == "probabilistic":
            #TODO: disallow nested functions in those effects.
            return effects.ProbabilisticEffect.parse(it.reset(), scope, timedEffects=False, onlySimple=True)[0]
        elif first.string == "assign-probabilistic":
            return effects.ProbabilisticEffect.parse_assign(it.reset(), scope)[0]
        else:
            scope.predicates.remove(predicates.equals)
            scope.predicates.add(predicates.equalAssign)
            if "fluents" in scope.requirements or "numeric-fluents" in scope.requirements:
                scope.predicates.remove(predicates.eq)
                scope.predicates.add(predicates.num_equalAssign)
            lit =  predicates.Literal.parse(it.reset(), scope, maxNesting=0)

            if "fluents" in scope.requirements or "numeric-fluents" in scope.requirements:
                scope.predicates.remove(predicates.num_equalAssign)
                scope.predicates.add(predicates.eq)
            scope.predicates.remove(predicates.equalAssign)
            scope.predicates.add(predicates.equals)

            return lit

