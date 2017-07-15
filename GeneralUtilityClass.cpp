#include <map>
#include <string>
#include <assert.h>
#include <sstream>
#include <fstream>
#include<GeneralUtilityClass.hpp>

using namespace std;

string GeneralUtilityClass::ExtractEqualities(isl_set* set) {
  string s;
  return s;
}

string GeneralUtilityClass::ConstructArrayReference(isl_set* set) {
  string arrayName = GeneralUtilityClass::GetSetName(set);
  vector<string*> iteratorNames;
  GeneralUtilityClass::CollectIteratorNames(iteratorNames, set);

  string Arrayreference = arrayName;
  for (int i = 0; i < iteratorNames.size(); i++) {
    Arrayreference += "[" + *(iteratorNames.at(i)) + "]";
  }
  return Arrayreference;
}

isl_ast_node* GeneralUtilityClass::CreateCustomNode2(isl_ast_node *node, isl_ast_build* build2, void* user) {
  string *str = new string("add_to_checksum_n_times(def_checksum, A[i0][i1], n-1) ");
  isl_id *id = isl_id_alloc (isl_ast_build_get_ctx(build2), NULL, str);

  return isl_ast_node_set_annotation(node,id);
}

isl_ast_node* GeneralUtilityClass::CreateCustomNode (isl_ast_node *leaf_node, isl_ast_build* build, void* user) {
  isl_ctx* ctx = isl_ast_build_get_ctx(build);
  isl_union_map *unionMap = isl_ast_build_get_schedule(build);
  isl_union_set *domain = isl_union_map_domain(isl_union_map_copy(unionMap));

  LiveDataInput* liveDataInput2 = (LiveDataInput*) user;
  isl_union_map* reverseLiveInMap = liveDataInput2->reverseLiveInMap;
  vector<string> *LeafNodeStr = liveDataInput2->LeafNodeStr;

  isl_set *set = isl_set_from_union_set(isl_union_set_copy(domain));
  isl_set *context = isl_union_set_params(isl_union_set_copy(domain));

  set = isl_set_remove_redundancies(set);
  set = isl_set_coalesce(set);

  vector<string*> iteratorNames;
  GeneralUtilityClass::CollectIteratorNames(iteratorNames, set);

  vector<string*> MissingIterators;
  isl_set* ParameterizedSet = GeneralUtilityClass::ParameterizeAllDimensions (set, &iteratorNames, &MissingIterators);

  string arrayName = GeneralUtilityClass::GetSetName(set);
  string Arrayreference = arrayName;
  for (int i = 0; i < iteratorNames.size(); i++) {
    Arrayreference += "[" + *(iteratorNames.at(i)) + "]";
  }

  string macro = "add_to_checksum_n_times";
  string *messageStr = new string ("{\n");

  for (int i = 0; i < MissingIterators.size(); i++) {
    if (i == 0) {
      *messageStr += "int ";
    }
    if (i > 0) {
      *messageStr += ", ";
    }

    *messageStr += *MissingIterators.at(i);
    if (i == MissingIterators.size() - 1) {
      *messageStr += ";\n";
    }
  }

  if (arrayName.length() > 0) {
    if (reverseLiveInMap != NULL) {
      vector<string*> *use_counts = new vector<string*>();
      isl_union_set *iterationSetThatUsesAParametricLiveInReference = GeneralUtilityClass::ApplyUnionMapOnSet (ParameterizedSet, reverseLiveInMap, 1);
      isl_union_pw_qpolynomial* paramLiveInCardinality = isl_union_set_card(isl_union_set_copy(iterationSetThatUsesAParametricLiveInReference));
      paramLiveInCardinality = isl_union_pw_qpolynomial_coalesce(paramLiveInCardinality);

      paramLiveInCardinality = isl_union_pw_qpolynomial_gist(
                                isl_union_pw_qpolynomial_copy(paramLiveInCardinality),
                                isl_union_set_from_set(isl_set_copy(context)));

      paramLiveInCardinality = isl_union_pw_qpolynomial_coalesce(paramLiveInCardinality);

      GeneralUtilityClass::PrintUnion_pw_qpolynomialCode(paramLiveInCardinality, &use_counts);
      map<string, string> *polyMap = new map<string,string>();
      map<string, string>::iterator polyIt;
      GeneralUtilityClass::PrintPwQpolyInUnionPwQpoly(paramLiveInCardinality, &polyMap);

      if (use_counts->size() > 1) {
        for (int k = 0; k < use_counts->size(); k++) {
          cout << *(use_counts->at(k)) << endl;
        }
      }
      assert (use_counts->size() == 1);
      for (polyIt=polyMap->begin();polyIt!=polyMap->end();++polyIt) {
        *messageStr += "if (" + (polyIt->second) + ")\n{\n";
        LeafNodeStr->push_back(polyIt->second);
        *messageStr += macro + "(def_checksum, " + Arrayreference + "," + (polyIt->first) + " );\n}\n";
      }

    } else {
      *messageStr += macro + "(use_checksum, " + Arrayreference + ", 1);\n";
    }
  }
  *messageStr += "\n}\n";

  isl_id *id = isl_id_alloc (ctx, NULL, messageStr);
  __isl_give isl_id *id2 = isl_ast_node_get_annotation(leaf_node);
  __isl_give isl_printer *printer = isl_printer_to_file(ctx, stdout);
  isl_printer_print_id (printer, id2);
  const char *idname = isl_id_get_name (id2);

  return  isl_ast_node_set_annotation (leaf_node, id);
}

