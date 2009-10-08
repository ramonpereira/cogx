/* Generated By:JavaCC: Do not edit this line. BNConfigParser.java */
/** New line translator. */

package binder.bayesiannetwork.configparser;

import binder.autogen.core.*;
import binder.autogen.featvalues.*;
import binder.autogen.bayesiannetworks.*;
import java.util.Vector;
import java.util.Enumeration;

import cast.cdl.CASTTime;


// TODO: bayesian network for non-string values

public class BNConfigParser implements BNConfigParserConstants {

  /** Main entry point. */
  public static void main(String args[]) throws ParseException {
    BNConfigParser parser = new BNConfigParser(System.in);
    BayesianNetwork network = parser.Configuration();

    log("number of nodes in BN: " + network.nodes.length);
    for (int i = 0; i < network.nodes.length; i++) {
        log("nb of feat values: " + network.nodes[i].feat.alternativeValues.length);
    }

  }


public static BayesianNetworkNode getNode(String s, BayesianNetworkNode[] nodes) {
        for (int i = 0; i< nodes.length; i++) {
                BayesianNetworkNode node = nodes[i];
                if (node.feat.featlabel.equals(s)) {
                        return node;
                }
        }
        return null;
}

public static void log(String s) {
        System.out.println("[BayesianNetworkConfigParser] " + s);
}

  static final public Vector<FeatureValue> FeatureValues() throws ParseException {
Vector<FeatureValue> features = new Vector<FeatureValue>();
Token featvalue;
    featvalue = jj_consume_token(ID);
                         features.add(new StringValue(0, new CASTTime(), featvalue.image));
    label_1:
    while (true) {
      switch ((jj_ntk==-1)?jj_ntk():jj_ntk) {
      case 14:
        ;
        break;
      default:
        jj_la1[0] = jj_gen;
        break label_1;
      }
      jj_consume_token(14);
      featvalue = jj_consume_token(ID);
                             features.add(new StringValue(0, new CASTTime(), featvalue.image));
    }
{if (true) return features;}
    throw new Error("Missing return statement in function");
  }

  static final public FeatureValue FeatureProb() throws ParseException {
Token tok1;
Token tok2;
Token tok3;
    jj_consume_token(15);
    tok1 = jj_consume_token(ID);
    jj_consume_token(16);
    tok2 = jj_consume_token(ID);
    jj_consume_token(17);
    jj_consume_token(16);
    tok3 = jj_consume_token(PROB);
   FeatureValue featvalue = new StringValue(0, new CASTTime(), tok2.image);
  featvalue.independentProb = Float.parseFloat (tok3.image);
 // System.out.println( featvalue.independentProb);
 {if (true) return featvalue;}
    throw new Error("Missing return statement in function");
  }

  static final public Vector<FeatureValue> FeatureProbs() throws ParseException {
  Vector<FeatureValue> features = new Vector<FeatureValue>();
FeatureValue featvalue;
    featvalue = FeatureProb();
                                 features.add(featvalue);
    label_2:
    while (true) {
      switch ((jj_ntk==-1)?jj_ntk():jj_ntk) {
      case 14:
        ;
        break;
      default:
        jj_la1[1] = jj_gen;
        break label_2;
      }
      jj_consume_token(14);
      featvalue = FeatureProb();
                                                                                            features.add(featvalue);
    }
{if (true) return features;}
    throw new Error("Missing return statement in function");
  }

  static final public BayesianNetworkNode Node() throws ParseException {
BayesianNetworkNode node = new BayesianNetworkNode();
node.feat = new Feature();
Token label;
Vector<FeatureValue> featuresVector1;
Vector<FeatureValue> featuresVector = new Vector<FeatureValue>();
    label = jj_consume_token(ID);
                     node.feat.featlabel = label.image;
    jj_consume_token(LBRACE);
    jj_consume_token(FEATVALUESID);
    jj_consume_token(LBRACE);
    featuresVector1 = FeatureValues();
    jj_consume_token(RBRACE);
    switch ((jj_ntk==-1)?jj_ntk():jj_ntk) {
    case FEATPROBSID:
      jj_consume_token(FEATPROBSID);
      jj_consume_token(LBRACE);
      featuresVector = FeatureProbs();
      jj_consume_token(RBRACE);
      break;
    default:
      jj_la1[2] = jj_gen;
      ;
    }
    jj_consume_token(RBRACE);
        for (Enumeration<FeatureValue> e = featuresVector1.elements() ; e.hasMoreElements(); ) {
                FeatureValue v = e.nextElement();
                boolean existsAlready = false;
                for (Enumeration<FeatureValue> f = featuresVector.elements() ; f.hasMoreElements(); ) {
                        if (((StringValue)f.nextElement()).val.equals(((StringValue)v).val)) {
                                existsAlready = true;
                        }
                }
                if (!existsAlready) {
                        featuresVector.add(v);
                }
        }
         node.feat.alternativeValues = new FeatureValue[featuresVector.size()];
for (int i = 0; i <  node.feat.alternativeValues.length ; i++) {
         node.feat.alternativeValues[i] = featuresVector.elementAt(i);
        }
        {if (true) return node;}
    throw new Error("Missing return statement in function");
  }

