#! /usr/bin/env python
# -*- coding: latin-1 -*-

import itertools

import mapltypes as types
import predicates

class FunctionTable(dict):
    """This class is used to store and retrieve PDDL Function objects
    according to name and argument types."""
    
    def __init__(self, functions=[]):
        """Create a new FunctionTable.

        Arguments:
        functions -- List of Function objects this table should contain."""
        for f in functions:
            self.add(f)

    def copy(self):
        """Create a copy of this table."""
        c =  FunctionTable()
        for f in self:
            c.add(f)
        return c

    def add(self, function):
        """Add a new Function to the table. If a function with
        identical name and arguments exists, an Exception will be
        raised.

        Arguments:
        function -- Function object or list of Function objects .
        """
        if isinstance(function, (list, tuple, set)):
            for f in function:
                self.add(f)
            return
                
        if function.name not in self:
            dict.__setitem__(self, function.name, set())
        else:
            if self.get(function.name, function.args):
                raise Exception("A function with this name and arguments already exists: " + str(function))

        dict.__getitem__(self, function.name).add(function)
        

    def remove(self, function):
        """Remove Functions from this table.

        Arguments:
        function -- Function object or list of Function objects .
        """
        if isinstance(function, (list, tuple, set)):
            for f in function:
                self.remove(f)
            return
        
        if function not in self:
            return
        
        fs = dict.__getitem__(self, function.name)
        fs.remove(function)
        if not fs:
            del self[function.name]
                
        
    def get(self, name, args):
        """Get all functions matching the provided name and argument
        types.

        If excactly one function matches the supplied information, it
        will be returned. Otherwise a list of matching Functions will
        be returned.

        Arguments:
        name -- the name of the function to look for
        args -- a list of Terms, TypedObjects or Types that describe
        the arguments of the function that is searched."""
        
        if name not in self:
            return []
        
        fs = dict.__getitem__(self, name.lower())
        
        argtypes = []
        for arg in args:
            if isinstance(arg, predicates.Term):
                argtypes.append(arg.get_type())
            elif isinstance(arg, types.TypedObject):
                argtypes.append(arg.type)
            elif isinstance(arg, types.Type):
                argtypes.append(arg)
            else:
                raise Exception("Wrong type for argument list:" + str(type(arg)))
                
        result = []
#        print name, map(str, args)
        for f in fs:
            #in case one tries to check the function argument, too
            if len(argtypes) == len(f.args):
                funcargs = {}
                matches = True
                for t, arg in zip(argtypes, f.args):
                    argtype = arg.type
                    #make sure that "typeof(?f)" parameters are checked correctly
                    if isinstance(arg.type, types.ProxyType) and arg.type.parameter in funcargs:
                        argtype = funcargs[arg.type.parameter]
                        
                    if not t.equal_or_subtype_of(argtype):
                        matches = False
                        break
                    
                    if isinstance(arg.type, types.FunctionType):
                        funcargs[arg] = t.type
                if matches:
#            if len(argtypes) == len(f.args) and all(map(lambda t, fa: , argtypes, f.args)):
#                print f
#                print map(lambda t, fa: "%s <= %s: %s" %(str(t), str(fa.type),str(t.equal_or_subtype_of(fa.type))), argtypes, f.args)
                    result.append(f)

        if len(result) == 1:
            return iter(result).next()
        return list(result)

    def __iter__(self):
        return itertools.chain(*self.itervalues())

    def __contains__(self, key):
        if isinstance(key, predicates.Function):
            if dict.__contains__(self, key.name):
                return key in dict.__getitem__(self, key.name)
            return False
        
        key = key.lower()
        return dict.__contains__(self, key)
        
    def __getitem__(self, key):
        if isinstance(key, predicates.Function):
            key = key.name
        key = key.lower()

        result = dict.__getitem__(self, key)
        #if len(result) == 1:
        #    return iter(result).next()
        return list(result)
        
    def __setitem__(self, key, value):
        raise NotImplementedError

