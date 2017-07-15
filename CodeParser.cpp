#include <cstdio>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>
#include <CodeParser.hpp>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include <clang/Lex/Pragma.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/AST/Expr.h>
#include <Statement.hpp>
#include <clang/AST/PrettyPrinter.h>
#include <GeneralUtilityClass.hpp>

using namespace clang;
using namespace std;

CompilerInstance* pTheCompInst_g;

struct FtLoc {
  FtLoc() : end(0) {}
  unsigned start;
  unsigned end;
};

class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
  public:
    MyASTVisitor (Preprocessor &PP, Rewriter  &R, FtLoc &loc, vector<Statement*> s, vector<string*> *liveDataDefChecksumCode, map<string,int> arraySizeMap)
      : PP(PP), TheRewriter(R), loc(loc), FaultTolerantRegionStarted(false), statements(s), LiveDataDefChecksumCode(liveDataDefChecksumCode), ArraySizeMap(arraySizeMap){}

    void TraverseExprToGetOperands (Expr *expr, vector<string> &operands){
      if (expr->getStmtClass() == Stmt::BinaryOperatorClass){
        BinaryOperator* b = cast<BinaryOperator>(expr);
        TraverseExprToGetOperands (b->getLHS(), operands);
        TraverseExprToGetOperands (b->getRHS(), operands);
      } else if (expr->getStmtClass() == Stmt::IntegerLiteralClass || expr->getStmtClass() == Stmt::FloatingLiteralClass) {
        // do nothing
      }else if (expr->getStmtClass() == Stmt::ParenExprClass){
        ParenExpr *p = cast<ParenExpr>(expr);
        TraverseExprToGetOperands(p->getSubExpr(), operands);
      }else if (expr->getStmtClass() == Stmt::ImplicitCastExprClass){
        ImplicitCastExpr *i = cast<ImplicitCastExpr>(expr);
        TraverseExprToGetOperands(i->getSubExpr(), operands);
      }else{
        clang::LangOptions LangOpts;
        LangOpts.CPlusPlus = true;
        clang::PrintingPolicy Policy(LangOpts);

        string TypeS;
        llvm::raw_string_ostream s(TypeS);
        expr->printPretty(s, NULL, Policy);
        operands.push_back(s.str());
      }
    }

    bool VisitBinaryOperator(BinaryOperator *b) {
      SourceManager &SM = PP.getSourceManager();
      if (SM.getFileOffset(b->getLocStart()) <= loc.start)
        return true;
      if (SM.getFileOffset(b->getLocEnd()) >= loc.end){
        if (FaultTolerantRegionStarted == true) {
          FaultTolerantRegionStarted = false;
          TheRewriter.InsertTextBefore(b->getLocStart(), "// Fault tolerance ends here\n");
        }
        return true;
      }

      if (b->isAssignmentOp() || b->isCompoundAssignmentOp()){
        // If this is a computation statement, add checksum instructions
        for (int i = 0; i < statements.size(); i++){
          SourceLocation LineSTART = b->getLocStart();
          int line_number = SM.getExpansionLineNumber(LineSTART);
          if (line_number == statements[i]->getSourceCodeLineNumber()){
            Expr *lhs = b->getLHS();
            Expr *rhs = b->getRHS();
            vector<string> operands;
            // End location of statement
            SourceLocation END = b->getLocEnd();
            int offset = Lexer::MeasureTokenLength(END,
                pTheCompInst_g->getSourceManager(),
                pTheCompInst_g->getLangOpts()) + 1;
            SourceLocation END1 = END.getLocWithOffset(offset);
            // Start location of statement
            SourceLocation START = b->getLocStart();
            offset = Lexer::MeasureTokenLength(START,
                pTheCompInst_g->getSourceManager(),
                pTheCompInst_g->getLangOpts())-1;
            SourceLocation START1 = START.getLocWithOffset(offset);
            // Insert fault tolerance code for use_checksum
            TheRewriter.InsertText(START, "{\n", true, true);
            vector<string*>* useDataCodes = statements[i]->GetUseDataCodes();
            if (useDataCodes != NULL)
              for (int j = 0; j < useDataCodes->size(); j++)
                TheRewriter.InsertText(START, *useDataCodes->at(j), true, true);

            vector<string*> *use_counts;
            GeneralUtilityClass::PrintUnion_pw_qpolynomialCode(statements[i]->GetTargetSetCardinality(), &use_counts);

            clang::LangOptions LangOpts;
            LangOpts.CPlusPlus = true;
            clang::PrintingPolicy Policy(LangOpts);

            string TypeS;
            llvm::raw_string_ostream s(TypeS);
            lhs->printPretty(s, NULL, Policy);

            map<string,string> *polyMap;
            map<string,string>::iterator polyIt;
            GeneralUtilityClass::PrintPwQpolyInUnionPwQpoly(statements[i]->GetTargetSetCardinality(), &polyMap);
            string *strInsert = new string("\n");

            for (polyIt=polyMap->begin();polyIt!=polyMap->end();++polyIt){
              *strInsert += "if ("+ (polyIt->second) +")\n{\n";
              *strInsert +=  statements[i]->getDefMacro()+"(def_checksum, "+s.str()+","+(polyIt->first)+");\n}\n";
            }

            for (int j = 0; j < use_counts->size(); j++){
              TheRewriter.InsertText(END1,*strInsert, true, true);
            }
            TheRewriter.InsertText(END1, "}\n", true, true);
          }
        }
      }
      return true;
    }

    bool VisitStmt(Stmt *s){
      SourceManager &SM = PP.getSourceManager();
      if (SM.getFileOffset(s->getLocStart()) <= loc.start)
        return true;

      if (SM.getFileOffset(s->getLocEnd()) >= loc.end){
        if (FaultTolerantRegionStarted == true){
          FaultTolerantRegionStarted = false;
        }
        return true;
      }

      if (FaultTolerantRegionStarted == false){
        FaultTolerantRegionStarted = true;
        if (LiveDataDefChecksumCode != NULL){
          for (int i = 0; i < LiveDataDefChecksumCode->size(); i++){
            TheRewriter.InsertTextAfter(s->getLocStart(), "// Live-in data\n");
            TheRewriter.InsertTextAfter(s->getLocStart(), *(LiveDataDefChecksumCode->at(i)));
            TheRewriter.InsertTextAfter(s->getLocStart(), "\n");
          }
        }
      }

      if (isa<IfStmt>(s))
      {
        IfStmt *IfStatement = cast<IfStmt>(s);
        Stmt *Then = IfStatement->getThen();

        PresumedLoc PLoc = pTheCompInst_g->getSourceManager().getPresumedLoc(IfStatement->getLocStart(), 1);
        stringstream str_before;
        TheRewriter.InsertText(Then->getLocStart(), str_before.str(), true, true);

        SourceLocation END = Then->getLocEnd();
        int offset = Lexer::MeasureTokenLength(END, pTheCompInst_g->getSourceManager(),	pTheCompInst_g->getLangOpts()) + 1;
        SourceLocation END1 = END.getLocWithOffset(offset);

        stringstream str_after;
        TheRewriter.InsertText(END1, str_after.str(), true, true);

        Stmt *Else = IfStatement->getElse();
        if (Else){
          SourceLocation END = Else->getLocEnd();
          int offset = Lexer::MeasureTokenLength(END, pTheCompInst_g->getSourceManager(),
              pTheCompInst_g->getLangOpts()) + 1;
          SourceLocation END1 = END.getLocWithOffset(offset);
          TheRewriter.InsertText(Else->getLocStart(), "// the 'else' part\n", true, true);
        }
      }
      return true;
    }

    bool VisitFunctionDecl (FunctionDecl *f) {
      if (f->hasBody()){
        Stmt *FunBody = f->getBody();
        QualType QT = f->getResultType();
        string TypeStr = QT.getAsString();

        DeclarationName DeclName = f->getNameInfo().getName();
        string FuncName = DeclName.getAsString();

        stringstream SSBefore;
        SSBefore << "// Begin function " << FuncName << " returning " << TypeStr << "\n";
        SourceLocation ST = f->getSourceRange().getBegin();
        TheRewriter.InsertText(ST, SSBefore.str(), true, true);

        stringstream SSAfter;
        SSAfter << "\n // End function " << FuncName << "\n";
        ST = FunBody->getLocEnd().getLocWithOffset(1);
        TheRewriter.InsertText(ST, SSAfter.str(), true, true);
      }
      return true;
    }

  private:
    Rewriter &TheRewriter;
    FtLoc &loc;
    Preprocessor &PP;
    bool FaultTolerantRegionStarted;
    vector<Statement*> statements;
    vector<string*> *LiveDataDefChecksumCode;
    map<string,int> ArraySizeMap;
};