  static final public FeatureValueCorrelation ConditionalProb() throws ParseException {
FeatureValueCorrelation corr = new FeatureValueCorrelation();
Token tok1;
Token tok2;
Token tok3;
    jj_consume_token(15);
    jj_consume_token(ID);
    jj_consume_token(16);
    tok1 = jj_consume_token(ID);
    jj_consume_token(18);
    jj_consume_token(ID);
    jj_consume_token(16);
    tok2 = jj_consume_token(ID);
    jj_consume_token(17);
    jj_consume_token(16);
    tok3 = jj_consume_token(PROB);
  corr.value1 = new StringValue(0, new CASTTime(), tok1.image);
 corr.value2 = new StringValue(0, new CASTTime(), tok2.image);
  corr.condProb = Float.parseFloat(tok3.image);
 {if (true) return corr;}
    throw new Error("Missing return statement in function");
  }

  static final public Vector<FeatureValueCorrelation> ConditionalProbs() throws ParseException {
Vector<FeatureValueCorrelation> correlations = new Vector<FeatureValueCorrelation>();
FeatureValueCorrelation prob;
    prob = ConditionalProb();
                                correlations.add(prob);
    label_3:
    while (true) {
      switch ((jj_ntk==-1)?jj_ntk():jj_ntk) {
      case 14:
        ;
        break;
      default:
        jj_la1[3] = jj_gen;
        break label_3;
      }
      jj_consume_token(14);
      prob = ConditionalProb();
                                                                                       correlations.add(prob);
    }
         {if (true) return correlations;}
    throw new Error("Missing return statement in function");
  }
 
  static final public BayesianNetworkEdge Edge(BayesianNetworkNode[] nodes) throws ParseException {
 Token tok1;
Token tok2;
BayesianNetworkEdge edge = new BayesianNetworkEdge();
Vector<FeatureValueCorrelation> correlationsVector;
    tok1 = jj_consume_token(ID);
                    edge.incomingNode = getNode(tok1.image,nodes);
    jj_consume_token(19);
    tok2 = jj_consume_token(ID);
                                                                                       edge.outgoingNode = getNode(tok2.image,nodes);
    jj_consume_token(LBRACE);
    correlationsVector = ConditionalProbs();
    jj_consume_token(RBRACE);
edge.correlations = new FeatureValueCorrelation[correlationsVector.size()];
for (int i = 0; i < edge.correlations.length; i++) {
        edge.correlations[i] = correlationsVector.elementAt(i);
}
{if (true) return edge;}
    throw new Error("Missing return statement in function");
  }

  static final public BayesianNetworkNode[] Nodes() throws ParseException {
Vector<BayesianNetworkNode> nodesVector = new Vector<BayesianNetworkNode>();
BayesianNetworkNode n;
    label_4:
    while (true) {
      switch ((jj_ntk==-1)?jj_ntk():jj_ntk) {
      case ID:
        ;
        break;
      default:
        jj_la1[4] = jj_gen;
        break label_4;
      }
      n = Node();
            nodesVector.add(n);
    }
BayesianNetworkNode[] nodes = new BayesianNetworkNode[nodesVector.size()];
for (int i = 0; i < nodes.length ; i++) {
        nodes[i] = nodesVector.elementAt(i);
}
{if (true) return nodes;}
    throw new Error("Missing return statement in function");
  }

  static final public BayesianNetworkEdge[] Edges(BayesianNetworkNode[] nodes) throws ParseException {
Vector<BayesianNetworkEdge> edgesVector = new Vector<BayesianNetworkEdge>();
BayesianNetworkEdge e;
    label_5:
    while (true) {
      switch ((jj_ntk==-1)?jj_ntk():jj_ntk) {
      case ID:
        ;
        break;
      default:
        jj_la1[5] = jj_gen;
        break label_5;
      }
      e = Edge(nodes);
                 edgesVector.add(e);
    }
BayesianNetworkEdge[] edges = new BayesianNetworkEdge[edgesVector.size()];
for (int i = 0; i < edges.length ; i++) {
        edges[i] = edgesVector.elementAt(i);
}
{if (true) return edges;}
    throw new Error("Missing return statement in function");
  }

/** Top level production. */
  static final public BayesianNetwork Configuration() throws ParseException {
        BayesianNetwork network = new BayesianNetwork();
    jj_consume_token(NODEID);
    jj_consume_token(LBRACE);
    network.nodes = Nodes();
    jj_consume_token(RBRACE);
    jj_consume_token(EDGEID);
    jj_consume_token(LBRACE);
    network.edges = Edges(network.nodes);
    jj_consume_token(RBRACE);
    jj_consume_token(0);
         {if (true) return network;}
    throw new Error("Missing return statement in function");
  }

