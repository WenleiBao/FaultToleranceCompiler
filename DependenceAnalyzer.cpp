#include <assert.h>
#include <isl/flow.h>
#include <DependenceAnalyzer.hpp>
#include <GeneralUtilityClass.hpp>

Dependences* DependenceAnalyzer::ComputeDependencies (vector<Statement*>  &statements) {
  Dependences *deps = new Dependences();

  isl_union_map *sink        = NULL; 
  isl_union_map *schedule    = NULL; 
  isl_union_map *may_source  = NULL;
  isl_union_map *must_source = NULL; 

  for (int i = 0; i < statements.size(); i++) {
    if (statements[i] == NULL) {
	    cout << "Statement is NULL" << endl;
	    assert(0);
	  }

    if (statements[i]->getWriteReferenceMap() == NULL) {
	    cout << "Write map is null" << endl;
	    assert (0);
	  }

	  isl_union_map *tempMap = isl_union_map_from_map(isl_map_copy(statements[i]->getWriteReferenceMap()));

	  if (must_source == NULL){
	    must_source = tempMap;
    } else {
	    must_source = isl_union_map_union (tempMap, must_source);
	  }

	  vector<isl_map*> read_refs = statements[i]->getReadReferenceMaps();
    for (int j = 0; j < read_refs.size(); j++) {
	    tempMap = isl_union_map_from_map(isl_map_copy(read_refs[j]));

	    if (sink == NULL) {
		    sink = tempMap;
	    } else {
		    sink = isl_union_map_union (tempMap, sink);
	    } 
	  }

	  tempMap = isl_union_map_from_map(isl_map_copy(statements[i]->getScheduleMap()));

	  if (schedule == NULL) {
	     schedule = tempMap;
	  } else {
	     schedule = isl_union_map_union (tempMap, schedule);
	  }
  }

  isl_union_map *must_dep = NULL, *may_dep = NULL, *must_no_source = NULL, *may_no_source = NULL;
  may_source = isl_union_map_empty (isl_union_map_get_space(must_source));

  int returnValue = isl_union_map_compute_flow(
                        sink, 
                        isl_union_map_copy(must_source), 
                        isl_union_map_copy(may_source), 
                        isl_union_map_copy(schedule), 
                        &must_dep, 
                        &may_dep, 
                        &must_no_source, 
                        &may_no_source);

  deps->setWriteMaps (must_source);

  if (must_dep != NULL) {
	  deps->setFlowDependences (must_dep);
  } else {
    cout << "must_dep is NULL" << endl;
  }

  if (must_no_source != NULL) {
	  deps->setLiveIn (must_no_source);
  } else {
    cout << "must_no_source is NULL" << endl;
  }

  must_dep = NULL;
  may_dep = NULL;
  must_no_source = NULL;
  may_no_source = NULL;

  returnValue = isl_union_map_compute_flow(
                    isl_union_map_copy(must_source), 
                    isl_union_map_copy(must_source), 
                    isl_union_map_copy(may_source), 
                    isl_union_map_copy(schedule), 
                    &must_dep, 
                    &may_dep, 
                    &must_no_source, 
                    &may_no_source);

  if (must_dep != NULL) {
	  deps->setWAW (must_dep);
  } else {
    cout << "WAW is NULL" << endl;
  }

  return deps;
}
