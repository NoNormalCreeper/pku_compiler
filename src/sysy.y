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
  char char_val;
  BaseAST* ast_val;
}

// lexer 返回的所有 token 种类的声明
%token INT RETURN
%token <str_val> IDENT
%token <int_val> INT_CONST
%token <char_val> UNARY_OP
%token <char_val> MUL_OP
%token <char_val> ADD_OP
%token '(' LEFT_PAREN
%token ')' RIGHT_PAREN

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt Number Exp UnaryExp PrimaryExp
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
    delete $2;  // 清理 IDENT 的内存
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

// Stmt ::= "return" Exp ';'
Stmt
  : RETURN Exp ';' {
    auto stmt = std::make_unique<StmtAST>(
        std::unique_ptr<ExpAST>(static_cast<ExpAST*>($2))
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

// Exp ::= UnaryExp
Exp : UnaryExp
  {
    auto exp = std::make_unique<ExpAST>(
      std::unique_ptr<UnaryExpAST>(static_cast<UnaryExpAST*>($1))
    );
    $$ = exp.release();
  }
  ;

UnaryExp : PrimaryExp
  {
    // UnaryExp ::= PrimaryExp
    auto unary_exp = std::make_unique<UnaryExpAST>(
      std::unique_ptr<PrimaryExpAST>(static_cast<PrimaryExpAST*>($1))
    );
    $$ = unary_exp.release();
  }
  | UNARY_OP UnaryExp
  {
    // UnaryExp ::= UnaryOp UnaryExp;
    UnaryOp op;
    switch ($1) {
      case '+': op = UNARY_OP_POSITIVE; break;
      case '-': op = UNARY_OP_NEGATIVE; break;
      case '!': op = UNARY_OP_NOT; break;
      default: op = UNARY_OP_POSITIVE; break;
    }

    auto unary_exp_op_and_exp = std::make_unique<UnaryExpOpAndExpAST>(
      op,
      std::unique_ptr<UnaryExpAST>(static_cast<UnaryExpAST*>($2))
    );
    
    auto unary_exp = std::make_unique<UnaryExpAST>(std::move(unary_exp_op_and_exp));
    $$ = unary_exp.release();
  }
  ;


PrimaryExp : '(' Exp ')'
  {
    // PrimaryExp ::= "(" Exp ")"
    auto primary_exp = std::make_unique<PrimaryExpAST>(
      std::unique_ptr<ExpAST>(static_cast<ExpAST*>($2))
    );
    $$ = primary_exp.release();
  }
  | Number
  {
    // PrimaryExp ::= Number; (Number ::= INT_CONST)
    NumberAST* number_ptr = static_cast<NumberAST*>($1);
    auto primary_exp = std::make_unique<PrimaryExpAST>(
      NumberAST(number_ptr->value)  // 创建 NumberAST 对象的副本
    );
    delete number_ptr;  // 清理原来的对象
    $$ = primary_exp.release();
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}