class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(Preprocessor &PP, Rewriter &R, FtLoc &l, vector<Statement*> s, vector<string*> *liveDataDefChecksumCode, map<string,int> arraySizeMap) :
      Visitor (PP, R, l, s, liveDataDefChecksumCode, arraySizeMap), loc(l), PP(PP), statements(s), LiveDataDefChecksumCode(liveDataDefChecksumCode), ArraySizeMap(arraySizeMap){
    statements = s;
  }

  virtual bool HandleTopLevelDecl (DeclGroupRef DR){
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) {
      FunctionDecl *fd = dyn_cast<clang::FunctionDecl>(*b);
      if (!fd)
        continue;
      if (!fd->hasBody())
        continue;

      SourceManager &SM = PP.getSourceManager();
      if (SM.getFileOffset(fd->getLocStart()) > loc.end)
        continue;
      if (SM.getFileOffset(fd->getLocEnd()) < loc.start)
        continue;
      Visitor.TraverseDecl(*b);
    }
    return true;
  }

private:
  MyASTVisitor Visitor;
  FtLoc &loc;
  Preprocessor &PP;
  vector<Statement*> &statements;
  vector<string*> *LiveDataDefChecksumCode;
  map<string,int> ArraySizeMap;
}; 

struct PragmaFtHandler : public PragmaHandler {
  FtLoc &loc;
	Rewriter &TheRewriter;

