#include <iostream>
#include <Dependences.hpp>
#include <GeneralUtilityClass.hpp>

using namespace std;

Dependences::Dependences() {
  WAW       = NULL;
  live_in   = NULL;
  flow_dep  = NULL;
  writeMaps = NULL;
}

isl_union_map* Dependences::getFlowDependences() {
  return flow_dep;
}

isl_union_map* Dependences::getLiveIn() {
  return live_in;
}

void Dependences::setFlowDependences(isl_union_map* flow)
{
  flow_dep = flow;
}

void Dependences::setLiveIn(isl_union_map* li) {
  live_in = li;
}

isl_union_map* Dependences::getWriteMaps() {
  return writeMaps;
}

void Dependences::setWriteMaps(isl_union_map* wm) {
  writeMaps = wm;
}

isl_union_map* Dependences::getWAW() {
  return WAW;
}

void Dependences::setWAW(isl_union_map* inWAW) {
  WAW = inWAW;
}

void Dependences::display() {
  cout << "Flow Dependences: " << endl;
  GeneralUtilityClass::PrintUnionMap (flow_dep);
  GeneralUtilityClass::PrintUnionMap (isl_union_map_reverse(isl_union_map_copy(flow_dep)));
  cout << endl;

  cout << "Output dependences: " << endl;
  GeneralUtilityClass::PrintUnionMap (WAW);
  GeneralUtilityClass::PrintUnionMap (isl_union_map_reverse(isl_union_map_copy(WAW)));
  cout << endl;

  cout << "Live-in maps: " << endl;
  GeneralUtilityClass::PrintUnionMap (live_in);
  cout << endl;
}
