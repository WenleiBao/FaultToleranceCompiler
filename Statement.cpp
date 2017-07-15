#include <iostream>
#include <isl/aff.h>
#include <isl/constraint.h>
#include <Statement.hpp>
using namespace std;

Statement::Statement(isl_ctx *ctx_in) {
  ctx = ctx_in;
}

void Statement::ReadInputString(string & str) {
  str = "";
  getline(cin, str);

  if (str == "") {
    getline(cin, str);
  }
}

void Statement::ReadStatementDescription() {
  cout << "Enter the domain (set) : ";
  ReadInputString (domain);

  cout << "Enter the write reference (relation) : ";
  ReadInputString (write_reference);

  int NumReadRefs;
  cout << "Enter the number of read references : ";
  cin >> NumReadRefs;

  string temp; 
  for (int i = 0; i < NumReadRefs; i++) {    
    cout << "Enter a read reference: ";
    ReadInputString (temp);
    read_references.push_back(temp);
  }

  cout << "Enter the schedule: ";
  ReadInputString (schedule);

  cout << "Enter the parallel task domain (set) : ";
  ReadInputString (parallelTaskDomain);
}

void Statement::DisplayStatementDescription() {
  cout << "Domain: " << domain << endl;
  cout << "Write reference: " << write_reference << endl;
  cout << "Read  reference(s): " << endl;

  for (int i = 0; i < read_references.size(); i++) {
    cout << read_references[i] << endl;
  }

  cout << "Schedule: " << schedule << endl;
  cout << "ParallelTaskDomain: " << parallelTaskDomain << endl;
}

void Statement::ConstructISLObjects() {
  domainSet = isl_set_read_from_str(ctx, domain.c_str());
  write_referenceMap = isl_map_read_from_str(ctx, write_reference.c_str());
  write_referenceMap = isl_map_intersect_domain (isl_map_copy(write_referenceMap), isl_set_copy(domainSet));

  for (int i = 0; i < read_references.size(); i++) {
    isl_map *tempMap = isl_map_read_from_str(ctx, read_references[i].c_str());
    tempMap = isl_map_intersect_domain(tempMap, isl_set_copy(domainSet));
    read_referenceMaps.push_back (tempMap);
  }

  scheduleMap = isl_map_read_from_str(ctx, schedule.c_str());
  parallelTaskDomainSet = isl_set_read_from_str(ctx, parallelTaskDomain.c_str());
}

isl_set* Statement::ComputeWriteSet() {
  isl_map *local_write_referenceMap = isl_map_copy (write_referenceMap);
  isl_set *local_domainSet = isl_set_copy(domainSet);
  isl_set *writeSet = isl_set_apply(local_domainSet, local_write_referenceMap);
  return writeSet;
}

void Statement::PrintSetCode (isl_set* set) {
  __isl_give isl_printer *printer = isl_printer_to_file(ctx, stdout);
  isl_union_set *unionSet = isl_union_set_from_set(set);
  isl_union_map *schedule = isl_union_set_identity(unionSet);
  isl_set *context = isl_set_universe(isl_union_map_get_space(schedule));
  isl_ast_build *build = isl_ast_build_from_context(context);
  isl_ast_node *node = isl_ast_build_ast_from_schedule (build, schedule);
  printer = isl_printer_set_output_format(printer, ISL_FORMAT_C);    
  isl_printer_print_ast_node (printer, node);
}


void Statement::PrintUnionSetCode (isl_union_set* set) {
  __isl_give isl_printer *printer = isl_printer_to_file(ctx, stdout);
  isl_union_map *schedule = isl_union_set_identity(set);
  isl_set *context = isl_set_universe(isl_union_map_get_space(schedule));
  isl_ast_build *build = isl_ast_build_from_context(context);
  isl_ast_node *node = isl_ast_build_ast_from_schedule (build, schedule);
  printer = isl_printer_set_output_format(printer, ISL_FORMAT_C);    
  isl_printer_print_ast_node (printer, node);
}

void Statement::PrintMap (isl_map* map) {
  __isl_give isl_printer *printer = isl_printer_to_file(ctx, stdout);
  isl_printer_print_map (printer, map);
}

void Statement::PrintUnionMap (isl_union_map* map) {
  __isl_give isl_printer *printer = isl_printer_to_file(ctx, stdout);
  isl_printer_print_union_map (printer, map);
}

void Statement::PrintSet(isl_set* set) {
  __isl_give isl_printer *printer = isl_printer_to_file(ctx, stdout);
  isl_printer_print_set (printer, set);
}

void Statement::PrintUnionSet(isl_union_set* set) {
  __isl_give isl_printer *printer = isl_printer_to_file(ctx, stdout);
  isl_printer_print_union_set (printer, set);
}

string Statement::getDomain() {
  return domain;
}

void Statement::setDomain(string dom) {
  domain = dom;
}

vector<string> Statement::get_read_references() {
  return read_references;
}

void Statement::set_read_references(vector<string> read_refs) {
  read_references = read_refs;
}

string Statement::get_write_reference() {
  return write_reference;
}

void Statement::set_write_reference(string s) {
  write_reference = s;
}

string Statement::get_schedule() {
  return schedule;
}

void Statement::set_schedule(string sched)
{
  schedule = sched;
}

void Statement::setCtx(isl_ctx *ctx) {
  this->ctx = ctx;
}

void Statement::setDomainSet(isl_set *domainSet) {
  this->domainSet = domainSet;
}

void Statement::setReadReferenceMaps(vector<isl_map*> read_referenceMaps) {
  this->read_referenceMaps = read_referenceMaps;
}

void Statement::addToReadReferenceMaps (isl_map *read_referenceMap) {
  (this->read_referenceMaps).push_back(read_referenceMap);
}

void Statement::setWriteReferenceMaps(isl_map *write_referenceMap) {
  this->write_referenceMap = write_referenceMap;
}

void Statement::setScheduleMap(isl_map *scheduleMap) {
  this->scheduleMap = scheduleMap;
}

isl_ctx *Statement::getCtx() {
  return ctx;
}

isl_set *Statement::getDomainSet() {
  return domainSet;
}

vector<isl_map*> Statement::getReadReferenceMaps() {
  return read_referenceMaps;
}

isl_map *Statement::getWriteReferenceMap() {
  return write_referenceMap;
}

isl_map *Statement::getScheduleMap() {
  return scheduleMap;
}

string Statement::getParallelTaskDomain() {
  return parallelTaskDomain;
}

void Statement::setParallelTaskDomain(string dom) {
  parallelTaskDomain = dom;
}

isl_set* Statement::getParallelTaskDomainSet() {
  return parallelTaskDomainSet;
}

void Statement::setParallelTaskDomainSet(isl_set* dom) {
  parallelTaskDomainSet = dom;
}

int Statement::getSourceCodeLineNumber() {
  return SourceCodeLineNumber;
}

void Statement::setSourceCodeLineNumber(int l) {
  SourceCodeLineNumber = l;
}

void Statement::setTargetSetCardinality(isl_union_pw_qpolynomial* t) {
  targetSetCardinality = t;
}

isl_union_pw_qpolynomial* Statement::GetTargetSetCardinality() {
  return targetSetCardinality;
}

vector<string*>* Statement::GetUseDataCodes() {
  return UseDataCodes;
}

void Statement::SetUseDataCodes(vector<string*> *useDataCodes) {
  UseDataCodes = useDataCodes;
}

string Statement::getDefMacro() {
  return def_macro;
}

void Statement::setDefMacro(string s) {
  def_macro = s;
}
