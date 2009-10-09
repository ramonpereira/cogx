///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2007 Michael White
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

package opennlp.ccg.parse;

import opennlp.ccg.synsem.*;
import java.util.*;
import java.text.*;

/**
 * <p>
 * An edge is a wrapper for a sign, ie a sign together 
 * with a score, and optionally a list of alternative edges. 
 * A representative edge is an edge that represents (stands in for) 
 * other edges with the same category (but different LFs) during the 
 * chart construction process, stored in the list of alternative edges; 
 * it is considered disjunctive when there is more than one alternative.
 * Note that initially a representative edge will be in its list 
 * of alternatives, but it can be removed during pruning.
 * </p>
 *
 * @author      Michael White
 * @version     $Revision: 1.2 $, $Date: 2007/12/20 21:30:22 $
 */
public class Edge {

	/** The sign. */
    public Sign sign;
    
    /** The edge score. */
    protected double score;
    
    /** The alternative edges (none initially). */
    protected List<Edge> altEdges = null;
    
    
    /** Constructor (score defaults to 0.0). */
    public Edge(Sign sign) { this(sign, 0.0); }

    /** Constructor with score. */
    public Edge(Sign sign, double score) {
        this.sign = sign; this.score = score; 
    }

    
    /** Returns the sign. */
    public Sign getSign() { return sign; }
    
    
    /** Returns the score. */
    public double getScore() { return score; }
    
    /** Sets the score. */
    public void setScore(double score) { this.score = score; }
    
    
    /** Returns whether this edge is a representative. */
    public boolean isRepresentative() { return altEdges != null; }
    
    /** Returns whether this edge is disjunctive. */
    public boolean isDisjunctive() { return altEdges != null && altEdges.size() > 1; }
    
    /** Returns the list of alt edges, or the empty list if none. */
    public List<Edge> getAltEdges() {
        if (altEdges == null) return Collections.emptyList(); 
        else return altEdges;
    }
    
    /** Initializes the alt edges list with a default capacity, adding this edge. */
    public void initAltEdges() { initAltEdges(3); }
    
    /** Initializes the alt edges list with the given capacity, adding this edge. */
    public void initAltEdges(int capacity) {
        // check uninitialized
        if (altEdges != null) throw new RuntimeException("Alt edges already initialized!");
        altEdges = new ArrayList<Edge>(capacity);
        altEdges.add(this);
    }
    
    
    /** Returns a hash code for this edge, based on its sign. (Alternatives and the score are not considered.) */
    public int hashCode() {
        return sign.hashCode() * 23;
    }
    
    /** Returns a hash code for this edge based on the sign's surface words. (Alternatives and the score are not considered.) */
    public int surfaceWordHashCode() {
    	return sign.surfaceWordHashCode() * 23;
    }
    
    /** Returns whether this edge equals the given object. (Alternatives and the score are not considered.) */
    public boolean equals(Object obj) {
        if (obj == this) return true;
        if (!(obj instanceof Edge)) return false;
        Edge edge = (Edge) obj;
        return sign.equals(edge.sign);
    }
    
    /** Returns whether this edge equals the given object based on the sign's surface words. (Alternatives and the score are not considered.) */
    public boolean surfaceWordEquals(Object obj) {
        if (obj == this) return true;
        if (!(obj instanceof Edge)) return false;
        Edge edge = (Edge) obj;
        return sign.surfaceWordEquals(edge.sign);
    }
    
    
    /**
     * Returns a string for the edge in the format
     * [score] orthography :- category. 
     */
    public String toString() {
        StringBuffer sbuf = new StringBuffer();
        if (score >= 0.001 || score == 0.0) sbuf.append("[" + nf3.format(score) + "] ");
        else sbuf.append("[" + nfE.format(score) + "] ");
        sbuf.append(sign.toString());
        return sbuf.toString();
    }
    
    // formats to three decimal places
    private static final NumberFormat nf3 = initNF3();
    private static NumberFormat initNF3() { 
        NumberFormat f = NumberFormat.getInstance();
        f.setMinimumIntegerDigits(1);
        f.setMinimumFractionDigits(3);
        f.setMaximumFractionDigits(3);
        return f;
    }
    
    // formats to "0.##E0"
    private static final NumberFormat nfE = new DecimalFormat("0.##E0");
}
