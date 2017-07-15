#ifndef DEPENDENCE_ANALYZER_HPP
#define DEPENDENCE_ANALYZER_HPP

#include <isl/ast_build.h>
#include <isl/ast.h>
#include <isl/schedule.h>
#include <Statement.hpp>
#include <vector>
#include <Dependences.hpp>

using namespace std;

class DependenceAnalyzer {
  public:
  static Dependences* ComputeDependencies (vector<Statement*>  & statements);
};

#endif
