/* Generated By:JavaCC: Do not edit this line. FormulaParser.java */
package de.dfki.lt.tr.dialmanagement.utils.formulaParser;

import java.util.List ;
import java.util.LinkedList ;
import java.util.Enumeration ;
import java.io.* ;
import java.util.Hashtable;


import de.dfki.lt.tr.beliefs.slice.logicalcontent.*;
import de.dfki.lt.tr.dialmanagement.arch.DialogueException;

public class FormulaParser implements FormulaParserConstants {

  static final public ElementaryFormula ElemFormulaExtended() throws ParseException {
String curString = "";
Token tok;
    jj_consume_token(QUOTE);
    label_1:
    while (true) {
      tok = jj_consume_token(WORD);
   curString += tok.toString() +" " ;
      switch ((jj_ntk==-1)?jj_ntk():jj_ntk) {
      case WORD:
        ;
        break;
      default:
        jj_la1[0] = jj_gen;
        break label_1;
      }
    }
    jj_consume_token(QUOTE);
         {if (true) return new ElementaryFormula(0, "\"" + curString.substring(0, curString.length()-1) + "\"");}
    throw new Error("Missing return statement in function");
  }

  static final public ElementaryFormula ElemFormula() throws ParseException {
Token tok;
    tok = jj_consume_token(WORD);
   {if (true) return new ElementaryFormula(0, tok.toString());}
    throw new Error("Missing return statement in function");
  }

  static final public IntegerFormula IntegerFormula() throws ParseException {
Token tok;
    tok = jj_consume_token(INTEGER);
   {if (true) return new IntegerFormula(0, Integer.parseInt(tok.toString()));}
    throw new Error("Missing return statement in function");
  }

  static final public BooleanFormula BooleanFormula() throws ParseException {
Token tok;
    tok = jj_consume_token(BOOLEAN);
   {if (true) return new BooleanFormula(0, Boolean.valueOf(tok.toString()));}
    throw new Error("Missing return statement in function");
  }

  static final public FloatFormula FloatFormula() throws ParseException {
Token tok;
    tok = jj_consume_token(FLOAT);
   {if (true) return new FloatFormula(0, Float.parseFloat(tok.toString()));}
    throw new Error("Missing return statement in function");
  }

  static final public ComplexFormula CFormulaConj() throws ParseException {
List<dFormula> subFormulae = new LinkedList<dFormula>();
dFormula formula;
    formula = SubFormula();
                   subFormulae.add(formula);
    label_2:
    while (true) {
      jj_consume_token(LAND);
      formula = SubFormula();
                   subFormulae.add(formula);
      switch ((jj_ntk==-1)?jj_ntk():jj_ntk) {
      case LAND:
        ;
        break;
      default:
        jj_la1[1] = jj_gen;
        break label_2;
      }
    }
         {if (true) return new ComplexFormula(0, subFormulae, BinaryOp.conj);}
    throw new Error("Missing return statement in function");
  }

  static final public ComplexFormula CFormulaDisj() throws ParseException {
List<dFormula> subFormulae = new LinkedList<dFormula>();
dFormula formula;
    formula = SubFormula();
    subFormulae.add(formula);
    label_3:
    while (true) {
      jj_consume_token(LOR);
      formula = SubFormula();
                   subFormulae.add(formula);
      switch ((jj_ntk==-1)?jj_ntk():jj_ntk) {
      case LOR:
        ;
        break;
      default:
        jj_la1[2] = jj_gen;
        break label_3;
      }
    }
         {if (true) return new ComplexFormula(0, subFormulae, BinaryOp.disj);}
    throw new Error("Missing return statement in function");
  }

