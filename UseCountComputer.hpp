#ifndef USECOUNTCOMPUTER_HPP
#define USECOUNTCOMPUTER_HPP

#include <string>
#include <vector>
#include <map>

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

class UseCountComputer {
  public:
    static isl_set* ParameterizeAllDimensions (isl_set*);
    static isl_union_set* ParameterizeAllDimensions (isl_union_set*);
    static isl_union_set *ApplyUnionMapOnUnionSet(isl_union_set *set, isl_union_map* map, int remake, int print = 1, vector<isl_union_pw_qpolynomial*> *live_in_use_count = NULL);

    static void DetermineUnusedDefinitions(vector<Statement*> statements, Dependences* dependences);
    static void ComputeFlowDependenceDefChecksumContribution(vector<Statement*> statements, Dependences* dependences, map<string,int> &ArraySizeMap);
};

#endif
