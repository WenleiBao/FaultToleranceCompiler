#ifndef LIVEDATACOMPUTER_HPP
#define LIVEDATACOMPUTER_HPP

#include <map>
#include <string>
#include <vector>

#include <isl/ast_build.h>
#include <isl/ast.h>
#include <isl/schedule.h>
#include <isl/set.h>
#include <isl/union_map.h>
#include <isl/ctx.h>
#include <isl/map.h>
#include <isl/flow.h>

#include <Statement.hpp>
#include <Dependences.hpp>
using namespace std;

class LiveDataComputer {
  public:
    static string ExtractEqualities(isl_set* set);
    static isl_set* ParameterizeAllDimensions (isl_set *domainSet, vector<string*> *MissingIterators);
    static void ComputeLiveInDataAndUseCounts(vector<Statement*> statements, Dependences* dependences, vector<string*> **LiveDataDefChecksumCode, map<string,int> &ArraySizeMap);
}; 

#endif
