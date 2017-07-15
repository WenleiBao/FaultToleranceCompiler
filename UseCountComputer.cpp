#include <barvinok/isl.h>
#include <UseCountComputer.hpp>
#include <GeneralUtilityClass.hpp>

isl_set* UseCountComputer::ParameterizeAllDimensions (isl_set *domainSet) {
  isl_set *paramDomainSet = isl_set_extend(isl_set_copy(domainSet), isl_set_n_param (domainSet) + isl_set_n_dim(domainSet), isl_set_n_dim(domainSet));
  for (int j = 0; j < isl_set_n_dim(domainSet); j++) {
    int param_pos = isl_set_n_param (domainSet) + j;
    const char* iteratorName = isl_set_get_dim_name(isl_set_copy(domainSet), isl_dim_set, j);
    paramDomainSet = isl_set_set_dim_name(isl_set_copy(paramDomainSet), isl_dim_param, param_pos, iteratorName);
  }

  for (int j = 0; j < isl_set_n_dim(domainSet); j++) {
    int iterator_pos = j;
    int param_pos = isl_set_n_param (domainSet) + j;
    paramDomainSet = isl_set_equate (isl_set_copy(paramDomainSet), isl_dim_param, param_pos, isl_dim_set, iterator_pos);
  }

  return paramDomainSet;
}

int process_set (isl_set *set, void *user) {
  isl_set *paramSet = GeneralUtilityClass::ParameterizeAllDimensions(set, NULL, NULL);

  vector<isl_set*> *unionSetVector = (vector<isl_set*> *)user;
  unionSetVector->push_back (paramSet);
  return 0;
}

isl_union_set* UseCountComputer::ParameterizeAllDimensions (isl_union_set *dataSet) {
  vector<isl_set*> *unionSetVector = new vector<isl_set*>();
  int returnValue = isl_union_set_foreach_set(dataSet, &process_set, unionSetVector);

  isl_union_set *parameterizedUnionSet = NULL;
  for (int i = 0; i < unionSetVector->size(); i++) {
    isl_union_set* newSet = isl_union_set_from_set (isl_set_copy(unionSetVector->at(i)));

    if (parameterizedUnionSet == NULL) {
      parameterizedUnionSet = newSet;
    }else{
      parameterizedUnionSet = isl_union_set_union(isl_union_set_copy(parameterizedUnionSet), isl_union_set_copy(newSet));
    }
  }
  return parameterizedUnionSet;
}

int PrintAnElement(__isl_take isl_point *pnt, void *user) {
  GeneralUtilityClass::PrintPointCode(pnt);
  return 0;
}

void UseCountComputer::ComputeFlowDependenceDefChecksumContribution(vector<Statement*> statements, Dependences* dependences, map<string,int> &ArraySizeMap)
{
  for (int i = 0; i < statements.size(); i++) {
    Statement *stmt = statements[i];
    isl_set *domainSet = stmt->getDomainSet();
    isl_space *space = isl_set_get_space(domainSet);

    const char* tupleNames = isl_set_get_tuple_name (domainSet);
    const char* tupleName = isl_set_get_dim_name(isl_set_copy(domainSet), isl_dim_param, 0);

    isl_set *paramDomainSet = ParameterizeAllDimensions (domainSet);
    isl_union_map* flow_dependences = dependences->getFlowDependences();
    isl_union_set *targetIterations = GeneralUtilityClass::ApplyUnionMapOnSet(paramDomainSet, flow_dependences);

    isl_union_pw_qpolynomial* targetSetCardinality = isl_union_set_card(isl_union_set_copy(targetIterations));
    isl_set* context = isl_set_params(isl_set_copy(domainSet));

    targetSetCardinality = isl_union_pw_qpolynomial_gist(
        isl_union_pw_qpolynomial_copy(targetSetCardinality),
        isl_union_set_from_set(isl_set_copy(context)));

    stmt->setTargetSetCardinality(targetSetCardinality);
    targetSetCardinality = isl_union_pw_qpolynomial_coalesce(targetSetCardinality);

    vector<string*> *use_counts;
    GeneralUtilityClass::PrintUnion_pw_qpolynomialCode (targetSetCardinality, &use_counts);

    isl_set *writeMap = isl_map_range (isl_map_copy(stmt->getWriteReferenceMap()));
    string arrayName = GeneralUtilityClass::GetSetName(writeMap);
    map<string,int>::iterator it = ArraySizeMap.find(arrayName);
    string macro = "add_to_checksum_n_times";

    stmt->setDefMacro(macro);
    isl_union_map *reverseFlowDependenceMap = isl_union_map_reverse (isl_union_map_copy(flow_dependences));
    isl_union_set *sourceIterations = GeneralUtilityClass::ApplyUnionMapOnSet(paramDomainSet, reverseFlowDependenceMap);

    isl_union_map* liveInDataMaps = dependences->getLiveIn();
    isl_union_set* contextUnionSet = isl_union_set_from_set(isl_set_copy(context));

    isl_union_set* readSet = GeneralUtilityClass::ApplyUnionMapOnSet(paramDomainSet, liveInDataMaps);
    vector<string*>* UseDataCodes = new vector<string*>();
    for (int j = 0; j < statements.size(); j++) {
      isl_union_set *localReadSet = GeneralUtilityClass::ApplyUnionMapOnUnionSet(sourceIterations,
          isl_union_map_from_map(statements[j]->getWriteReferenceMap()));

      if (readSet == NULL) {
        readSet = localReadSet;
      } else {
        readSet = isl_union_set_union (isl_union_set_copy(readSet), isl_union_set_copy(localReadSet));
      }

      readSet = isl_union_set_coalesce(readSet);
      readSet = isl_union_set_gist(readSet, isl_union_set_copy(contextUnionSet));
    }

    GeneralUtilityClass::DataSetPrintUnionSetCode(readSet, 0, NULL, UseDataCodes, ArraySizeMap);
    statements[i]->SetUseDataCodes(UseDataCodes);
  }
}

