#include <iostream>
#include <GeneralUtilityClass.hpp>
#include <UseCountComputer.hpp>
#include <LiveDataComputer.hpp>
#include <FTOrchestrator.hpp>

using namespace std;

FTOrchestrator::FTOrchestrator() {
  dependences = NULL;
}

FTReturnValues FTOrchestrator::drive(string fileName) {
  FTReturnValues ftReturnValues;

  isl_ctx* ctx = isl_ctx_alloc();
  struct pet_scop *scop = pet_scop_extract_from_C_source(ctx, fileName.c_str(), NULL);

  FormStatements(ctx, scop);

  dependences = DependenceAnalyzer::ComputeDependencies (statements);
  //dependences->display();

  UseCountComputer::ComputeFlowDependenceDefChecksumContribution(statements, dependences, ArraySizeMap);

  isl_union_set* return_live_in_data;
  vector<isl_union_pw_qpolynomial*> *return_live_in_use_count;

  LiveDataComputer::ComputeLiveInDataAndUseCounts (statements, dependences, &ftReturnValues.LiveDataDefChecksumCode, ArraySizeMap);    

  ftReturnValues.Statements = GetStatements();    
  ftReturnValues.ArraySizeMap = ArraySizeMap;

  return ftReturnValues;
}

void GatherReadWriteAccesses(Statement *statement, struct pet_expr *expr);

void FTOrchestrator::FormStatements (isl_ctx *ctx, struct pet_scop *scop) {

  int i = 0;
  for (i = 0; i < scop->n_stmt; i++) {
    Statement *statement = new Statement (ctx);      
    struct pet_stmt *petStmt = scop->stmts[i];
    statement->setSourceCodeLineNumber(petStmt->line);
    statement->setDomainSet(isl_set_remove_redundancies(isl_set_copy(petStmt->domain)));
    statement->setScheduleMap(petStmt->schedule);

    struct pet_expr *body = petStmt->body;      	      
    GatherReadWriteAccesses (statement, body);
    statements.push_back (statement);
  }

  struct pet_array **arrays = scop->arrays;
  for (i = 0; i < scop->n_array; i++) {
    struct pet_array *array = scop->arrays[i];
    char *element_type = array->element_type;
    int element_size = array->element_size;      

    isl_set *extent = array->extent;
    string arrayName = GeneralUtilityClass::GetSetName(extent);
    ArraySizeMap.insert(pair<string, int>(arrayName, element_size));
  }
}

void FTOrchestrator::MakeSchedulesOfAllStatementsEquiDimensional() {
  int maxDim = 0;
  for (int i = 0; i < statements.size(); i++) {
    if (isl_map_n_out(statements[i]->getScheduleMap()) > maxDim) {
      maxDim = isl_map_n_out(statements[i]->getScheduleMap());
    }
  }

  for (int i = 0; i < statements.size(); i++) {
    if (isl_map_n_out(statements[i]->getScheduleMap()) < maxDim) {
      isl_map *newMap = isl_map_extend(isl_map_copy(
            statements[i]->getScheduleMap()),
          isl_map_n_param(statements[i]->getScheduleMap()), 
          isl_map_n_in(statements[i]->getScheduleMap()), 
          maxDim);

      for (int j = isl_map_n_out(statements[i]->getScheduleMap()); j < maxDim; j++) {	  
        newMap = isl_map_fix_si(isl_map_copy(newMap), isl_dim_out, j, 0);	  	  
      }

      statements[i]->setScheduleMap(newMap);
    }
  }
}

void GatherReadWriteAccesses (Statement *statement, struct pet_expr *expr) {
  if (expr->type == pet_expr_access) {
    if ((expr->acc).read == 1) {
      isl_map *readMap = isl_map_intersect_domain (isl_map_copy((expr->acc).access), isl_set_copy(statement->getDomainSet()));
      statement->addToReadReferenceMaps(readMap);
    }
    if ((expr->acc).write == 1) {
      isl_map *writeMap = isl_map_intersect_domain (isl_map_copy((expr->acc).access), isl_set_copy(statement->getDomainSet()));
      statement->setWriteReferenceMaps (writeMap);
    }
  }

  for (int i = 0; i < expr->n_arg; i++) {
    GatherReadWriteAccesses (statement, expr->args[i]);
  }
}

vector<Statement*> FTOrchestrator::GetStatements() {
  return statements;
}