isl_printer* GeneralUtilityClass::PrintingFunction (isl_printer *p, isl_ast_print_options *options, isl_ast_node *node, void *user) {
  __isl_give isl_id *id = isl_ast_node_get_annotation(node);
  void *usr = isl_id_get_user (id);
  string *userStr = (string*) usr;
  //cout << "in printing function: "<< *userStr << endl;

  return isl_printer_print_str (p, (*userStr).c_str());
}

isl_printer* GeneralUtilityClass::PrintTest(isl_printer *p, isl_ast_print_options *op, isl_ast_node *node2, void *user) {
  isl_id *id = isl_ast_node_get_annotation(node2);
  void *usr = isl_id_get_user(id);
  string *userStr = (string *) usr;
  //cout << "In print test: " << *userStr << endl;
  return isl_printer_print_str(p, (*userStr).c_str());
}

isl_set* GeneralUtilityClass::ParameterizeAllDimensions (isl_set *domainSet, vector<string*> *iteratorNames, vector<string*> *MissingIterators) {
  isl_set *paramDomainSet = isl_set_extend(isl_set_copy(domainSet), isl_set_n_param (domainSet) + isl_set_n_dim(domainSet), isl_set_n_dim(domainSet));

  for (int j = 0; j < isl_set_n_dim(domainSet); j++) {
    int param_pos = isl_set_n_param (domainSet) + j;
    const char* iteratorName = isl_set_get_dim_name(isl_set_copy(domainSet), isl_dim_set, j);
    if (iteratorName != NULL) {
      paramDomainSet = isl_set_set_dim_name(isl_set_copy(paramDomainSet), isl_dim_param, param_pos, iteratorName);
    }
  }

  for (int j = 0; j < isl_set_n_dim(domainSet); j++) {
    int param_pos = isl_set_n_param (domainSet) + j;
    int iterator_pos = j;

    isl_pw_aff *min = isl_set_dim_min (isl_set_copy(domainSet), iterator_pos);
    isl_pw_aff *max = isl_set_dim_max (isl_set_copy(domainSet), iterator_pos);

    isl_pw_aff *min2 = isl_set_dim_min (isl_set_copy(paramDomainSet), iterator_pos);
    isl_pw_aff *max2 = isl_set_dim_max (isl_set_copy(paramDomainSet), iterator_pos);

    if (strcmp(GeneralUtilityClass::Print_pw_affToString(min), GeneralUtilityClass::Print_pw_affToString(max)) == 0 ||
        strcmp(GeneralUtilityClass::Print_pw_affToString(min2), GeneralUtilityClass::Print_pw_affToString(max2)) == 0) {
      isl_pw_aff *minToUse = NULL;
      if (strcmp(GeneralUtilityClass::Print_pw_affToString(min), GeneralUtilityClass::Print_pw_affToString(max)) == 0) {
        minToUse = min;
      }else{
        minToUse = min2;
      }

      const char* iteratorName = isl_set_get_dim_name(isl_set_copy(domainSet), isl_dim_set, j);

      if (iteratorName != NULL) {
        string *iteratorAssignment = new string(iteratorName);
        *iteratorAssignment += " = " + string(GeneralUtilityClass::Print_pw_affToString(minToUse));

        if (MissingIterators != NULL) {
          MissingIterators->push_back(iteratorAssignment);
        }
      }else{
        string *iteratorAssignment = new string(GeneralUtilityClass::Print_pw_affToString(minToUse));

        if (iteratorNames != NULL){
          iteratorNames->at(iterator_pos) = iteratorAssignment;
        }
      }
    }

    if (strcmp(GeneralUtilityClass::Print_pw_affToString(min), GeneralUtilityClass::Print_pw_affToString(max)) != 0){
      paramDomainSet = isl_set_equate (isl_set_copy(paramDomainSet), isl_dim_param, param_pos, isl_dim_set, iterator_pos);
    }
  }

  return paramDomainSet;
}

