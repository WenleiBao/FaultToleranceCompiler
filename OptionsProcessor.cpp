#include <assert.h>
#include <OptionsProcessor.hpp>

string OptionsProcessor::GetOutputFileName(string fileName) {
  string outputFileName;

  int last_index = fileName.length() - 1;
  int begin_index = 0;

  for (int i = last_index; i >= 0; i--) {
    if (fileName[i] == '/') {
      begin_index = i+1;
      break;
    }
  }

  if (begin_index > last_index) {
    assert("File name is not correct");
  }

  outputFileName = "FT_" + fileName.substr(begin_index, last_index - begin_index + 1);
  return outputFileName;
}
