#ifndef GENERAL_UTILITY_CLASS_HPP
#define GENERAL_UTILITY_CLASS_HPP

#include <map>
#include <string>
#include <vector>

#include <isl/ast.h>
#include <isl/ast_build.h>
#include <isl/schedule.h>
#include <isl/union_map.h>
#include <isl/set.h>
#include <isl/ctx.h>
#include <isl/map.h>
#include <isl/flow.h>
#include <isl/aff.h>
#include <isl/point.h>
#include <isl/polynomial.h>
#include <barvinok/isl.h>

using namespace std;

struct LiveDataInput {
  isl_union_map* reverseLiveInMap;
  vector<string*> *LiveDataDefChecksumCode;
  map<string,int> ArraySizeMap;
  vector<string> *LeafNodeStr;
};

class GeneralUtilityClass {
  public:
    static void PrintMap (isl_map* map);
    static void PrintSet(isl_set* set);
    static void PrintUnionSet(isl_union_set* set);
    static void PrintUnionMap (isl_union_map* map);
    static void PrintSetCode (isl_set* set);
    static void PrintPointCode(isl_point *point);
    static void PrintUnionSetCode (isl_union_set* set, int remake = 0);
    static void PrintUnion_pw_qpolynomial(isl_union_pw_qpolynomial *poly);
    static void PrintUnion_pw_qpolynomialCode(isl_union_pw_qpolynomial *poly, vector<string*> **use_counts);
    static void PrintPw_qpolynomialCode(isl_pw_qpolynomial *poly);
    static int  PrintPw_qpolynomialCode(isl_pw_qpolynomial *poly, void *user);

    static void Print_pw_aff(isl_pw_aff *aff);
    static void CollectIteratorNames(vector<string*> &iteratorNames, isl_set *set);
    static char *Print_pw_affToString(isl_pw_aff *aff);

    static isl_set* RemakeSet(isl_set* set);
    static isl_set* ParameterizeAllDimensions (isl_set *domainSet, vector<string*> *iteratorNames, vector<string*> *MissingIterators);
    static isl_union_set *ApplyMapOnUnionSet(isl_union_set *set, isl_map* map);
    static isl_union_set *ApplyUnionMapOnSet(isl_set *set, isl_union_map* map, int remake = 0);
    static isl_union_set *ApplyUnionMapOnUnionSet(isl_union_set *set, isl_union_map* map, int remake = 0);
    static isl_union_set *RemakeAUnionSet(isl_union_set* set);

    static string GetSetName(isl_set *set);
    static string ConvertIntToString(int Number);
    static string ConstructArrayReference(isl_set* set);
    static string ExtractEqualities(isl_set* set);
    static isl_id_list *Generate_ISL_ID_List(isl_ctx *ctx, vector<string*> &iteratorNames);
    static isl_printer *PrintingFunction (isl_printer *p, isl_ast_print_options *options, isl_ast_node *node, void *user);
    static isl_ast_node *CreateCustomNode (isl_ast_node *leaf_node, isl_ast_build* build, void* user);

    static int LiveSetApplyReverseLiveMapOnParameterizedLiveInSet (isl_set *set, void *user);
    static void DataSetPrintUnionSetCode(isl_union_set* set, int remake, isl_union_map *reverseLiveInMap, vector<string*> *LiveDataDefChecksumCode, map<string,int> &ArraySizeMap);

    static void test1(isl_set* set, isl_ast_build* build, isl_union_map* baow);
    static void PrintBasicSet(isl_basic_set *basicSet);
    static void PrintPwQpolyInUnionPwQpoly(isl_union_pw_qpolynomial *poly,map<string,string> **use_counts);
    static int PrintPw_qpolynomial(isl_pw_qpolynomial *poly, void *user);
    static int PrintPw_qpolynomialCell(isl_pw_qpolynomial *poly, void *user);
    static int PrintPw_qpolynomialCellCode(isl_set *cellSet,isl_qpolynomial *poly,void *user);
    static int Print_basic_set_constraint(isl_basic_set *basicSet, void *user);
    static int Print_constraint(isl_constraint *constraint, void *user);
    static int PrintSpace(isl_space *space);
    static isl_set* DumpLeafNode(isl_space* space, string constraint);
    static isl_printer* PrintTest(isl_printer *p,isl_ast_print_options *op, isl_ast_node *node, void *user);
    static isl_ast_node* UseChecksum(isl_ast_node *leaf_node, isl_ast_build* build, void* user);
    static isl_ast_node* CreateCustomNode2(isl_ast_node *node,isl_ast_build* build2, void *user);
};

#endif