int GeneralUtilityClass::LiveSetApplyReverseLiveMapOnParameterizedLiveInSet (isl_set *set, void *user) {
  set = isl_set_coalesce(set);
  //GeneralUtilityClass::PrintSet(set);

  LiveDataInput* liveDataInput = (LiveDataInput*) user;
  isl_union_map* reverseLiveInMap = liveDataInput->reverseLiveInMap;
  vector<string*> *LiveDataDefChecksumCode = liveDataInput->LiveDataDefChecksumCode;

  LiveDataInput* liveDataInput2 = new LiveDataInput;
  liveDataInput2->reverseLiveInMap = reverseLiveInMap;

  vector<string> *LeafNodeStr = new vector<string>();
  vector<string>::iterator LeafNodeIter;
  liveDataInput2->LeafNodeStr = LeafNodeStr;

  vector<string*> iteratorNames;
  GeneralUtilityClass::CollectIteratorNames(iteratorNames, set);

  isl_id_list* iteratorNameIDs = GeneralUtilityClass::Generate_ISL_ID_List(isl_set_get_ctx(set), iteratorNames);
  isl_union_map *schedule = isl_union_set_identity(isl_union_set_from_set(set));
  schedule = isl_union_map_coalesce(schedule);

  isl_set *context = isl_set_universe(isl_union_map_get_space(schedule));
  isl_ast_build *build = isl_ast_build_from_context(context);
  build = isl_ast_build_set_iterators(build, iteratorNameIDs);

  isl_ast_build *build2 = isl_ast_build_copy(build);
  build = isl_ast_build_set_at_each_domain(build, &CreateCustomNode, liveDataInput2);

  //GeneralUtilityClass::PrintUnionMap(schedule);
  isl_ast_node *node = isl_ast_build_ast_from_schedule (build, schedule);
  __isl_give isl_printer *printer = isl_printer_to_str(isl_set_get_ctx(set));
  printer = isl_printer_set_output_format(printer, ISL_FORMAT_C);
  isl_ast_print_options *options = isl_ast_print_options_alloc(isl_set_get_ctx(set));
  options = isl_ast_print_options_set_print_user(options, &PrintingFunction, NULL);

  isl_ast_node_print(node, printer, options);
  string nodeCode = isl_printer_get_str(printer);

  if (nodeCode.find("use_checksum")!=string::npos) {
    string *add = new string("");
    while (nodeCode.find("use_checksum")!=string::npos) {
      nodeCode = nodeCode.substr(nodeCode.find_first_of("add"),nodeCode.find_last_of(";")-nodeCode.find_first_of("add")+1);
      *add += nodeCode.substr(nodeCode.find_first_of("add"),nodeCode.find_first_of(";")-nodeCode.find_first_of("add")+1);
      *add += "\n";
      nodeCode = nodeCode.substr(nodeCode.find_first_of(";")+1,nodeCode.find_last_of(";")-nodeCode.find_first_of(";"));
    }

    LiveDataDefChecksumCode->push_back(new string(*add));
  } else {
    LiveDataDefChecksumCode->push_back(new string(nodeCode));
  }

  return 0;
}

isl_set* GeneralUtilityClass::RemakeSet(isl_set* set) {
  char *setStr;
  __isl_give isl_printer *printer = isl_printer_to_str(isl_set_get_ctx(set));
  isl_printer_print_set (printer, set);
  setStr = isl_printer_get_str(printer);
  set = isl_set_read_from_str(isl_set_get_ctx(set), setStr);
  return set;
}

isl_set* GeneralUtilityClass::DumpLeafNode(isl_space* space, string constraint) {
  FILE *tmp;
  tmp = fopen(".leafNode","w");
  isl_ctx *ctx = isl_space_get_ctx(space);
  isl_printer *p;
  p = isl_printer_to_file(ctx, tmp);
  isl_printer_print_space(p,space);
  fclose(tmp);
  isl_printer_free(p);

  // write file with constraint
  ofstream fcst(".cst");
  if (!fcst) return NULL;
  fcst << constraint << endl;
  fcst.close();

  //execute python
  string *py = new string("python leafNode.py .leafNode .cst");
  system((*py).c_str());

  // read file with result
  tmp = fopen(".set", "r");
  isl_set *set = isl_set_read_from_file(ctx, tmp);
  fclose(tmp);

  // remove tmp files
  system("rm .leafNode");
  system("rm .cst");
  system("rm .set");
  return set;
}

