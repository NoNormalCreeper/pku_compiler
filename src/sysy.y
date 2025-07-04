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
  std::vector<std::unique_ptr<BaseAST>>* ast_vec_val;
}

// lexer 返回的所有 token 种类的声明
%token INT BTYPE RETURN CONST IF ELSE
%token <str_val> IDENT
%token <int_val> INT_CONST
%token <char_val> UNARY_OP
%token <char_val> MUL_OP
%token <char_val> ADD_OP
%token <str_val> REL_OP
%token <str_val> EQ_OP
%token <str_val> LAND_OP
%token <str_val> LOR_OP
%token '(' LEFT_PAREN
%token ')' RIGHT_PAREN


// 越往下定义的优先级越高
%nonassoc THEN // 为 "if-then" 规则创建一个较低的优先级
%nonassoc ELSE // 为 "else" 关键字赋予一个更高的优先级

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block BlockItem Stmt Number
%type <ast_val> Exp UnaryExp PrimaryExp CompUnit MulExp AddExp RelExp EqExp LAndExp LOrExp
%type <ast_val> Decl ConstDecl ConstDef ConstInitVal LVal ConstExp VarDecl VarDef
%type <ast_vec_val> ConstDefList BlockItemList VarDefList

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
  : BTYPE {
    auto func_type = std::make_unique<FuncTypeAST>(string("int"));
    $$ = func_type.release();
  }
  ;

// BlockItem ::= Decl | Stmt
BlockItem
  : Decl {
    // BlockItem ::= Decl;
    auto block_item = std::make_unique<BlockItemAST>(
        std::unique_ptr<DeclAST>(static_cast<DeclAST*>($1))
    );
    $$ = block_item.release();
  }
  | Stmt {
    // BlockItem ::= Stmt;
    auto block_item = std::make_unique<BlockItemAST>( 
        std::unique_ptr<StmtAST>(static_cast<StmtAST*>($1))
    );
    $$ = block_item.release();
  }
  ;

// BlockItemList ::= BlockItem | BlockItemList BlockItem
BlockItemList
  : BlockItem {
    // BlockItemList ::= BlockItem;
    auto block_item_list = new std::vector<std::unique_ptr<BaseAST>>();
    block_item_list->push_back(std::unique_ptr<BaseAST>(static_cast<BaseAST*>($1)));
    $$ = block_item_list;
  }
  | BlockItemList BlockItem {
    // BlockItemList ::= BlockItemList BlockItem;
    auto block_item_list = $1;
    block_item_list->push_back(std::unique_ptr<BaseAST>(static_cast<BaseAST*>($2)));
    $$ = block_item_list;  // 返回更新后的列表
  }
  ;

// Block ::= "{" {BlockItem} "}"; - 支持空块
Block
  : '{' '}' {
    // Block ::= "{" "}"; - 空块
    auto block = std::make_unique<BlockAST>();
    $$ = block.release();
  }
  | '{' BlockItemList '}' {
    // Block ::= "{" BlockItemList "}";
    // 需要将 BaseAST* 转换为 BlockItemAST*
    std::vector<std::unique_ptr<BlockItemAST>> block_items;
    auto base_list = $2;
    for (auto& item : *base_list) {
      block_items.push_back(std::unique_ptr<BlockItemAST>(static_cast<BlockItemAST*>(item.release())));
    }
    delete base_list;
    auto block = std::make_unique<BlockAST>(std::move(block_items));
    $$ = block.release();
  }
  ;


