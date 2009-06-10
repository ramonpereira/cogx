package celmarchitecture.global;

/**
 *  This class holds some global compile time switches / settings
 *  for the C-ELM system.
 */
public class GlobalSettings {

    public static final boolean verbose               = true;
    public static final boolean singleSA              = true; // true;
    
    // for debugging:
    public static final boolean exitOnException       = false;
    public static final int     exitValueOnException  = 2;

    public static final double  defaultAtomicBuffer   = 0.01; // 20;

}