  static final public dFormula SubFormula() throws ParseException {
dFormula formula;
dFormula formulaIn;
    switch ((jj_ntk==-1)?jj_ntk():jj_ntk) {
    case LBRACKET:
      jj_consume_token(LBRACKET);
      formula = GeneralFormula();
      jj_consume_token(RBRACKET);
      break;
    case NEG:
      jj_consume_token(NEG);
      jj_consume_token(LBRACKET);
      formulaIn = GeneralFormula();
      jj_consume_token(RBRACKET);
          formula = new NegatedFormula(0,formulaIn);
      break;
    case LANGLE:
      formula = ModalFormula();
      break;
    case BOOLEAN:
      formula = BooleanFormula();
      break;
    case FLOAT:
      formula = FloatFormula();
      break;
    case INTEGER:
      formula = IntegerFormula();
      break;
    case WORD:
      formula = ElemFormula();
      break;
    default:
      jj_la1[3] = jj_gen;
      jj_consume_token(-1);
      throw new ParseException();
    }
         {if (true) return formula;}
    throw new Error("Missing return statement in function");
  }

  static final public dFormula ModalFormula() throws ParseException {
Token tok;
dFormula underFormula;
    jj_consume_token(LANGLE);
    tok = jj_consume_token(WORD);
    jj_consume_token(RANGLE);
    underFormula = SubFormula();
    {if (true) return new ModalFormula(0,tok.toString(), underFormula);}
    throw new Error("Missing return statement in function");
  }

  static final public dFormula GeneralFormula() throws ParseException {
dFormula formula;
dFormula formulaIn;
    if (jj_2_1(3)) {
      jj_consume_token(LBRACKET);
      formula = GeneralFormula();
      jj_consume_token(RBRACKET);
    } else if (jj_2_2(3)) {
      jj_consume_token(NEG);
      jj_consume_token(LBRACKET);
      formulaIn = GeneralFormula();
      jj_consume_token(RBRACKET);
          formula = new NegatedFormula(0,formulaIn);
    } else if (jj_2_3(3)) {
      formula = ModalFormula();
    } else if (jj_2_4(2)) {
      formula = CFormulaConj();
    } else if (jj_2_5(2)) {
      formula = CFormulaDisj();
    } else {
      switch ((jj_ntk==-1)?jj_ntk():jj_ntk) {
      case BOOLEAN:
        formula = BooleanFormula();
        break;
      case FLOAT:
        formula = FloatFormula();
        break;
      case INTEGER:
        formula = IntegerFormula();
        break;
      case WORD:
        formula = ElemFormula();
        break;
      case QUOTE:
        formula = ElemFormulaExtended();
        break;
      default:
        jj_la1[4] = jj_gen;
        jj_consume_token(-1);
        throw new ParseException();
      }
    }
          {if (true) return formula;}
    throw new Error("Missing return statement in function");
  }

  static final public dFormula Input() throws ParseException {
dFormula formula;
    formula = GeneralFormula();
    jj_consume_token(0);
          {if (true) return formula;}
    throw new Error("Missing return statement in function");
  }

  static final private boolean jj_2_1(int xla) {
    jj_la = xla; jj_lastpos = jj_scanpos = token;
    try { return !jj_3_1(); }
    catch(LookaheadSuccess ls) { return true; }
    finally { jj_save(0, xla); }
  }

  static final private boolean jj_2_2(int xla) {
    jj_la = xla; jj_lastpos = jj_scanpos = token;
    try { return !jj_3_2(); }
    catch(LookaheadSuccess ls) { return true; }
    finally { jj_save(1, xla); }
  }

  static final private boolean jj_2_3(int xla) {
    jj_la = xla; jj_lastpos = jj_scanpos = token;
    try { return !jj_3_3(); }
    catch(LookaheadSuccess ls) { return true; }
    finally { jj_save(2, xla); }
  }

  static final private boolean jj_2_4(int xla) {
    jj_la = xla; jj_lastpos = jj_scanpos = token;
    try { return !jj_3_4(); }
    catch(LookaheadSuccess ls) { return true; }
    finally { jj_save(3, xla); }
  }

  static final private boolean jj_2_5(int xla) {
    jj_la = xla; jj_lastpos = jj_scanpos = token;
    try { return !jj_3_5(); }
    catch(LookaheadSuccess ls) { return true; }
    finally { jj_save(4, xla); }
  }