  PragmaFtHandler(FtLoc &loc, Rewriter &R) : PragmaHandler("scop"), loc(loc), TheRewriter(R){}

  virtual void HandlePragma(Preprocessor &PP,
                            PragmaIntroducerKind Introducer,
                            Token &ScopTok) {

      SourceManager &SM = PP.getSourceManager();
      SourceLocation sloc = ScopTok.getLocation();
      int line = SM.getExpansionLineNumber(sloc);
      sloc = SM.translateLineCol(SM.getFileID(sloc), line, 1);
      loc.start = SM.getFileOffset(sloc);

      TheRewriter.InsertTextAfter(sloc, "#include<assert.h>\n");
	    TheRewriter.InsertTextAfter(sloc, "#define add_to_checksum_n_times(c,v,n) (c) += (n)* *(long long*)(&(v))\n");
      TheRewriter.InsertTextAfter(sloc, "#define max(x,y)    ((x) > (y)? (x) : (y))\n");
      TheRewriter.InsertTextAfter(sloc, "#define min(x,y)    ((x) < (y)? (x) : (y))\n");
	    TheRewriter.InsertTextAfter(sloc, "long long def_checksum = 0;\n");
	    TheRewriter.InsertTextAfter(sloc, "long long use_checksum = 0;\n");
	    TheRewriter.InsertTextAfter(sloc, "int use_count;\n");
      }
};

struct PragmaEndftHandler : public PragmaHandler {
  FtLoc &loc;
	Rewriter &TheRewriter;
  PragmaEndftHandler(FtLoc &loc, Rewriter &R) : PragmaHandler("endscop"), loc(loc), TheRewriter(R) {}

  virtual void HandlePragma(Preprocessor &PP,
                            PragmaIntroducerKind Introducer,
                            Token &ScopTok) {

      SourceManager &SM = PP.getSourceManager();
      SourceLocation sloc = ScopTok.getLocation();
      int line = SM.getExpansionLineNumber(sloc);
      sloc = SM.translateLineCol(SM.getFileID(sloc), line, 1);
      loc.end = SM.getFileOffset(sloc);
		  TheRewriter.InsertTextBefore(sloc, "\n printf (\"def_checksum=%Ld, use_checksum=%Ld\\n\",def_checksum, use_checksum);\n");
      }
};

void WriteToFile(string outputFileName, string output) {
  ofstream ofs (outputFileName.c_str(), ofstream::out);
  ofs << output;
  ofs.close();
}

int ParseScop(char *fileName, FTReturnValues ftReturnValues, string outputFileName) {
  vector<Statement*> statements = ftReturnValues.Statements;

  CompilerInstance TheCompInst;
  pTheCompInst_g = &TheCompInst;
  TheCompInst.createDiagnostics (NULL, false);

  TargetOptions TO;
  TO.Triple = llvm::sys::getDefaultTargetTriple();
  TargetInfo *TI = TargetInfo::CreateTargetInfo( TheCompInst.getDiagnostics(), &TO);
  TheCompInst.setTarget(TI);

  TheCompInst.createFileManager();
  FileManager &FileMgr = TheCompInst.getFileManager();
  TheCompInst.createSourceManager (FileMgr);
  SourceManager &SourceMgr = TheCompInst.getSourceManager();
  TheCompInst.createPreprocessor();
  TheCompInst.createASTContext();

  Rewriter TheRewriter;
  TheRewriter.setSourceMgr(SourceMgr, TheCompInst.getLangOpts());

  const FileEntry *FileIn = FileMgr.getFile(fileName);
  SourceMgr.createMainFileID(FileIn);
  TheCompInst.getDiagnosticClient().BeginSourceFile( TheCompInst.getLangOpts(), &TheCompInst.getPreprocessor());

  Preprocessor &PP = TheCompInst.getPreprocessor();
  FtLoc loc;
  PP.AddPragmaHandler(new PragmaFtHandler(loc, TheRewriter));
  PP.AddPragmaHandler(new PragmaEndftHandler(loc, TheRewriter));

  MyASTConsumer TheConsumer(PP, TheRewriter, loc, statements, ftReturnValues.LiveDataDefChecksumCode, ftReturnValues.ArraySizeMap);

  ParseAST(PP, &TheConsumer, TheCompInst.getASTContext());
  const RewriteBuffer *RewriteBuf = TheRewriter.getRewriteBufferFor(SourceMgr.getMainFileID());

  string Output = string (RewriteBuf->begin(), RewriteBuf->end());
  WriteToFile(outputFileName, Output);

  exit(-1);
  return 0;
}
