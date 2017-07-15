#include <assert.h>
#include <barvinok/isl.h>
#include <LiveDataComputer.hpp>
#include <GeneralUtilityClass.hpp>

void LiveDataComputer::ComputeLiveInDataAndUseCounts(
                        vector<Statement*> statements, 
                        Dependences* dependences, 
                        vector<string*> **LiveDataDefChecksumCode, 
                        map<string,int> &ArraySizeMap) {

  *LiveDataDefChecksumCode = new vector<string*>();

  isl_union_set* live_in_data = NULL;
  for (int i = 0; i < statements.size(); i++) {
    isl_union_set *localLiveInData = GeneralUtilityClass::ApplyUnionMapOnSet(statements[i]->getDomainSet(), dependences->getLiveIn());

    if (live_in_data == NULL) {
      live_in_data = localLiveInData;
    } else {
      live_in_data = isl_union_set_union (isl_union_set_copy(live_in_data), isl_union_set_copy(localLiveInData));
    }
  }

  isl_union_map *reverseLiveInMap = isl_union_map_reverse (isl_union_map_copy(dependences->getLiveIn()));
  GeneralUtilityClass::DataSetPrintUnionSetCode(live_in_data, 1, reverseLiveInMap, *LiveDataDefChecksumCode, ArraySizeMap);
}