class Scope(dict):
    """This class represents any PDDL object that can define variables
    or constants. It implements a lookup table to get the associated
    Parameters or TypedObjects given their name. Most methods of this
    class can take strings, TypedObjects or Terms as keys. For the
    latter two, the name of the objects will be used as a key.

    Scopes can be nested; when a symbol is not found in one scope, it
    will be looked up in it's parent Scope.

    Usually, a Scope object will not be used directly but be inherited
    by classes representing PDDL elements, e.g. domains, actions,
    quantified conditions/effects.
    """
    
    def __init__(self, objects, parent):
        """Create a new Scope object.

        Arguments:
        objects -- List of Parameters and TypedObjects that should be defined in this scope.
        parent -- Scope that this scope resides in. May be None if no parent exists.
        """
        
        self.set_parent(parent)
        self.termcache = {}
        for obj in objects:
            dict.__setitem__(self, obj.name, obj)

    def set_parent(self, parent):
        """Change the parent of this Scope object.

        Arguments:
        parent -- Scope object."""
        self.parent = parent
        if parent is not None:
            self.predicates = parent.predicates
            self.functions = parent.functions
            self.types = parent.types
            assert not "false" in self.types
            self.requirements = parent.requirements
        else:
            self.predicates = FunctionTable()
            self.functions = FunctionTable()
            self.types = {}
            self.requirements = set()

    
    def lookup(self, args):
        """Lookup a list of symbols in this Scope. Returns a list of
        Terms corresponding to the supplied symbols.

        Arguments:
        args -- list of strings, TypesObjects or Terms to look up"""
        result = []
        for arg in args:
            if arg.__class__ == predicates.FunctionTerm:
                result.append(predicates.FunctionTerm(arg.function, self.lookup(arg.args)))
            else:
                res = self[arg]
                if res not in self.termcache:
                    self.termcache[res] = predicates.Term(res)
                result.append(self.termcache[res])
                #result.append(predicates.Term(self[arg]))

        return result

    def add(self, obj):
        """Add an object to this Scope.

        Arguments:
        obj -- TypedObject or Parameter to add."""
        if isinstance(obj, (tuple, list)):
            for o in obj:
                dict.__setitem__(self, o.name, o)
        else:
            dict.__setitem__(self, obj.name, obj)

    # def merge(self, other):
    #     #assume unique variables for now
    #     for entry in other.values():
    #         name = entry.name
    #         #reassign existing variables
    #         if name in self:
    #             raise KeyError, "Variable %s already exists" % name
                
    #         dict.__setitem__(self, entry.name, entry)

    def tryInstantiate(self, mapping):
        """Try to instantiate parameters in this scope. If
        instantiation fails (see instantiate() method), this method
        will return False, otherwise True.

        Arguments:
        mapping -- dictionary from parameter to object."""
        try:
            self.instantiate(mapping)
        except:
            self.uninstantiate()
            return False
        return True
            
    def instantiate(self, mapping):
        """Instantiate Parameters. All parameters and values must be
        defined in this Scope or one of its ancestors. An exception is
        instantiating a functional variable with a FunctionTerm, here
        the FunctionTerm is not looked up in the scope.

        If any of the objects are not defined, an Exception will be raised.

        Arguments:
        mapping -- dictionary from parameter to object."""
        self.uninstantiate()
        nonfunctions = []
        #instantiate function variables first, as they can affect the types of other parameters
        for key, val in mapping.iteritems():
            key = self[key]
            
            if not isinstance(key, types.Parameter):
                raise TypeError("Cannot instantiate %s: it is no Parameter but %s" % (key, type(key)))
            
            if isinstance(val, predicates.FunctionTerm):
                key.instantiate(val)
            else:
                nonfunctions.append((key, val))
            
        for key,val in nonfunctions:
            val = self[val]
            key.instantiate(val)

            
    def uninstantiate(self):
        """Uninstantiate all Parameters defined in this Scope."""
        for val in self.itervalues():
            if isinstance(val, types.Parameter):
                val.instantiate(None)

    def uniquify_variables(self):
        """Rename objects to that there are no name collisions."""
        renamings = {}
        for entry in self.values():
            if entry.name in self.parent:
                i = 2
                while "%s%d" % (entry.name, i) in self.parent:
                    i += 1
                del self[entry.name]
                newname =  "%s%d" % (entry.name, i)
                renamings[entry.name] = newname
                entry.name = newname
                dict.__setitem__(self, newname, entry)
        return renamings
                
    def __contains__(self, key):
        if isinstance(key, (predicates.ConstantTerm, predicates.VariableTerm)):
            key = key.object
        if isinstance(key, (float, int)):
            return types.TypedNumber(key)
        if isinstance(key, types.TypedObject):
            if key.type == types.t_number:
                return True
            key = key.name
            
        key = key.lower()

        if dict.__contains__(self, key):
            return True
        if self.parent:
            return key in self.parent
        
    def __getitem__(self, key):
        #print type(key)
        if isinstance(key, predicates.VariableTerm):
            key = key.object.name
        elif isinstance(key, str):
            pass
        elif isinstance(key, predicates.ConstantTerm):
            if key.object.type == types.t_number:
                return key.object
            key = key.object.name
        elif isinstance(key, types.TypedObject):
            if key.type == types.t_number:
                return key
            key = key.name
        elif isinstance(key, (float, int)):
            return types.TypedNumber(key)
        # if isinstance(key, types.TypedObject):
        #     if key.type == types.t_number:
        #         return key
        #     key = key.name
        
        key = key.lower()

        if dict.__contains__(self, key):
            return dict.__getitem__(self, key)
        if self.parent:
            return self.parent[key]

        raise KeyError, "Symbol %s not found." % key
        
    def __setitem__(self, key, value):
        raise NotImplementedError

    def __hash__(self):
        return object.__hash__(self)
    def __eq__(self, other):
        return self.__hash__() == other.__hash__()
    def __nonzero__(self):
        return True