  static final private boolean jj_3R_16() {
    if (jj_scan_token(BOOLEAN)) return true;
    return false;
  }

  static final private boolean jj_3R_15() {
    if (jj_scan_token(LOR)) return true;
    return false;
  }

  static final private boolean jj_3R_7() {
    if (jj_3R_13()) return true;
    Token xsp;
    if (jj_3R_15()) return true;
    while (true) {
      xsp = jj_scanpos;
      if (jj_3R_15()) { jj_scanpos = xsp; break; }
    }
    return false;
  }

  static final private boolean jj_3R_5() {
    if (jj_scan_token(LANGLE)) return true;
    if (jj_scan_token(WORD)) return true;
    if (jj_scan_token(RANGLE)) return true;
    return false;
  }

  static final private boolean jj_3R_18() {
    if (jj_scan_token(INTEGER)) return true;
    return false;
  }

  static final private boolean jj_3R_14() {
    if (jj_scan_token(LAND)) return true;
    return false;
  }

  static final private boolean jj_3R_26() {
    if (jj_3R_18()) return true;
    return false;
  }

  static final private boolean jj_3R_12() {
    if (jj_3R_20()) return true;
    return false;
  }

  static final private boolean jj_3R_25() {
    if (jj_3R_17()) return true;
    return false;
  }

  static final private boolean jj_3R_6() {
    if (jj_3R_13()) return true;
    Token xsp;
    if (jj_3R_14()) return true;
    while (true) {
      xsp = jj_scanpos;
      if (jj_3R_14()) { jj_scanpos = xsp; break; }
    }
    return false;
  }

  static final private boolean jj_3R_27() {
    if (jj_3R_19()) return true;
    return false;
  }

  static final private boolean jj_3R_19() {
    if (jj_scan_token(WORD)) return true;
    return false;
  }

  static final private boolean jj_3R_11() {
    if (jj_3R_19()) return true;
    return false;
  }

  static final private boolean jj_3R_24() {
    if (jj_3R_16()) return true;
    return false;
  }

  static final private boolean jj_3R_28() {
    if (jj_scan_token(WORD)) return true;
    return false;
  }

  static final private boolean jj_3R_10() {
    if (jj_3R_18()) return true;
    return false;
  }

  static final private boolean jj_3R_23() {
    if (jj_3R_5()) return true;
    return false;
  }

  static final private boolean jj_3R_9() {
    if (jj_3R_17()) return true;
    return false;
  }

  static final private boolean jj_3R_22() {
    if (jj_scan_token(NEG)) return true;
    if (jj_scan_token(LBRACKET)) return true;
    return false;
  }

  static final private boolean jj_3R_8() {
    if (jj_3R_16()) return true;
    return false;
  }

  static final private boolean jj_3_5() {
    if (jj_3R_7()) return true;
    return false;
  }

  static final private boolean jj_3_4() {
    if (jj_3R_6()) return true;
    return false;
  }

  static final private boolean jj_3R_20() {
    if (jj_scan_token(QUOTE)) return true;
    Token xsp;
    if (jj_3R_28()) return true;
    while (true) {
      xsp = jj_scanpos;
      if (jj_3R_28()) { jj_scanpos = xsp; break; }
    }
    return false;
  }

  static final private boolean jj_3R_21() {
    if (jj_scan_token(LBRACKET)) return true;
    if (jj_3R_4()) return true;
    return false;
  }

  static final private boolean jj_3_3() {
    if (jj_3R_5()) return true;
    return false;
  }

  static final private boolean jj_3R_13() {
    Token xsp;
    xsp = jj_scanpos;
    if (jj_3R_21()) {
    jj_scanpos = xsp;
    if (jj_3R_22()) {
    jj_scanpos = xsp;
    if (jj_3R_23()) {
    jj_scanpos = xsp;
    if (jj_3R_24()) {
    jj_scanpos = xsp;
    if (jj_3R_25()) {
    jj_scanpos = xsp;
    if (jj_3R_26()) {
    jj_scanpos = xsp;
    if (jj_3R_27()) return true;
    }
    }
    }
    }
    }
    }
    return false;
  }

