#include <iostream>
#include <CodeParser.hpp>
#include <FTOrchestrator.hpp>
#include <OptionsProcessor.hpp>
using namespace std;

int main(int argc, char **argv) {
 char *fileName = "/Users/wenleibao/Documents/BitBucket/FT_compiler/test/gemm.c";
 
  if (argc >= 2) {
    fileName = argv[1];
    cout << "Input file: " << fileName << endl;
  }

 FTOrchestrator fTOrchestrator;
 string fileNameStr(fileName);

 string outputFileName = OptionsProcessor::GetOutputFileName(fileNameStr);
 cout << "Output file: " << outputFileName << endl;

 FTReturnValues ftReturnValues = fTOrchestrator.drive(fileNameStr);
 ParseScop(fileName, ftReturnValues, outputFileName);

  return 0;
}