// Stmt ::= LVal "=" Exp ";"
//        | [Exp] ";"
//        | Block
//        | "return" [Exp] ";";
Stmt
  : LVal '=' Exp ';'
  {
    // Stmt ::= LVal "=" Exp ";"
    auto lval_eq_exp_stmt = std::make_unique<LValEqExpStmtAST>(
      std::unique_ptr<LValAST>(static_cast<LValAST*>($1)),
      std::unique_ptr<ExpAST>(static_cast<ExpAST*>($3))
    );
    auto stmt = std::make_unique<StmtAST>(
      std::move(lval_eq_exp_stmt)
    );
    $$ = stmt.release();
  }
  | RETURN Exp ';'
  {
    // Stmt ::= "return" Exp ";"
    auto return_exp_stmt = std::make_unique<ReturnExpStmtAST>(
      std::optional(std::unique_ptr<ExpAST>(static_cast<ExpAST*>($2)))
    );
    auto stmt = std::make_unique<StmtAST>(
      std::move(return_exp_stmt)
    );
    $$ = stmt.release();
  }
  | RETURN ';'
  {
    // Stmt ::= "return" ";"
    auto return_exp_stmt = std::make_unique<ReturnExpStmtAST>(
      std::nullopt  // 没有表达式
    );
    auto stmt = std::make_unique<StmtAST>(
      std::move(return_exp_stmt)
    );
    $$ = stmt.release();
  }
  | Block
  {
    // Stmt ::= Block;
    auto block_stmt = std::make_unique<BlockStmtAST>(
      std::unique_ptr<BlockAST>(static_cast<BlockAST*>($1))
    );
    auto stmt = std::make_unique<StmtAST>(
      std::move(block_stmt)
    );
    $$ = stmt.release();
  }
  | Exp ';'
  {
    // Stmt ::= Exp ";"
    auto exp_stmt = std::make_unique<OptionalExpStmtAST>(
      std::unique_ptr<ExpAST>(static_cast<ExpAST*>($1))
    );
    auto stmt = std::make_unique<StmtAST>(
      std::move(exp_stmt)
    );
    $$ = stmt.release();
  }
  | ';'
  {
    // Stmt ::= ";"
    auto empty_stmt = std::make_unique<OptionalExpStmtAST>();
    auto stmt = std::make_unique<StmtAST>(
      std::move(empty_stmt)
    );
    $$ = stmt.release();
  }
  | IF '(' Exp ')' Stmt %prec THEN  // 最低优先级
  {
    // Stmt ::= IF '(' Exp ')' Stmt;
    auto if_else_stmt = std::make_unique<IfElseStmtAST>(
      std::unique_ptr<ExpAST>(static_cast<ExpAST*>($3)),
      std::unique_ptr<StmtAST>(static_cast<StmtAST*>($5)),
      std::nullopt  // 没有 else 分支
    );
    auto stmt = std::make_unique<StmtAST>(
      std::move(if_else_stmt)
    );
    $$ = stmt.release();
  }
  | IF '(' Exp ')' Stmt ELSE Stmt
  {
    // Stmt ::= IF '(' Exp ')' Stmt ELSE Stmt;
    auto if_else_stmt = std::make_unique<IfElseStmtAST>(
      std::unique_ptr<ExpAST>(static_cast<ExpAST*>($3)),
      std::unique_ptr<StmtAST>(static_cast<StmtAST*>($5)),
      std::unique_ptr<StmtAST>(static_cast<StmtAST*>($7))
    );
    auto stmt = std::make_unique<StmtAST>(
      std::move(if_else_stmt)
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

// Exp ::= LOrExp
Exp : LOrExp
  {
    auto exp = std::make_unique<ExpAST>(
      std::unique_ptr<LOrExpAST>(static_cast<LOrExpAST*>($1))
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
  | LVal
  {
    // PrimaryExp ::= LVal; (LVal ::= IDENT)
    auto lval_ptr = static_cast<LValAST*>($1);
    auto primary_exp = std::make_unique<PrimaryExpAST>(
      std::unique_ptr<LValAST>(new LValAST(lval_ptr->ident))  // 创建 LValAST 对象的副本
    );
    delete lval_ptr;  // 清理原来的对象
    $$ = primary_exp.release();
  }
  ;

// MulExp ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
MulExp: UnaryExp
  {
    // MulExp ::= UnaryExp
    auto mul_exp = std::make_unique<MulExpAST>(
      std::unique_ptr<UnaryExpAST>(static_cast<UnaryExpAST*>($1))
    );
    $$ = mul_exp.release();
  }
  | MulExp MUL_OP UnaryExp
  {
    // MulExp ::= MulExp ("*" | "/" | "%") UnaryExp;
    MulOp op;
    switch ($2) {
      case '*': op = MUL_OP_MUL; break;
      case '/': op = MUL_OP_DIV; break;
      case '%': op = MUL_OP_MOD; break;
      default: op = MUL_OP_MUL; break;
    }

    auto mul_exp_op_and_exp = std::make_unique<MulExpOpAndExpAST>(
      op,
      std::unique_ptr<MulExpAST>(static_cast<MulExpAST*>($1)),
      std::unique_ptr<UnaryExpAST>(static_cast<UnaryExpAST*>($3))
    );
    
    auto mul_exp = std::make_unique<MulExpAST>(std::move(mul_exp_op_and_exp));
    $$ = mul_exp.release();
  }
  ;

// AddExp ::= MulExp | AddExp ("+" | "-") MulExp;
AddExp: MulExp
  {
    // AddExp ::= MulExp
    auto add_exp = std::make_unique<AddExpAST>(
      std::unique_ptr<MulExpAST>(static_cast<MulExpAST*>($1))
    );
    $$ = add_exp.release();
  }
  | AddExp ADD_OP MulExp
  {
    // AddExp ::= AddExp ("+" | "-") MulExp;
    AddOp op;
    switch ($2) {
      case '+': op = ADD_OP_ADD; break;
      case '-': op = ADD_OP_SUB; break;
      default: op = ADD_OP_ADD; break;
    }

    auto add_exp_op_and_exp = std::make_unique<AddExpOpAndMulExpAST>(
      op,
      std::unique_ptr<AddExpAST>(static_cast<AddExpAST*>($1)),
      std::unique_ptr<MulExpAST>(static_cast<MulExpAST*>($3))
    );
    
    auto add_exp = std::make_unique<AddExpAST>(std::move(add_exp_op_and_exp));
    $$ = add_exp.release();
  }
  | AddExp UNARY_OP MulExp // 疑似加减号会被识别为 UNARY_OP
    {
    // AddExp ::= AddExp ("+" | "-") MulExp;
    AddOp op;
    switch ($2) {
      case '+': op = ADD_OP_ADD; break;
      case '-': op = ADD_OP_SUB; break;
      default: op = ADD_OP_ADD; break;
    }

    auto add_exp_op_and_exp = std::make_unique<AddExpOpAndMulExpAST>(
      op,
      std::unique_ptr<AddExpAST>(static_cast<AddExpAST*>($1)),
      std::unique_ptr<MulExpAST>(static_cast<MulExpAST*>($3))
    );
    
    auto add_exp = std::make_unique<AddExpAST>(std::move(add_exp_op_and_exp));
    $$ = add_exp.release();
  }
  ;

// RelExp ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
RelExp: AddExp
  {
    // RelExp ::= AddExp
    auto rel_exp = std::make_unique<RelExpAST>(
      std::unique_ptr<AddExpAST>(static_cast<AddExpAST*>($1))
    );
    $$ = rel_exp.release();
  }
  | RelExp REL_OP AddExp
  {
    // RelExp ::= RelExp ("<" | ">" | "<=" | ">=") AddExp;
    RelOp op;
    if (*$2 == "<") {
      op = REL_OP_LT;
    } else if (*$2 == ">") {
      op = REL_OP_GT;
    } else if (*$2 == "<=") {
      op = REL_OP_LE;
    } else if (*$2 == ">=") {
      op = REL_OP_GE;
    } else {
      op = REL_OP_LT; // 默认情况
    }

    auto rel_exp_op_and_exp = std::make_unique<RelExpOpAndAddExpAST>(
      op,
      std::unique_ptr<RelExpAST>(static_cast<RelExpAST*>($1)),
      std::unique_ptr<AddExpAST>(static_cast<AddExpAST*>($3))
    );
    
    auto rel_exp = std::make_unique<RelExpAST>(std::move(rel_exp_op_and_exp));
    $$ = rel_exp.release();
  }
  ;

// EqExp ::= RelExp | EqExp ("==" | "!=") RelExp;
EqExp: RelExp
  {
    // EqExp ::= RelExp
    auto eq_exp = std::make_unique<EqExpAST>(
      std::unique_ptr<RelExpAST>(static_cast<RelExpAST*>($1))
    );
    $$ = eq_exp.release();
  }
  | EqExp EQ_OP RelExp
  {
    // EqExp ::= EqExp ("==" | "!=") RelExp;
    EqOp op;
    if (*$2 == "==") {
      op = EQ_OP_EQ;
    } else if (*$2 == "!=") {
      op = EQ_OP_NE;
    } else {
      op = EQ_OP_EQ; // 默认情况
    }

    auto eq_exp_op_and_exp = std::make_unique<EqExpOpAndRelExpAST>(
      op,
      std::unique_ptr<EqExpAST>(static_cast<EqExpAST*>($1)),
      std::unique_ptr<RelExpAST>(static_cast<RelExpAST*>($3))
    );
    
    auto eq_exp = std::make_unique<EqExpAST>(std::move(eq_exp_op_and_exp));
    $$ = eq_exp.release();
  }
  ;

// LAndExp ::= EqExp | LAndExp "&&" EqExp;
LAndExp: EqExp
  {
    // LAndExp ::= EqExp
    auto land_exp = std::make_unique<LAndExpAST>(
      std::unique_ptr<EqExpAST>(static_cast<EqExpAST*>($1))
    );
    $$ = land_exp.release();
  }
  | LAndExp LAND_OP EqExp
  {
    // LAndExp ::= LAndExp "&&" EqExp;
    auto land_exp_op_and_exp = std::make_unique<LAndExpOpAndEqExpAST>(
      std::unique_ptr<LAndExpAST>(static_cast<LAndExpAST*>($1)),
      std::unique_ptr<EqExpAST>(static_cast<EqExpAST*>($3))
    );
    
    auto land_exp = std::make_unique<LAndExpAST>(std::move(land_exp_op_and_exp));
    $$ = land_exp.release();
  }
  ;

// LOrExp ::= LAndExp | LOrExp "||" LAndExp;
LOrExp: LAndExp
  {
    // LOrExp ::= LAndExp
    auto lor_exp = std::make_unique<LOrExpAST>(
      std::unique_ptr<LAndExpAST>(static_cast<LAndExpAST*>($1))
    );
    $$ = lor_exp.release();
  }
  | LOrExp LOR_OP LAndExp
  {
    // LOrExp ::= LOrExp "||" LAndExp;
    auto lor_exp_op_and_exp = std::make_unique<LOrExpOpAndLAndExpAST>(
      std::unique_ptr<LOrExpAST>(static_cast<LOrExpAST*>($1)),
      std::unique_ptr<LAndExpAST>(static_cast<LAndExpAST*>($3))
    );
    
    auto lor_exp = std::make_unique<LOrExpAST>(std::move(lor_exp_op_and_exp));
    $$ = lor_exp.release();
  }
  ;

// ConstInitVal ::= ConstExp;
ConstInitVal: ConstExp
  {
    auto const_init_val = std::make_unique<ConstInitValAST>(
      std::unique_ptr<ConstExpAST>(static_cast<ConstExpAST*>($1))
    );
    $$ = const_init_val.release();
  }
  ;

// ConstExp ::= Exp;
ConstExp: Exp
  {
    auto const_exp = std::make_unique<ConstExpAST>(
      std::unique_ptr<ExpAST>(static_cast<ExpAST*>($1))
    );
    $$ = const_exp.release();
  }
  ;

// ConstDef ::= IDENT "=" ConstInitVal;
ConstDef: IDENT '=' ConstInitVal
  {
    auto const_def = std::make_unique<ConstDefAST>(
      std::string(*$1),
      std::unique_ptr<ConstInitValAST>(static_cast<ConstInitValAST*>($3))
    );
    delete $1;  // 清理 IDENT 的内存
    $$ = const_def.release();
  }
  ;

// ConstDefList ::= ConstDef | ConstDefList ',' ConstDef;
ConstDefList: ConstDef
  {
    // ConstDefList ::= ConstDef;
    auto const_def_list = new std::vector<std::unique_ptr<BaseAST>>();
    const_def_list->push_back(std::unique_ptr<BaseAST>(static_cast<BaseAST*>($1)));
    $$ = const_def_list;
  }
  | ConstDefList ',' ConstDef
  {
    // ConstDefList ::= ConstDefList ConstDef;
    auto const_def_list = $1;
    const_def_list->push_back(std::unique_ptr<BaseAST>(static_cast<BaseAST*>($3)));
    $$ = const_def_list;  // 返回更新后的列表
  }


// ConstDecl ::= "const" BType ConstDef {"," ConstDef} ";";
ConstDecl
  : CONST BTYPE ConstDefList ';'
  {
    // ConstDecl ::= "const" BType ConstDefList ";";
    // 需要将 BaseAST* 转换为 ConstDefAST*
    std::vector<std::unique_ptr<ConstDefAST>> const_defs;
    auto base_list = $3;
    for (auto& item : *base_list) {
      const_defs.push_back(std::unique_ptr<ConstDefAST>(static_cast<ConstDefAST*>(item.release())));
    }
    delete base_list;
    auto const_decl = std::make_unique<ConstDeclAST>(
      BT_INT,  // BType 暂时只能为 "int"
      std::move(const_defs)  // 移动 ConstDefList
    );
    $$ = const_decl.release();
  }
  ;

// Decl ::= ConstDecl | VarDecl;
//
Decl
  : ConstDecl
  {
    // Decl ::= ConstDecl;
    auto decl = std::make_unique<DeclAST>(
      std::unique_ptr<ConstDeclAST>(static_cast<ConstDeclAST*>($1))
    );
    $$ = decl.release();
  }
  | VarDecl
  {
    // Decl ::= VarDecl;
    auto decl = std::make_unique<DeclAST>(
      std::unique_ptr<VarDeclAST>(static_cast<VarDeclAST*>($1))
    );
    $$ = decl.release();
  }
  ;

// LVal ::= IDENT;
LVal
  : IDENT
  {
    auto lval = std::make_unique<LValAST>(std::string(*$1));
    delete $1;  // 清理 IDENT 的内存
    $$ = lval.release();
  }
  ;

// VarDef ::= IDENT | IDENT "=" InitVal;
VarDef
  : IDENT
  {
    // VarDef ::= IDENT;
    auto var_def = std::make_unique<VarDefAST>(std::string(*$1));
    delete $1;  // 清理 IDENT 的内存
    $$ = var_def.release();
  }
  | IDENT '=' ConstInitVal
  {
    // VarDef ::= IDENT "=" ConstInitVal;
    auto var_def = std::make_unique<VarDefAST>(
      std::string(*$1),
      std::unique_ptr<ConstInitValAST>(static_cast<ConstInitValAST*>($3))
    );
    delete $1;  // 清理 IDENT 的内存
    $$ = var_def.release();
  }
  ;

VarDefList
  : VarDef
  {
    // VarDefList ::= VarDef;
    auto var_def_list = new std::vector<std::unique_ptr<BaseAST>>();
    var_def_list->push_back(std::unique_ptr<BaseAST>(static_cast<BaseAST*>($1)));
    $$ = var_def_list;
  }
  | VarDefList ',' VarDef
  {
    // VarDefList ::= VarDefList ',' VarDef;
    auto var_def_list = $1;
    var_def_list->push_back(std::unique_ptr<BaseAST>(static_cast<BaseAST*>($3)));
    $$ = var_def_list;  // 返回更新后的列表
  }
  ;

// VarDecl ::= BType VarDef {"," VarDef} ";";
VarDecl
  : BTYPE VarDefList ';'
  {
    // VarDecl ::= BType VarDefList ";";
    // 需要将 BaseAST* 转换为 VarDefAST*
    std::vector<std::unique_ptr<VarDefAST>> var_defs;
    auto base_list = $2;
    for (auto& item : *base_list) {
      var_defs.push_back(std::unique_ptr<VarDefAST>(static_cast<VarDefAST*>(item.release())));
    }
    delete base_list;
    auto var_decl = std::make_unique<VarDeclAST>(
      BT_INT,  // BType 暂时只能为 "int"
      std::move(var_defs)  // 移动 VarDefList
    );
    $$ = var_decl.release();
  }
  ;




%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}