  static final private boolean jj_3R_17() {
    if (jj_scan_token(FLOAT)) return true;
    return false;
  }

  static final private boolean jj_3_2() {
    if (jj_scan_token(NEG)) return true;
    if (jj_scan_token(LBRACKET)) return true;
    if (jj_3R_4()) return true;
    return false;
  }

  static final private boolean jj_3_1() {
    if (jj_scan_token(LBRACKET)) return true;
    if (jj_3R_4()) return true;
    if (jj_scan_token(RBRACKET)) return true;
    return false;
  }

  static final private boolean jj_3R_4() {
    Token xsp;
    xsp = jj_scanpos;
    if (jj_3_1()) {
    jj_scanpos = xsp;
    if (jj_3_2()) {
    jj_scanpos = xsp;
    if (jj_3_3()) {
    jj_scanpos = xsp;
    if (jj_3_4()) {
    jj_scanpos = xsp;
    if (jj_3_5()) {
    jj_scanpos = xsp;
    if (jj_3R_8()) {
    jj_scanpos = xsp;
    if (jj_3R_9()) {
    jj_scanpos = xsp;
    if (jj_3R_10()) {
    jj_scanpos = xsp;
    if (jj_3R_11()) {
    jj_scanpos = xsp;
    if (jj_3R_12()) return true;
    }
    }
    }
    }
    }
    }
    }
    }
    }
    return false;
  }

  static private boolean jj_initialized_once = false;
  static public FormulaParserTokenManager token_source;
  static SimpleCharStream jj_input_stream;
  static public Token token, jj_nt;
  static private int jj_ntk;
  static private Token jj_scanpos, jj_lastpos;
  static private int jj_la;
  static public boolean lookingAhead = false;
  static private boolean jj_semLA;
  static private int jj_gen;
  static final private int[] jj_la1 = new int[5];
  static private int[] jj_la1_0;
  static {
      jj_la1_0();
   }
   private static void jj_la1_0() {
      jj_la1_0 = new int[] {0x10000,0x2000,0x4000,0x11ea0,0x19c00,};
   }
  static final private JJCalls[] jj_2_rtns = new JJCalls[5];
  static private boolean jj_rescan = false;
  static private int jj_gc = 0;

  public FormulaParser(java.io.InputStream stream) {
     this(stream, null);
  }
  public FormulaParser(java.io.InputStream stream, String encoding) {
    if (jj_initialized_once) {
      System.out.println("ERROR: Second call to constructor of static parser.  You must");
      System.out.println("       either use ReInit() or set the JavaCC option STATIC to false");
      System.out.println("       during parser generation.");
      throw new Error();
    }
    jj_initialized_once = true;
    try { jj_input_stream = new SimpleCharStream(stream, encoding, 1, 1); } catch(java.io.UnsupportedEncodingException e) { throw new RuntimeException(e); }
    token_source = new FormulaParserTokenManager(jj_input_stream);
    token = new Token();
    jj_ntk = -1;
    jj_gen = 0;
    for (int i = 0; i < 5; i++) jj_la1[i] = -1;
    for (int i = 0; i < jj_2_rtns.length; i++) jj_2_rtns[i] = new JJCalls();
  }

  static public void ReInit(java.io.InputStream stream) {
     ReInit(stream, null);
  }
  static public void ReInit(java.io.InputStream stream, String encoding) {
    try { jj_input_stream.ReInit(stream, encoding, 1, 1); } catch(java.io.UnsupportedEncodingException e) { throw new RuntimeException(e); }
    token_source.ReInit(jj_input_stream);
    token = new Token();
    jj_ntk = -1;
    jj_gen = 0;
    for (int i = 0; i < 5; i++) jj_la1[i] = -1;
    for (int i = 0; i < jj_2_rtns.length; i++) jj_2_rtns[i] = new JJCalls();
  }

