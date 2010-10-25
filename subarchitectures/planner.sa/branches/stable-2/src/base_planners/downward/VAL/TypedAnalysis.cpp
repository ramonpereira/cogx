/*
  main() for the PDDL2.1 Analysis tester

  $Date: 2001/08/02 16:44:19 $
  $Revision: 3.2 $

  This expects any number of filenames as arguments, although
  it probably doesn't ever make sense to supply more than two.

  derek.long@cis.strath.ac.uk

  Strathclyde Planning Group
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include "ptree.h"
#include <FlexLexer.h>
#include "TypedAnalyser.h"


extern int yyparse();
extern int yydebug;

using std::ifstream;
using std::ofstream;

namespace VAL {

bool Verbose = false;
ostream * report = &cout;
parse_category* top_thing=NULL;

analysis an_analysis;
analysis* current_analysis;

yyFlexLexer* yfl;

TypeChecker * theTC;

int PropInfo::x = 0;

};

char * current_filename;
using namespace VAL;


int main(int argc,char * argv[])
{
    current_analysis= &an_analysis;
    an_analysis.pred_tab.replaceFactory<holding_pred_symbol>();
    an_analysis.func_tab.replaceFactory<extended_func_symbol>();
    
    ifstream* current_in_stream;
    yydebug=0; // Set to 1 to output yacc trace 

    yfl= new yyFlexLexer;

    // Loop over given args
    for (int a=1; a<argc; ++a)
    {
	current_filename= argv[a];
	cout << "File: " << current_filename << '\n';
	current_in_stream= new ifstream(current_filename);
	if (current_in_stream->bad())
	{
	    // Output a message now
	    cout << "Failed to open\n";
	    
	    // Log an error to be reported in summary later
	    line_no= 0;
	    log_error(E_FATAL,"Failed to open file");
	}
	else
	{
	    line_no= 1;

	    // Switch the tokeniser to the current input stream
	    yfl->switch_streams(current_in_stream,&cout);
	    yyparse();

	    // Output syntax tree
	    //if (top_thing) top_thing->display(0);
	}
	delete current_in_stream;
    }
    // Output the errors from all input files
    current_analysis->error_list.report();
    delete yfl;

    TypeChecker tc(current_analysis);
    theTC = &tc;
    
    TypePredSubstituter a;
    current_analysis->the_problem->visit(&a);
   	current_analysis->the_domain->visit(&a); 
   	Analyser aa;
   	current_analysis->the_problem->visit(&aa);
   	current_analysis->the_domain->visit(&aa);
    current_analysis->the_domain->predicates->visit(&aa);
    if(current_analysis->the_domain->functions) current_analysis->the_domain->functions->visit(&aa);
}
