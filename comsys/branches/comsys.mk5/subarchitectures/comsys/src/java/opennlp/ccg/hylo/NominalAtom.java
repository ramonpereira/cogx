///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2003-4 Jason Baldridge and University of Edinburgh (Michael White)
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////////////

package opennlp.ccg.hylo;

import opennlp.ccg.grammar.*;
import opennlp.ccg.synsem.*;
import opennlp.ccg.unify.*;
import org.jdom.*;

/**
 * A hybrid logic nominal, an atomic formula which holds true at exactly one
 * point in a model.
 * The type is checked for compatibility during unification with nominal vars, 
 * but it is not updated, since nominal atoms are constants.
 * If no type is given, the TOP type is used for backwards compatibility.
 *
 * @author      Jason Baldridge
 * @author      Michael White
 * @version     $Revision: 1.9 $, $Date: 2005/11/01 19:39:27 $
 **/
public class NominalAtom extends HyloAtom implements Nominal {

    protected boolean shared = false;
    
    public NominalAtom(String name) {
        this(name, null);
    }
    
    public NominalAtom(String name, SimpleType st) {
        this(name, st, false);
    }
    
    public NominalAtom(String name, SimpleType st, boolean shared) {
        super(name, st);
        type = (st != null) ? st : Grammar.theGrammar.types.getSimpleType(Types.TOP_TYPE);
        this.shared = shared;
    }

    public String getName() { return _name; }
    
    public boolean isShared() { return shared; }

    public void setShared(boolean shared) { this.shared = shared; }
    
    public LF copy() {
        return new NominalAtom(_name, type, shared);
    }

    /** Returns a hash code based on the atom name and type. */
    public int hashCode() { 
        return _name.hashCode() + type.hashCode();
    }

    /**
     * Returns whether this atom equals the given object based on the atom name and type.
     */
    public boolean equals(Object obj) {
        if (!super.equals(obj)) return false;
        NominalAtom nom = (NominalAtom) obj;
        return type.equals(nom.type);
    }

    public Object unify(Object u, Substitution sub) throws UnifyFailure {
        if (equals(u)) return this;
        return super.unify(u, sub);
    }
    
    public int compareTo(Nominal nom) {
        if (nom instanceof NominalAtom) { 
            return super.compareTo((NominalAtom)nom);
        }
        int retval = _name.compareTo(nom.getName());
        if (retval == 0) { retval = -1; } // atom precedes var if names equal
        return retval;
    }
    
    public String toString() {
        String retval = _name;
        if (!type.getName().equals(Types.TOP_TYPE)) retval += ":" + type.getName();
        return retval;
    }
    
    /**
     * Returns an XML representation of this LF.
     */
    public Element toXml() {
        Element retval = new Element("nom");
        retval.setAttribute("name", toString());
        return retval;
    }
}