void GeneralUtilityClass::DataSetPrintUnionSetCode (isl_union_set* set, int remake, isl_union_map *reverseLiveInMap,
    vector<string*> *LiveDataDefChecksumCode, map<string,int> &ArraySizeMap) {

  if (remake == 1) {
    char *setStr;
    
    __isl_give isl_printer *printer = isl_printer_to_str(isl_union_set_get_ctx(set));
    isl_printer_print_union_set (printer, set);
    setStr = isl_printer_get_str(printer);
    
    set = isl_union_set_read_from_str(isl_union_set_get_ctx(set), setStr);
  }

  LiveDataInput* liveDataInput = new LiveDataInput;
  liveDataInput->reverseLiveInMap = reverseLiveInMap;
  liveDataInput->LiveDataDefChecksumCode = LiveDataDefChecksumCode;
  liveDataInput->ArraySizeMap = ArraySizeMap;

  isl_union_set_foreach_set(set, &LiveSetApplyReverseLiveMapOnParameterizedLiveInSet, liveDataInput);
}

void GeneralUtilityClass::PrintUnionMap (isl_union_map* map) {
  if (map == NULL) {
    return;
  }
  __isl_give isl_printer *printer = isl_printer_to_file(isl_union_map_get_ctx(map), stdout);
  isl_printer_print_union_map (printer, map);
}

void GeneralUtilityClass::PrintMap(isl_map* map) {
  if (map == NULL) {
    return;
  }
  __isl_give isl_printer *printer = isl_printer_to_file(isl_map_get_ctx(map), stdout);
  isl_printer_print_map (printer, map);
}

void GeneralUtilityClass::PrintSet(isl_set* set) {
  __isl_give isl_printer *printer = isl_printer_to_file(isl_set_get_ctx(set), stdout);
  isl_printer_print_set (printer, set);
}

void GeneralUtilityClass::PrintBasicSet(isl_basic_set* basicSet) {
  __isl_give isl_printer *printer = isl_printer_to_file(isl_basic_set_get_ctx(basicSet), stdout);
  isl_printer_print_basic_set(printer, basicSet);
}

isl_union_set *GeneralUtilityClass::ApplyUnionMapOnSet(isl_set *set, isl_union_map* map, int remake) {
  if (remake == 0) {
    return isl_union_set_apply (isl_union_set_from_set (isl_set_copy(set)), isl_union_map_copy(map));
  } else {
    char *setStr, *mapStr;
    {
      __isl_give isl_printer *printer = isl_printer_to_str(isl_set_get_ctx(set));
      isl_printer_print_set (printer, set);
      setStr = isl_printer_get_str(printer);
    }

    {
      __isl_give isl_printer *printer = isl_printer_to_str(isl_set_get_ctx(set));
      isl_printer_print_union_map (printer, map);
      mapStr = isl_printer_get_str(printer);
    }

    isl_set *newSet = isl_set_read_from_str(isl_set_get_ctx(set), setStr);
    isl_union_map* newMap = isl_union_map_read_from_str(isl_set_get_ctx(set), mapStr);

    return isl_union_set_apply (isl_union_set_from_set (isl_set_copy(newSet)), isl_union_map_copy(newMap));
  }
}

isl_union_set* GeneralUtilityClass::RemakeAUnionSet(isl_union_set* set) {
  char *setStr;
  __isl_give isl_printer *printer = isl_printer_to_str(isl_union_set_get_ctx(set));
  isl_printer_print_union_set (printer, set);
  setStr = isl_printer_get_str(printer);

  isl_union_set *newSet = isl_union_set_read_from_str(isl_union_set_get_ctx(set), setStr);
  return newSet;
}

isl_union_set *GeneralUtilityClass::ApplyUnionMapOnUnionSet(isl_union_set *set, isl_union_map* map, int remake) {
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

    return isl_union_set_apply (isl_union_set_copy(newSet), isl_union_map_copy(newMap));
  }
}

isl_union_set *GeneralUtilityClass::ApplyMapOnUnionSet(isl_union_set *set, isl_map* map) {
  return isl_union_set_apply (isl_union_set_copy (set), isl_union_map_from_map (isl_map_copy(map)));
}