  public FormulaParser(java.io.Reader stream) {
    if (jj_initialized_once) {
      System.out.println("ERROR: Second call to constructor of static parser.  You must");
      System.out.println("       either use ReInit() or set the JavaCC option STATIC to false");
      System.out.println("       during parser generation.");
      throw new Error();
    }
    jj_initialized_once = true;
    jj_input_stream = new SimpleCharStream(stream, 1, 1);
    token_source = new FormulaParserTokenManager(jj_input_stream);
    token = new Token();
    jj_ntk = -1;
    jj_gen = 0;
    for (int i = 0; i < 5; i++) jj_la1[i] = -1;
    for (int i = 0; i < jj_2_rtns.length; i++) jj_2_rtns[i] = new JJCalls();
  }

  static public void ReInit(java.io.Reader stream) {
    jj_input_stream.ReInit(stream, 1, 1);
    token_source.ReInit(jj_input_stream);
    token = new Token();
    jj_ntk = -1;
    jj_gen = 0;
    for (int i = 0; i < 5; i++) jj_la1[i] = -1;
    for (int i = 0; i < jj_2_rtns.length; i++) jj_2_rtns[i] = new JJCalls();
  }

  public FormulaParser(FormulaParserTokenManager tm) {
    if (jj_initialized_once) {
      System.out.println("ERROR: Second call to constructor of static parser.  You must");
      System.out.println("       either use ReInit() or set the JavaCC option STATIC to false");
      System.out.println("       during parser generation.");
      throw new Error();
    }
    jj_initialized_once = true;
    token_source = tm;
    token = new Token();
    jj_ntk = -1;
    jj_gen = 0;
    for (int i = 0; i < 5; i++) jj_la1[i] = -1;
    for (int i = 0; i < jj_2_rtns.length; i++) jj_2_rtns[i] = new JJCalls();
  }

  public void ReInit(FormulaParserTokenManager tm) {
    token_source = tm;
    token = new Token();
    jj_ntk = -1;
    jj_gen = 0;
    for (int i = 0; i < 5; i++) jj_la1[i] = -1;
    for (int i = 0; i < jj_2_rtns.length; i++) jj_2_rtns[i] = new JJCalls();
  }

  static final private Token jj_consume_token(int kind) throws ParseException {
    Token oldToken;
    if ((oldToken = token).next != null) token = token.next;
    else token = token.next = token_source.getNextToken();
    jj_ntk = -1;
    if (token.kind == kind) {
      jj_gen++;
      if (++jj_gc > 100) {
        jj_gc = 0;
        for (int i = 0; i < jj_2_rtns.length; i++) {
          JJCalls c = jj_2_rtns[i];
          while (c != null) {
            if (c.gen < jj_gen) c.first = null;
            c = c.next;
          }
        }
      }
      return token;
    }
    token = oldToken;
    jj_kind = kind;
    throw generateParseException();
  }

  static private final class LookaheadSuccess extends java.lang.Error { }
  static final private LookaheadSuccess jj_ls = new LookaheadSuccess();
  static final private boolean jj_scan_token(int kind) {
    if (jj_scanpos == jj_lastpos) {
      jj_la--;
      if (jj_scanpos.next == null) {
        jj_lastpos = jj_scanpos = jj_scanpos.next = token_source.getNextToken();
      } else {
        jj_lastpos = jj_scanpos = jj_scanpos.next;
      }
    } else {
      jj_scanpos = jj_scanpos.next;
    }
    if (jj_rescan) {
      int i = 0; Token tok = token;
      while (tok != null && tok != jj_scanpos) { i++; tok = tok.next; }
      if (tok != null) jj_add_error_token(kind, i);
    }
    if (jj_scanpos.kind != kind) return true;
    if (jj_la == 0 && jj_scanpos == jj_lastpos) throw jj_ls;
    return false;
  }

  static final public Token getNextToken() {
    if (token.next != null) token = token.next;
    else token = token.next = token_source.getNextToken();
    jj_ntk = -1;
    jj_gen++;
    return token;
  }

