%code requires {
  #include <memory>
  #include <string>
  #include "ast.h"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "ast.h"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
%token INT RETURN
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt Number
%type <ast_val> CompUnit

%%

// 开始符号, CompUnit ::= FuncDef
CompUnit
  : FuncDef {
    auto comp_unit = std::make_unique<CompUnitAST>(
        std::unique_ptr<FuncDefAST>(static_cast<FuncDefAST*>($1))
    );
    ast = std::move(comp_unit);
  }
  ;

// FuncDef ::= FuncType IDENT '(' ')' Block
FuncDef
  : FuncType IDENT '(' ')' Block {
    auto func_def = std::make_unique<FuncDefAST>(
        std::unique_ptr<FuncTypeAST>(static_cast<FuncTypeAST*>($1)),
        std::string(*$2),
        std::unique_ptr<BlockAST>(static_cast<BlockAST*>($5))
    );
    $$ = func_def.release();
  }
  ;

// FuncType ::= "int"
FuncType
  : INT {
    auto func_type = std::make_unique<FuncTypeAST>(string("int"));
    $$ = func_type.release();
  }
  ;

// Block ::= '{' Stmt '}'
Block
  : '{' Stmt '}' {
    auto block = std::make_unique<BlockAST>(unique_ptr<StmtAST>(static_cast<StmtAST*>($2)));
    $$ = block.release();
  }
  ;

// Stmt ::= "return" Number ';'
Stmt
  : RETURN Number ';' {
    auto stmt = std::make_unique<StmtAST>(
        std::unique_ptr<NumberAST>(static_cast<NumberAST*>($2))
    );
    $$ = stmt.release();
  }
  ;

// Number ::= INT_CONST
Number
  : INT_CONST {
    auto number = std::make_unique<NumberAST>($1);
    $$ = number.release();
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}