void GeneralUtilityClass::PrintUnionSet(isl_union_set* set) {
  __isl_give isl_printer *printer = isl_printer_to_file(isl_union_set_get_ctx(set), stdout);
  isl_printer_print_union_set (printer, set);
}

void GeneralUtilityClass::PrintUnion_pw_qpolynomial(isl_union_pw_qpolynomial *poly) {
  isl_printer *printer = isl_printer_to_file(isl_union_pw_qpolynomial_get_ctx(poly), stdout);
  isl_printer_print_union_pw_qpolynomial (printer, poly);
}

void GeneralUtilityClass::Print_pw_aff(isl_pw_aff *aff) {
  isl_printer *printer = isl_printer_to_file(isl_pw_aff_get_ctx(aff), stdout);
  isl_printer_print_pw_aff (printer, aff);
}

char* GeneralUtilityClass::Print_pw_affToString(isl_pw_aff *aff) {
  isl_printer *printer = isl_printer_to_str(isl_pw_aff_get_ctx(aff));
  printer = isl_printer_set_output_format(printer, ISL_FORMAT_C);
  isl_printer_print_pw_aff (printer, aff);
  return isl_printer_get_str(printer);
}

int GeneralUtilityClass::PrintPw_qpolynomialCode(isl_pw_qpolynomial *poly, void *user) {
  isl_printer *printer = isl_printer_to_str(isl_pw_qpolynomial_get_ctx(poly));
  printer = isl_printer_set_output_format(printer, ISL_FORMAT_C);
  isl_printer_print_pw_qpolynomial (printer, poly);
  char *output = isl_printer_get_str(printer);

  vector<string*> *use_counts = (vector<string*> *) user;
  use_counts->push_back(new string(output));
}

void GeneralUtilityClass::PrintPointCode(isl_point *point) {
  isl_printer *printer = isl_printer_to_file(isl_point_get_ctx(point), stdout);
  printer = isl_printer_set_output_format(printer, ISL_FORMAT_C);
  isl_printer_print_point (printer, point);
}

void GeneralUtilityClass::PrintPw_qpolynomialCode(isl_pw_qpolynomial *poly) {
  PrintPw_qpolynomialCode(poly, NULL);
}

void GeneralUtilityClass::PrintUnion_pw_qpolynomialCode(isl_union_pw_qpolynomial *poly, vector<string*> **use_counts) {
  *use_counts = new vector<string*>();
  isl_union_pw_qpolynomial_foreach_pw_qpolynomial(poly, &PrintPw_qpolynomialCode, *use_counts);
}

int GeneralUtilityClass::PrintSpace(isl_space *space) {
  isl_printer *printer = isl_printer_to_str(isl_space_get_ctx(space));
  isl_printer_print_space(printer, space);
  isl_printer_get_str(printer);
}

int GeneralUtilityClass::PrintPw_qpolynomial(isl_pw_qpolynomial *poly, void *user) {
  isl_printer *printer = isl_printer_to_str(isl_pw_qpolynomial_get_ctx(poly));
  printer = isl_printer_set_output_format(printer, ISL_FORMAT_C);
  isl_printer_print_pw_qpolynomial (printer, poly);
  char *output = isl_printer_get_str(printer);
  cout << "Output: " << output << endl;
}

int GeneralUtilityClass::Print_basic_set_constraint(isl_basic_set *basicSet, void *user) {
  isl_basic_set_foreach_constraint(basicSet, &Print_constraint, user);
}

int GeneralUtilityClass::Print_constraint(isl_constraint *constraint, void *user) {
  isl_printer *printer = isl_printer_to_str(isl_constraint_get_ctx(constraint));
  printer = isl_printer_set_output_format(printer, ISL_FORMAT_ISL);
  isl_printer_print_constraint(printer,constraint);
  cout << "single constraint : " << isl_printer_get_str(printer)<<endl;
}