  static final public Token getToken(int index) {
    Token t = lookingAhead ? jj_scanpos : token;
    for (int i = 0; i < index; i++) {
      if (t.next != null) t = t.next;
      else t = t.next = token_source.getNextToken();
    }
    return t;
  }

  static final private int jj_ntk() {
    if ((jj_nt=token.next) == null)
      return (jj_ntk = (token.next=token_source.getNextToken()).kind);
    else
      return (jj_ntk = jj_nt.kind);
  }

  static private java.util.Vector jj_expentries = new java.util.Vector();
  static private int[] jj_expentry;
  static private int jj_kind = -1;
  static private int[] jj_lasttokens = new int[100];
  static private int jj_endpos;

  static private void jj_add_error_token(int kind, int pos) {
    if (pos >= 100) return;
    if (pos == jj_endpos + 1) {
      jj_lasttokens[jj_endpos++] = kind;
    } else if (jj_endpos != 0) {
      jj_expentry = new int[jj_endpos];
      for (int i = 0; i < jj_endpos; i++) {
        jj_expentry[i] = jj_lasttokens[i];
      }
      boolean exists = false;
      for (java.util.Enumeration e = jj_expentries.elements(); e.hasMoreElements();) {
        int[] oldentry = (int[])(e.nextElement());
        if (oldentry.length == jj_expentry.length) {
          exists = true;
          for (int i = 0; i < jj_expentry.length; i++) {
            if (oldentry[i] != jj_expentry[i]) {
              exists = false;
              break;
            }
          }
          if (exists) break;
        }
      }
      if (!exists) jj_expentries.addElement(jj_expentry);
      if (pos != 0) jj_lasttokens[(jj_endpos = pos) - 1] = kind;
    }
  }

  static public ParseException generateParseException() {
    jj_expentries.removeAllElements();
    boolean[] la1tokens = new boolean[17];
    for (int i = 0; i < 17; i++) {
      la1tokens[i] = false;
    }
    if (jj_kind >= 0) {
      la1tokens[jj_kind] = true;
      jj_kind = -1;
    }
    for (int i = 0; i < 5; i++) {
      if (jj_la1[i] == jj_gen) {
        for (int j = 0; j < 32; j++) {
          if ((jj_la1_0[i] & (1<<j)) != 0) {
            la1tokens[j] = true;
          }
        }
      }
    }
    for (int i = 0; i < 17; i++) {
      if (la1tokens[i]) {
        jj_expentry = new int[1];
        jj_expentry[0] = i;
        jj_expentries.addElement(jj_expentry);
      }
    }
    jj_endpos = 0;
    jj_rescan_token();
    jj_add_error_token(0, 0);
    int[][] exptokseq = new int[jj_expentries.size()][];
    for (int i = 0; i < jj_expentries.size(); i++) {
      exptokseq[i] = (int[])jj_expentries.elementAt(i);
    }
    return new ParseException(token, exptokseq, tokenImage);
  }

  static final public void enable_tracing() {
  }

  static final public void disable_tracing() {
  }

  static final private void jj_rescan_token() {
    jj_rescan = true;
    for (int i = 0; i < 5; i++) {
    try {
      JJCalls p = jj_2_rtns[i];
      do {
        if (p.gen > jj_gen) {
          jj_la = p.arg; jj_lastpos = jj_scanpos = p.first;
          switch (i) {
            case 0: jj_3_1(); break;
            case 1: jj_3_2(); break;
            case 2: jj_3_3(); break;
            case 3: jj_3_4(); break;
            case 4: jj_3_5(); break;
          }
        }
        p = p.next;
      } while (p != null);
      } catch(LookaheadSuccess ls) { }
    }
    jj_rescan = false;
  }

  static final private void jj_save(int index, int xla) {
    JJCalls p = jj_2_rtns[index];
    while (p.gen > jj_gen) {
      if (p.next == null) { p = p.next = new JJCalls(); break; }
      p = p.next;
    }
    p.gen = jj_gen + xla - jj_la; p.first = token; p.arg = xla;
  }

  static final class JJCalls {
    int gen;
    Token first;
    int arg;
    JJCalls next;
  }

}
