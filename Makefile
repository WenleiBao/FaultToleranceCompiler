###########################################################################
## Adjust the following two variables to match your project.             ##
###########################################################################

# Only specify source files, not header files.
# Lists of files can be denoted by white-space separated filenames across
# one or more lines.  Use a backslash (\) at the end of a line if the list
# continues to the next line, e.g.
#
SOURCE_FILES	=	\
			main.cpp FTOrchestrator.cpp Statement.cpp Dependences.cpp DependenceAnalyzer.cpp GeneralUtilityClass.cpp UseCountComputer.cpp CodeParser.cpp LiveDataComputer.cpp \
			OptionsProcessor.cpp

BINARY_FILE	=	ft_compiler


PET_INSTALL = /Users/wenleibao/Softwares/barvinok-0.36_install
NTL_INSTALL = /Users/wenleibao/Softwares/ntl-5.5.2_install
GMP_INSTALL = /Users/wenleibao/Softwares/gmp-6.0.0_intall



CFLAGS = -fno-rtti
LLVM_BUILD_PATH = /Users/wenleibao/Softwares/llvm/build/
LLVM_BIN_PATH = $(LLVM_BUILD_PATH)/bin
LLVM_LIBS=all
LLVM_CONFIG_COMMAND = $(LLVM_BIN_PATH)/llvm-config --cxxflags --libs $(LLVM_LIBS); $(LLVM_BIN_PATH)/llvm-config  --ldflags 
CLANG_BUILD_FLAGS = -I$(LLVM_BUILD_PATH)/include/clang

CLANGLIBS = \
  -lclangFrontendTool -lclangFrontend -lclangDriver \
  -lclangSerialization -lclangCodeGen -lclangParse \
  -lclangSema -lclangStaticAnalyzerFrontend \
  -lclangStaticAnalyzerCheckers -lclangStaticAnalyzerCore \
  -lclangAnalysis -lclangARCMigrate -lclangRewriteCore -lclangRewriteFrontend \
  -lclangEdit -lclangAST -lclangLex -lclangBasic


###########################################################################
## You should not need to change anything below this line.               ##
###########################################################################

CXX = g++
LIBRARY_FLAGS = -L$(GMP_INSTALL)/lib -L$(PET_INSTALL)/lib -L$(NTL_INSTALL)/lib -lpet -lisl -lbarvinok  -lntl  -lgmp -lisl-polylib -lpolylibgmp $(CFLAGS) $(CLANG_BUILD_FLAGS) $(CLANGLIBS) `$(LLVM_CONFIG_COMMAND)`
CXXFLAGS = -O2 -I . -I utilities/ -I$(GMP_INSTALL)/include  -I$(PET_INSTALL)/include  $(CLANG_BUILD_FLAGS) `$(LLVM_CONFIG_COMMAND)`

TEMP0_FILES = $(SOURCE_FILES:.cpp=.o)
TEMP1_FILES = $(TEMP0_FILES:.C=.o)
OBJECT_FILES = $(TEMP1_FILES:.cc=.o)
OTHER_BINARIES = 


all		:	$(BINARY_FILE)

$(BINARY_FILE)	:	$(OBJECT_FILES)
			$(CXX) -o $(BINARY_FILE) $(LDFLAGS) $(OBJECT_FILES) $(OTHER_BINARIES) $(LIBRARY_FLAGS)
                        
.cpp.o          :
			$(CXX) -c $(CXXFLAGS) -o $@ $<
.C.o            :
			$(CXX) -c $(CXXFLAGS) -o $@ $<
.cc.o           :
			$(CXX) -c $(CXXFLAGS) -o $@ $<

clean		:
			rm -f *.o
			rm $(BINARY_FILE)


