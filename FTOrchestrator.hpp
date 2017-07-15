#ifndef FT_ORCHESTRATOR_HPP
#define FT_ORCHESTRATOR_HPP

#include <string>
#include <vector>
#include <map>
#include <pet.h>
#include <Statement.hpp>
#include <DependenceAnalyzer.hpp>

using namespace std;

struct FTReturnValues {
  vector<string*> *LiveDataDefChecksumCode;
  vector<Statement*> Statements;
  map<string,int> ArraySizeMap;
};

class FTOrchestrator {
  private:
    vector<Statement*> statements;
    Dependences* dependences;
    map<string,int> ArraySizeMap;

  public:
    FTOrchestrator();
    FTReturnValues drive(string fileName);
    void FormStatements (isl_ctx*  ctx, struct pet_scop *scop);
    void MakeSchedulesOfAllStatementsEquiDimensional();
    vector<Statement*> GetStatements();
};


#endif