int GeneralUtilityClass::PrintPw_qpolynomialCellCode(isl_set *cellSet,isl_qpolynomial *poly,void *user) {
  isl_printer *cprinter = isl_printer_to_str(isl_set_get_ctx(cellSet));
  isl_printer_print_set(cprinter,cellSet);
  string cond = isl_printer_get_str(cprinter);

  cond = cond.substr(cond.find(":")+1,cond.find("}")-cond.find(":")-1);
  while(cond.find("and")!=string::npos) {
    cond.replace(cond.find("and"), 3, "&&");
  }
  while(cond.find("or")!=string::npos) {
    cond.replace(cond.find("or"), 2, "||");
  }
  while(cond.find(" =")!=string::npos) {
    cond.replace(cond.find(" ="), 2, "==");
  }

  isl_printer *printer = isl_printer_to_str(isl_qpolynomial_get_ctx(poly));
  printer = isl_printer_set_output_format(printer, ISL_FORMAT_C);
  isl_printer_print_qpolynomial(printer,poly);

  map<string, string> *polyMap = (map<string, string> *) user;
  polyMap->insert ( pair<string,string>(isl_printer_get_str(printer),cond) );
}

int GeneralUtilityClass::PrintPw_qpolynomialCell(isl_pw_qpolynomial *poly, void *user) {
  isl_pw_qpolynomial_foreach_piece(poly, &PrintPw_qpolynomialCellCode, user);
}

void GeneralUtilityClass::PrintPwQpolyInUnionPwQpoly(isl_union_pw_qpolynomial *poly,map<string,string> **use_counts) {
  *use_counts = new map<string,string>();
  isl_union_pw_qpolynomial_foreach_pw_qpolynomial(poly, &PrintPw_qpolynomialCell, *use_counts);
}

void GeneralUtilityClass::PrintSetCode (isl_set* set) {
  __isl_give isl_printer *printer = isl_printer_to_file(isl_set_get_ctx(set), stdout);

  isl_union_set *unionSet = isl_union_set_from_set(set);
  isl_union_map *schedule = isl_union_set_identity(unionSet);
  isl_set *context = isl_set_universe(isl_union_map_get_space(schedule));
  isl_ast_build *build = isl_ast_build_from_context(context);
  isl_ast_node *node = isl_ast_build_ast_from_schedule (build, schedule);
  printer = isl_printer_set_output_format(printer, ISL_FORMAT_C);
  isl_printer_print_ast_node (printer, node);
}

isl_id_list *GeneralUtilityClass::Generate_ISL_ID_List(isl_ctx *ctx, vector<string*> &iteratorNames) {
  int i = 0;
  isl_id_list *names;

  names = isl_id_list_alloc(ctx, iteratorNames.size());
  for (i = 0; i < iteratorNames.size(); ++i) {
    isl_id *id;
    id = isl_id_alloc(ctx, (iteratorNames.at(i))->c_str(), NULL);
    names = isl_id_list_add(names, id);
  }

  return names;
}

string GeneralUtilityClass::GetSetName(isl_set *set) {
  string setName;
  if (isl_set_has_tuple_id(set)) {
    isl_id *id = isl_set_get_tuple_id(set);
    const char *name = isl_id_get_name(id);
    setName = name;
  } 

  return setName;
}

void GeneralUtilityClass::CollectIteratorNames(vector<string*>  &iteratorNames, isl_set *domainSet) {
  for (int j = 0; j < isl_set_n_dim(domainSet); j++) {
    const char* iteratorName = isl_set_get_dim_name(isl_set_copy(domainSet), isl_dim_set, j);
    if (iteratorName != NULL) {
      iteratorNames.push_back(new string(iteratorName));
    } else {
      iteratorNames.push_back(new string("c"+ConvertIntToString(j)));
    }
  }
}

void GeneralUtilityClass::PrintUnionSetCode (isl_union_set* set, int remake) {
  if (remake == 1) {
    char *setStr, *mapStr;
    {
      __isl_give isl_printer *printer = isl_printer_to_str(isl_union_set_get_ctx(set));
      isl_printer_print_union_set (printer, set);
      setStr = isl_printer_get_str(printer);
    }
    set = isl_union_set_read_from_str(isl_union_set_get_ctx(set), setStr);   
  } else {
    set = isl_union_set_copy(set);
  }

  __isl_give isl_printer *printer = isl_printer_to_file(isl_union_set_get_ctx(set), stdout);
  isl_union_map *schedule = isl_union_set_identity(set);
  isl_set *context = isl_set_universe(isl_union_map_get_space(schedule));
  isl_ast_build *build = isl_ast_build_from_context(context);
  isl_ast_node *node = isl_ast_build_ast_from_schedule (build, schedule);
  printer = isl_printer_set_output_format(printer, ISL_FORMAT_C);    
  isl_printer_print_ast_node (printer, node);
}

string GeneralUtilityClass::ConvertIntToString(int Number) {
  string Result;          
  ostringstream convert;   
  convert << Number;      
  Result = convert.str(); 
  return Result;
}
