#ifndef STATEMENT_HPP
#define STATEMENT_HPP

#include <string>
#include <vector>

#include <isl/ast.h>
#include <isl/ast_build.h>
#include <isl/schedule.h>
#include <isl/set.h>
#include <isl/union_map.h>
#include <isl/ctx.h>
#include <isl/map.h>
#include <isl/flow.h>

using namespace std;

class Statement {
  private:
    string def_macro;
    string domain;
    string schedule;
    string write_reference;
    string parallelTaskDomain;
    int SourceCodeLineNumber;

    isl_ctx *ctx;
    isl_set *domainSet;
    isl_set *parallelTaskDomainSet;
    isl_map *scheduleMap;
    isl_map *write_referenceMap;
    isl_union_pw_qpolynomial* targetSetCardinality;

    vector<string> read_references;
    vector<string*> *UseDataCodes;
    vector<isl_map*> read_referenceMaps;

  public:
    Statement(isl_ctx *ctx_in);
    void ReadInputString (string & str);
    void ReadStatementDescription();
    void DisplayStatementDescription();
    void ConstructISLObjects();
    void PrintSetCode (isl_set* set);
    void PrintUnionSetCode (isl_union_set* set);
    void PrintMap(isl_map* map);
    void PrintSet(isl_set* set);
    void PrintUnionMap (isl_union_map* map);
    void PrintUnionSet(isl_union_set* set);

    isl_set* ComputeWriteSet();
    isl_set* ComputeReadSet();

    string getDomain();
    string getParallelTaskDomain();
    string get_write_reference();
    string get_schedule();
    string getDefMacro();

    void setDomain(string dom);
    void setParallelTaskDomain(string dom);
    void set_read_references(vector<string> read_refs);
    void set_write_reference(string s);
    void set_schedule(string sched);
    int  getSourceCodeLineNumber();
    void setSourceCodeLineNumber(int l);
    void setDefMacro(string s);

    vector<string>  get_read_references();
    vector<string*> *getParameterizedIterators();
    vector<string*> *GetUseDataCodes();
    void SetUseDataCodes(vector<string*> *);
    void setParameterizedIterators(vector<string*> *paramIters);

    void setCtx(isl_ctx *ctx);
    void setDomainSet(isl_set *domainSet);
    void setParallelTaskDomainSet(isl_set *domainSet);
    void setReadReferenceMaps(vector<isl_map*> read_referenceMaps);
    void addToReadReferenceMaps (isl_map *read_referenceMap);
    void setWriteReferenceMaps(isl_map *write_referenceMap);
    void setScheduleMap(isl_map *scheduleMap);
    void setTargetSetCardinality(isl_union_pw_qpolynomial* targetSetCardinality);

    isl_ctx *getCtx();
    isl_set *getDomainSet();
    isl_set *getParallelTaskDomainSet();
    isl_map *getWriteReferenceMap();
    isl_map *getScheduleMap();
    isl_union_pw_qpolynomial* GetTargetSetCardinality();    
    vector<isl_map*> getReadReferenceMaps();
};


#endif