struct Input {
  isl_union_map* reverseLiveInMap;
  vector<isl_union_pw_qpolynomial*> *live_in_use_count;
};

int ApplyReverseLiveMapOnParameterizedLiveInSet (isl_set *set, void *user) {
  Input *input = (Input*) user;
  isl_union_map* reverseLiveInMap = input->reverseLiveInMap;
  vector<isl_union_pw_qpolynomial*> *live_in_use_count = input->live_in_use_count;

  cout << "Parameterized read live-in reference: " << endl;
  GeneralUtilityClass::PrintSet (set);

  isl_union_set *iterationSetThatUsesAParametricLiveInReference = GeneralUtilityClass::ApplyUnionMapOnSet (set, reverseLiveInMap);

  isl_union_pw_qpolynomial* paramLiveInCardinality = isl_union_set_card(isl_union_set_copy(iterationSetThatUsesAParametricLiveInReference));

  cout << "use_count for a parametric live-in reference: " << endl;
  GeneralUtilityClass::PrintUnion_pw_qpolynomial (paramLiveInCardinality);
  cout << endl;

  live_in_use_count->push_back(paramLiveInCardinality);

  return 0;
}

isl_union_set *UseCountComputer::ApplyUnionMapOnUnionSet(isl_union_set *set, isl_union_map* map, int remake, int print, vector<isl_union_pw_qpolynomial*> *live_in_use_count) {
  if (remake == 0) {
    return isl_union_set_apply (isl_union_set_copy(set), isl_union_map_copy(map));
  } else {
    char *setStr, *mapStr;
    {
      __isl_give isl_printer *printer = isl_printer_to_str(isl_union_set_get_ctx(set));
      isl_printer_print_union_set (printer, set);
      setStr = isl_printer_get_str(printer);
    }

    {
      __isl_give isl_printer *printer = isl_printer_to_str(isl_union_set_get_ctx(set));
      isl_printer_print_union_map (printer, map);
      mapStr = isl_printer_get_str(printer);
    }

    isl_union_set *newSet = isl_union_set_read_from_str(isl_union_set_get_ctx(set), setStr);
    isl_union_map* newMap = isl_union_map_read_from_str(isl_union_set_get_ctx(set), mapStr);

    if (print == 0) {
      return isl_union_set_apply (isl_union_set_copy(newSet), isl_union_map_copy(newMap));
    } else {
      Input *input = new Input;
      input->reverseLiveInMap = newMap;
      input->live_in_use_count = live_in_use_count;

      int returnValue = isl_union_set_foreach_set(newSet, &ApplyReverseLiveMapOnParameterizedLiveInSet, input);
    }
  }
}

void UseCountComputer::DetermineUnusedDefinitions(vector<Statement*> statements, Dependences* dependences) {
  isl_union_set* unusedDefinitions = NULL;

  for (int i = 0; i < statements.size(); i++) {
    isl_set *domain = statements[i]->getDomainSet();
    const char* tupleNames = isl_set_get_tuple_name (domain);
    cout << "tupleName: " << tupleNames << endl;
    isl_union_set* domainUnionSet = isl_union_set_from_set (isl_set_copy(domain));
    isl_union_map* flow_dependences = dependences->getFlowDependences();
    isl_union_map* output_dependences = dependences->getWAW();

    isl_union_set* flow_dependences_domain = isl_union_map_domain (isl_union_map_copy(flow_dependences));
    isl_union_set* output_dependences_domain = isl_union_map_domain (isl_union_map_copy(output_dependences));

    isl_union_set* unusedDefinitionIterations = isl_union_set_subtract (isl_union_set_copy(domainUnionSet), isl_union_set_copy(flow_dependences_domain));
    unusedDefinitionIterations = isl_union_set_subtract (isl_union_set_copy(unusedDefinitionIterations), isl_union_set_copy(output_dependences_domain));

    cout << "Iterations that produce unused definitions: " << endl;
    GeneralUtilityClass::PrintUnionSet (unusedDefinitionIterations);

    isl_map *writeMap = statements[i]->getWriteReferenceMap();
    isl_union_set* local_unusedDefinitions =  GeneralUtilityClass::ApplyMapOnUnionSet (unusedDefinitionIterations, writeMap);

    if (unusedDefinitions == NULL) {
      unusedDefinitions = local_unusedDefinitions;
    } else{
      unusedDefinitions = isl_union_set_union (isl_union_set_copy (local_unusedDefinitions), isl_union_set_copy (unusedDefinitions));
    }
  }
}