  static private boolean jj_initialized_once = false;
  static public BNConfigParserTokenManager token_source;
  static SimpleCharStream jj_input_stream;
  static public Token token, jj_nt;
  static private int jj_ntk;
  static private int jj_gen;
  static final private int[] jj_la1 = new int[6];
  static private int[] jj_la1_0;
  static {
      jj_la1_0();
   }
   private static void jj_la1_0() {
      jj_la1_0 = new int[] {0x4000,0x4000,0x100,0x4000,0x200,0x200,};
   }

  public BNConfigParser(java.io.InputStream stream) {
     this(stream, null);
  }
  public BNConfigParser(java.io.InputStream stream, String encoding) {
    if (jj_initialized_once) {
      System.out.println("ERROR: Second call to constructor of static parser.  You must");
      System.out.println("       either use ReInit() or set the JavaCC option STATIC to false");
      System.out.println("       during parser generation.");
      throw new Error();
    }
    jj_initialized_once = true;
    try { jj_input_stream = new SimpleCharStream(stream, encoding, 1, 1); } catch(java.io.UnsupportedEncodingException e) { throw new RuntimeException(e); }
    token_source = new BNConfigParserTokenManager(jj_input_stream);
    token = new Token();
    jj_ntk = -1;
    jj_gen = 0;
    for (int i = 0; i < 6; i++) jj_la1[i] = -1;
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
    for (int i = 0; i < 6; i++) jj_la1[i] = -1;
  }

  public BNConfigParser(java.io.Reader stream) {
    if (jj_initialized_once) {
      System.out.println("ERROR: Second call to constructor of static parser.  You must");
      System.out.println("       either use ReInit() or set the JavaCC option STATIC to false");
      System.out.println("       during parser generation.");
      throw new Error();
    }
    jj_initialized_once = true;
    jj_input_stream = new SimpleCharStream(stream, 1, 1);
    token_source = new BNConfigParserTokenManager(jj_input_stream);
    token = new Token();
    jj_ntk = -1;
    jj_gen = 0;
    for (int i = 0; i < 6; i++) jj_la1[i] = -1;
  }

  static public void ReInit(java.io.Reader stream) {
    jj_input_stream.ReInit(stream, 1, 1);
    token_source.ReInit(jj_input_stream);
    token = new Token();
    jj_ntk = -1;
    jj_gen = 0;
    for (int i = 0; i < 6; i++) jj_la1[i] = -1;
  }

  public BNConfigParser(BNConfigParserTokenManager tm) {
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
    for (int i = 0; i < 6; i++) jj_la1[i] = -1;
  }

  public void ReInit(BNConfigParserTokenManager tm) {
    token_source = tm;
    token = new Token();
    jj_ntk = -1;
    jj_gen = 0;
    for (int i = 0; i < 6; i++) jj_la1[i] = -1;
  }

  static final private Token jj_consume_token(int kind) throws ParseException {
    Token oldToken;
    if ((oldToken = token).next != null) token = token.next;
    else token = token.next = token_source.getNextToken();
    jj_ntk = -1;
    if (token.kind == kind) {
      jj_gen++;
      return token;
    }
    token = oldToken;
    jj_kind = kind;
    throw generateParseException();
  }

  static final public Token getNextToken() {
    if (token.next != null) token = token.next;
    else token = token.next = token_source.getNextToken();
    jj_ntk = -1;
    jj_gen++;
    return token;
  }

  static final public Token getToken(int index) {
    Token t = token;
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

  static public ParseException generateParseException() {
    jj_expentries.removeAllElements();
    boolean[] la1tokens = new boolean[20];
    for (int i = 0; i < 20; i++) {
      la1tokens[i] = false;
    }
    if (jj_kind >= 0) {
      la1tokens[jj_kind] = true;
      jj_kind = -1;
    }
    for (int i = 0; i < 6; i++) {
      if (jj_la1[i] == jj_gen) {
        for (int j = 0; j < 32; j++) {
          if ((jj_la1_0[i] & (1<<j)) != 0) {
            la1tokens[j] = true;
          }
        }
      }
    }
    for (int i = 0; i < 20; i++) {
      if (la1tokens[i]) {
        jj_expentry = new int[1];
        jj_expentry[0] = i;
        jj_expentries.addElement(jj_expentry);
      }
    }
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

}
