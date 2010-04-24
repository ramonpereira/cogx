#include "lazy_best_first_search_engine.h"
#include "open-lists/standard_scalar_open_list.h"
#include "open-lists/alternation_open_list.h"
#include "open-lists/tiebreaking_open_list.h"
#include "scalar_evaluator.h"
#include "heuristic.h"
#include <vector>

LazyBestFirstSearchEngine::LazyBestFirstSearchEngine():
    GeneralLazyBestFirstSearch(false){
    // TODO Auto-generated constructor stub

}

LazyBestFirstSearchEngine::~LazyBestFirstSearchEngine() {
    // TODO Auto-generated destructor stub
}


void LazyBestFirstSearchEngine::initialize() {
    //TODO children classes should output which kind of search
    cout << "Conducting lazy greedy best first search" << endl;

    assert(heuristics.size() > 0);

    if (heuristics.size() + preferred_operator_heuristics.size() == 1) {
        open_list = new StandardScalarOpenList<OpenListEntryLazy>(heuristics[0]);
    }
    else {
        vector<OpenList<OpenListEntryLazy>*> inner_lists;
        for (int i = 0; i < heuristics.size(); i++) {
            inner_lists.push_back(new StandardScalarOpenList<OpenListEntryLazy>(heuristics[i], false));
        }
        for (int i = 0; i < preferred_operator_heuristics.size(); i++) {
            inner_lists.push_back(new StandardScalarOpenList<OpenListEntryLazy>(preferred_operator_heuristics[i], true));
        }
        open_list = new AlternationOpenList<OpenListEntryLazy>(inner_lists);
    }

    GeneralLazyBestFirstSearch::initialize();
}
