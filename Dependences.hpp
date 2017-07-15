#ifndef DEPENDENCES_HPP
#define DEPENDENCES_HPP

#include <isl/ast_build.h>
#include <isl/ast.h>
#include <isl/schedule.h>

class Dependences {
  private:
    isl_union_map *flow_dep;
    isl_union_map *live_in;
    isl_union_map *writeMaps;
    isl_union_map *WAW;

  public:
    Dependences();
    ~Dependences();
    isl_union_map* getFlowDependences();
    isl_union_map* getLiveIn();
    isl_union_map* getWriteMaps();
    isl_union_map* getWAW();

    void setFlowDependences(isl_union_map*);
    void setWriteMaps(isl_union_map*);
    void setLiveIn(isl_union_map*);
    void setWAW(isl_union_map*);   
    void display();
};

#endif
