#pragma once

#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "string_format.h"
#include "symbol_table.h"

// Forward declarations for all AST node classes
class BaseAST;
class CompUnitAST;
class FuncDefAST;
class FuncTypeAST;
class BlockAST;
class BlockItemAST;
class OptionalExpStmtAST;
class BlockStmtAST;
class StmtAST;
class LValEqExpStmtAST;
class ReturnExpStmtAST;
class IfElseStmtAST;
class WhileStmtAST;
class BreakStmtAST;
class ContinueStmtAST;
class NumberAST;

class ExpAST;
class PrimaryExpAST;
class UnaryExpOpAndExpAST;
class UnaryExpAST;
class AddExpAST;
class AddExpOpAndMulExpAST;
class MulExpAST;
class MulExpOpAndExpAST;
class RelExpAST;
class RelExpOpAndAddExpAST;
class EqExpAST;
class EqExpOpAndRelExpAST;
class LAndExpAST;
class LAndExpOpAndEqExpAST;
class LOrExpAST;
class LOrExpOpAndLAndExpAST;

class DeclAST;
class ConstDeclAST;
class ConstDefAST;
class ConstInitValAST;
class ConstExpAST;
class LValAST;
class VarDeclAST;
class VarDefAST;


enum UnaryOp : std::uint8_t {
    UNARY_OP_POSITIVE, // +
    UNARY_OP_NEGATIVE, // -
    UNARY_OP_NOT, // !
};

enum MulOp : std::uint8_t {
    MUL_OP_MUL, // *
    MUL_OP_DIV, // /
    MUL_OP_MOD, // %
};

enum AddOp : std::uint8_t {
    ADD_OP_ADD, // +
    ADD_OP_SUB, // -
};

enum RelOp : std::uint8_t {
    REL_OP_LT, // <
    REL_OP_LE, // <=
    REL_OP_GT, // >
    REL_OP_GE, // >=
};

enum EqOp : std::uint8_t {
    EQ_OP_EQ, // ==
    EQ_OP_NE, // !=
};

enum BType : std::uint8_t {
    BT_INT, // int
};

// LAndOp 和 LOrOp 不需要枚举，因为它们没有额外的操作符


// 所有 AST 的基类
class BaseAST {
public:
    virtual ~BaseAST() = default;

    virtual void Dump() const;
    virtual std::string toKoopa() const;

    // 添加常量求值方法 - 如果表达式是常量则返回其值，否则返回nullopt
    virtual std::optional<int> evaluateConstant(SymbolTable& symbol_table) const;

    // 全局符号表引用（在main中初始化）
    static SymbolTable* global_symbol_table;

    static int getNewTempVar() {
        static int temp_var_count = 0;
        return temp_var_count++;
    }

    // 重置临时变量计数器（在每个函数开始时调用）
    static void resetTempVarCounter() {
        static int temp_var_count = 0;
        temp_var_count = 0;
    }
};

// Number
class NumberAST : public BaseAST {
public:
    int value;

    explicit NumberAST(int val);

    void Dump() const override;
    std::string toKoopa() const override;
    std::optional<int> evaluateConstant(SymbolTable& symbol_table) const override;
};

// FuncType
class FuncTypeAST : public BaseAST {
public:
    std::string type_name; // "int"
    std::string ident; // "main"

    FuncTypeAST(const std::string& name);

    void Dump() const override;
    std::string toKoopa() const override;

private:
    std::map<std::string, std::string> koopa_type_map = {
        { "int", "i32" },
        { "void", "void" },
    };
};

class StmtAST : public BaseAST {
public:
    std::variant<std::unique_ptr<LValEqExpStmtAST>, std::unique_ptr<ReturnExpStmtAST>,
        std::unique_ptr<OptionalExpStmtAST>, std::unique_ptr<BlockStmtAST>, 
        std::unique_ptr<IfElseStmtAST>, std::unique_ptr<WhileStmtAST>,
        std::unique_ptr<BreakStmtAST>, std::unique_ptr<ContinueStmtAST>>
        statement;

    StmtAST(std::unique_ptr<LValEqExpStmtAST> lval_eq_exp_stmt)
        : statement(std::move(lval_eq_exp_stmt)) {}
    StmtAST(std::unique_ptr<ReturnExpStmtAST> return_exp_stmt)
        : statement(std::move(return_exp_stmt)) {}
    StmtAST(std::unique_ptr<OptionalExpStmtAST> optional_exp_stmt)
        : statement(std::move(optional_exp_stmt)) {}
    StmtAST(std::unique_ptr<BlockStmtAST> block_stmt)
        : statement(std::move(block_stmt)) {}
    StmtAST(std::unique_ptr<IfElseStmtAST> if_else_stmt)
        : statement(std::move(if_else_stmt)) {}
    StmtAST(std::unique_ptr<WhileStmtAST> while_stmt)
        : statement(std::move(while_stmt)) {}
    StmtAST(std::unique_ptr<BreakStmtAST> break_stmt)
        : statement(std::move(break_stmt)) {}
    StmtAST(std::unique_ptr<ContinueStmtAST> continue_stmt)
        : statement(std::move(continue_stmt)) {}
    
    void Dump() const override;
    std::string toKoopa() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const;
};

class BreakStmtAST : public BaseAST {
public:
    std::optional<int> loop_id; // 循环 ID，用于生成唯一的标签

    BreakStmtAST(std::optional<int> loop_id = std::nullopt)
        : loop_id(loop_id) {}

    void Dump() const override {
        std::cout << "BreakStmtAST { break; }";
    }

    std::string toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table);
};

class ContinueStmtAST : public BaseAST {
public:
    std::optional<int> loop_id;

    ContinueStmtAST(std::optional<int> loop_id = std::nullopt)
        : loop_id(loop_id) {}

    void Dump() const override {
        std::cout << "ContinueStmtAST { continue; }";
    }

    std::string toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table);
};

class WhileStmtAST : public BaseAST {
public:
    std::unique_ptr<ExpAST> condition; // 循环条件表达式
    std::unique_ptr<StmtAST> body; // 循环体
    std::optional<int> loop_id; // 循环 ID，用于生成唯一的标签

    WhileStmtAST(std::unique_ptr<ExpAST> cond, std::unique_ptr<StmtAST> body_stmt)
        : condition(std::move(cond)), body(std::move(body_stmt)), loop_id(getNewTempVar()) {}
    
    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table);
    void setBodyLoopIds(int loop_id) const;

private:
    // 递归辅助函数，用于遍历所有语句类型并设置 loop_id
    void setStmtLoopIds(StmtAST* stmt, int loop_id) const;
};

class LValEqExpStmtAST : public BaseAST {
public:
    std::unique_ptr<LValAST> lval; // 左值
    std::unique_ptr<ExpAST> expression; // 右值表达式

    LValEqExpStmtAST(std::unique_ptr<LValAST> lval, std::unique_ptr<ExpAST> exp)
        : lval(std::move(lval)), expression(std::move(exp)) {}
    
    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const;
};

class OptionalExpStmtAST : public BaseAST {
public:
    std::optional<std::unique_ptr<ExpAST>> expression; // 可选的表达式

    OptionalExpStmtAST(std::unique_ptr<ExpAST> exp)
        : expression(std::move(exp)) {}
    OptionalExpStmtAST(): expression(std::nullopt) {}
    
    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const;
};

class BlockStmtAST : public BaseAST {
public:
    std::unique_ptr<BlockAST> block; // 块内容

    explicit BlockStmtAST(std::unique_ptr<BlockAST> blk)
        : block(std::move(blk)) {}
    
    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const;
};

// Stmt
class ReturnExpStmtAST : public BaseAST {
public:
    // Stmt -> "return" Exp ";"
    // std::unique_ptr<NumberAST> number;
    std::optional<std::unique_ptr<ExpAST>> expression;

    std::vector<std::string> generated_instructions; // 存储中间过程用于计算的 IR 指令

    ReturnExpStmtAST(std::optional<std::unique_ptr<ExpAST>> exp);

    void Dump() const override;
    std::string toKoopa() const override;
    std::string toKoopa() ;
};

class IfElseStmtAST : public BaseAST {
public:
    std::unique_ptr<ExpAST> condition; // 条件表达式
    std::unique_ptr<StmtAST> then_stmt; // if 分支语句
    std::optional<std::unique_ptr<StmtAST>> else_stmt; // 可选的 else 分支语句

    IfElseStmtAST(std::unique_ptr<ExpAST> cond, std::unique_ptr<StmtAST> then_stmt,
        std::optional<std::unique_ptr<StmtAST>> else_stmt = std::nullopt)
        : condition(std::move(cond))
        , then_stmt(std::move(then_stmt))
        , else_stmt(std::move(else_stmt))
    {}

    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const;
};

// Block
class BlockAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BlockItemAST>> block_items; // BlockItem 列表

    explicit BlockAST(std::vector<std::unique_ptr<BlockItemAST>> items)
        : block_items(std::move(items)) {}
    BlockAST() = default;

    void Dump() const override;
    std::string toKoopa() const override;
    
    // 处理块中的声明，维护符号表
    std::string toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const;
};

class BlockItemAST : public BaseAST {
public:
    // BlockItem 可以是声明或语句
    std::variant<std::unique_ptr<DeclAST>, std::unique_ptr<StmtAST>> item;

    explicit BlockItemAST(std::unique_ptr<DeclAST> decl)
        : item(std::move(decl)) {}
    explicit BlockItemAST(std::unique_ptr<StmtAST> stmt)
        : item(std::move(stmt)) {}

    void Dump() const override;
    
    // 处理BlockItem，维护符号表并生成IR
    std::string toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const;
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<FuncTypeAST> func_type;
    std::string ident;
    std::unique_ptr<BlockAST> block;

    FuncDefAST(std::unique_ptr<FuncTypeAST> type, const std::string& id, std::unique_ptr<BlockAST> blk);

    void Dump() const override;
    // std::string toKoopa() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions) const;

private:
    // Helper function to remove duplicate return statements in basic blocks
    void removeDuplicateReturns(std::vector<std::string>& instructions) const;
    void removeUnreachableInstructions(std::vector<std::string>& instructions) const;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
    // 用智能指针管理对象
    std::unique_ptr<FuncDefAST> func_def;

    CompUnitAST(std::unique_ptr<FuncDefAST> func);

    void Dump() const override;
    std::string toKoopa() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions) const;
};

/// 一元表达式

class PrimaryExpAST : public BaseAST {
public:
    std::variant<std::unique_ptr<ExpAST>, NumberAST, std::unique_ptr<LValAST>> expression;

    explicit PrimaryExpAST(std::unique_ptr<ExpAST> exp)
        : expression(std::move(exp)) {}
    explicit PrimaryExpAST(NumberAST number)
        : expression(std::move(number)) {}
    explicit PrimaryExpAST(std::unique_ptr<LValAST> lval)
        : expression(std::move(lval)) {}

    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
    std::optional<int> evaluateConstant(SymbolTable& symbol_table) const override;
};

class UnaryExpAST : public BaseAST {
public:
    std::variant<std::unique_ptr<PrimaryExpAST>, std::unique_ptr<UnaryExpOpAndExpAST>> expression;

    explicit UnaryExpAST(std::unique_ptr<PrimaryExpAST> primary_exp)
        : expression(std::move(primary_exp)) {}
    explicit UnaryExpAST(std::unique_ptr<UnaryExpOpAndExpAST> unary_exp_op_and_exp)
        : expression(std::move(unary_exp_op_and_exp)) {}
    
    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
    std::optional<int> evaluateConstant(SymbolTable& symbol_table) const override;
};

// UnaryOp UnaryExp
class UnaryExpOpAndExpAST : public BaseAST {
public:
    UnaryOp op; // 一元运算符
    std::unique_ptr<UnaryExpAST> latter_expression;

    UnaryExpOpAndExpAST(UnaryOp operation, std::unique_ptr<UnaryExpAST> exp)
        : op(operation), latter_expression(std::move(exp)) {}
    
    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
    std::optional<int> evaluateConstant(SymbolTable& symbol_table) const override;
};

class ExpAST : public BaseAST {
public:
    std::unique_ptr<LOrExpAST> expression;

    explicit ExpAST(std::unique_ptr<LOrExpAST> add_exp)
        : expression(std::move(add_exp)) {}
    
    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
    std::optional<int> evaluateConstant(SymbolTable& symbol_table) const override;
};

class MulExpOpAndExpAST : public BaseAST {
public:
    MulOp op; // 乘法运算符
    std::unique_ptr<MulExpAST> first_expression;
    std::unique_ptr<UnaryExpAST> latter_expression;

    MulExpOpAndExpAST(MulOp operation, std::unique_ptr<MulExpAST> first_exp, std::unique_ptr<UnaryExpAST> latter_exp)
        : op(operation), first_expression(std::move(first_exp)), latter_expression(std::move(latter_exp)) {}

    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
    std::optional<int> evaluateConstant(SymbolTable& symbol_table) const override;
};

class MulExpAST : public BaseAST {
public:
    std::variant<std::unique_ptr<UnaryExpAST>, std::unique_ptr<MulExpOpAndExpAST>> expression;
    
    explicit MulExpAST(std::unique_ptr<UnaryExpAST> unary_exp)
        : expression(std::move(unary_exp)) {}
    explicit MulExpAST(std::unique_ptr<MulExpOpAndExpAST> mul_exp_op_and_exp)
        : expression(std::move(mul_exp_op_and_exp)) {}

    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
    std::optional<int> evaluateConstant(SymbolTable& symbol_table) const override;
};

class AddExpOpAndMulExpAST : public BaseAST {
public:
    AddOp op; // 加法运算符
    std::unique_ptr<AddExpAST> first_expression;
    std::unique_ptr<MulExpAST> latter_expression;

    AddExpOpAndMulExpAST(AddOp operation, std::unique_ptr<AddExpAST> first_exp, std::unique_ptr<MulExpAST> latter_exp)
        : op(operation), first_expression(std::move(first_exp)), latter_expression(std::move(latter_exp)) {}

    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
    std::optional<int> evaluateConstant(SymbolTable& symbol_table) const override;
};

class AddExpAST : public BaseAST {
public:
    std::variant<std::unique_ptr<MulExpAST>, std::unique_ptr<AddExpOpAndMulExpAST>> expression;

    explicit AddExpAST(std::unique_ptr<MulExpAST> mul_exp)
        : expression(std::move(mul_exp)) {}
    explicit AddExpAST(std::unique_ptr<AddExpOpAndMulExpAST> add_exp_op_and_mul_exp)
        : expression(std::move(add_exp_op_and_mul_exp)) {}
    
    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
    std::optional<int> evaluateConstant(SymbolTable& symbol_table) const override;
};

class RelExpOpAndAddExpAST : public BaseAST {
public:
    RelOp op; // 关系运算符
    std::unique_ptr<RelExpAST> first_expression;
    std::unique_ptr<AddExpAST> latter_expression;

    RelExpOpAndAddExpAST(RelOp operation, std::unique_ptr<RelExpAST> first_exp, std::unique_ptr<AddExpAST> latter_exp)
        : op(operation), first_expression(std::move(first_exp)), latter_expression(std::move(latter_exp)) {}

    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
    std::optional<int> evaluateConstant(SymbolTable& symbol_table) const override;
};

class RelExpAST : public BaseAST {
public:
    std::variant<std::unique_ptr<AddExpAST>, std::unique_ptr<RelExpOpAndAddExpAST>> expression;

    explicit RelExpAST(std::unique_ptr<AddExpAST> add_exp)
        : expression(std::move(add_exp)) {}
    explicit RelExpAST(std::unique_ptr<RelExpOpAndAddExpAST> rel_exp)
        : expression(std::move(rel_exp)) {}
    
    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
    std::optional<int> evaluateConstant(SymbolTable& symbol_table) const override;
};

class EqExpOpAndRelExpAST : public BaseAST {
public:
    EqOp op; // 相等运算符
    std::unique_ptr<EqExpAST> first_expression;
    std::unique_ptr<RelExpAST> latter_expression;

    EqExpOpAndRelExpAST(EqOp operation, std::unique_ptr<EqExpAST> first_exp, std::unique_ptr<RelExpAST> latter_exp)
        : op(operation), first_expression(std::move(first_exp)), latter_expression(std::move(latter_exp)) {}
    
    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
    std::optional<int> evaluateConstant(SymbolTable& symbol_table) const override;
};

class EqExpAST : public BaseAST {
public:
    std::variant<std::unique_ptr<RelExpAST>, std::unique_ptr<EqExpOpAndRelExpAST>> expression;

    explicit EqExpAST(std::unique_ptr<RelExpAST> rel_exp)
        : expression(std::move(rel_exp)) {}
    explicit EqExpAST(std::unique_ptr<EqExpOpAndRelExpAST> eq_exp_op_and_rel_exp)
        : expression(std::move(eq_exp_op_and_rel_exp)) {}
    
    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
    std::optional<int> evaluateConstant(SymbolTable& symbol_table) const override;
};

class LAndExpOpAndEqExpAST : public BaseAST {
public:
    std::unique_ptr<LAndExpAST> first_expression;
    std::unique_ptr<EqExpAST> latter_expression;

    LAndExpOpAndEqExpAST(std::unique_ptr<LAndExpAST> first_exp, std::unique_ptr<EqExpAST> latter_exp)
        : first_expression(std::move(first_exp)), latter_expression(std::move(latter_exp)) {}
    
    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
    std::optional<int> evaluateConstant(SymbolTable& symbol_table) const override;
};

class LAndExpAST : public BaseAST {
public:
    std::variant<std::unique_ptr<EqExpAST>, std::unique_ptr<LAndExpOpAndEqExpAST>> expression;

    explicit LAndExpAST(std::unique_ptr<EqExpAST> eq_exp)
        : expression(std::move(eq_exp)) {}
    explicit LAndExpAST(std::unique_ptr<LAndExpOpAndEqExpAST> land_exp_op_and_eq_exp)
        : expression(std::move(land_exp_op_and_eq_exp)) {}

    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
    std::optional<int> evaluateConstant(SymbolTable& symbol_table) const override;
};

class LOrExpOpAndLAndExpAST : public BaseAST {
public:
    std::unique_ptr<LOrExpAST> first_expression;
    std::unique_ptr<LAndExpAST> latter_expression;

    LOrExpOpAndLAndExpAST(std::unique_ptr<LOrExpAST> first_exp, std::unique_ptr<LAndExpAST> latter_exp)
        : first_expression(std::move(first_exp)), latter_expression(std::move(latter_exp)) {}
    
    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
    std::optional<int> evaluateConstant(SymbolTable& symbol_table) const override;
};

class LOrExpAST : public BaseAST {
public:
    std::variant<std::unique_ptr<LAndExpAST>, std::unique_ptr<LOrExpOpAndLAndExpAST>> expression;

    explicit LOrExpAST(std::unique_ptr<LAndExpAST> land_exp)
        : expression(std::move(land_exp)) {}
    explicit LOrExpAST(std::unique_ptr<LOrExpOpAndLAndExpAST> lor_exp_op_and_land_exp)
        : expression(std::move(lor_exp_op_and_land_exp)) {}

    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
    std::optional<int> evaluateConstant(SymbolTable& symbol_table) const override;
};

class DeclAST : public BaseAST {
public:
    std::variant<std::unique_ptr<ConstDeclAST>, std::unique_ptr<VarDeclAST>> declaration;
    
    DeclAST(std::unique_ptr<ConstDeclAST> decl)
        : declaration(std::move(decl)) {}
    DeclAST(std::unique_ptr<VarDeclAST> decl)
        : declaration(std::move(decl)) {}

    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const;
};

class ConstDeclAST : public BaseAST {
public:
    BType btype; // 基本类型
    std::vector<std::unique_ptr<ConstDefAST>> const_defs; // 常量定义列表

    ConstDeclAST(BType type, std::vector<std::unique_ptr<ConstDefAST>> defs)
        : btype(type), const_defs(std::move(defs)) {}
    explicit ConstDeclAST(BType type)
        : btype(type) {}
    
    ConstDeclAST(BType type, std::unique_ptr<ConstDefAST> def)
        : btype(type) {
        const_defs.push_back(std::move(def));
    }

    void pushConstDef(std::unique_ptr<ConstDefAST> def) {
        const_defs.push_back(std::move(def));
    }

    void Dump() const override;
    // 处理常量声明并添加到符号表
    void processConstDecl(SymbolTable& symbol_table) const;
};

class ConstDefAST : public BaseAST {
public:
    std::string ident; // 标识符
    std::unique_ptr<ConstInitValAST> const_init_val; // 初始化值

    ConstDefAST(const std::string& identifier, std::unique_ptr<ConstInitValAST> init_val)
        : ident(identifier), const_init_val(std::move(init_val)) {}

    void Dump() const override;
    // 处理常量定义并添加到符号表，返回常量值
    std::optional<int> processConstDef(SymbolTable& symbol_table) const;
};

class ConstInitValAST : public BaseAST {
public:
    std::unique_ptr<ConstExpAST> const_exp; // 常量表达式

    ConstInitValAST(std::unique_ptr<ConstExpAST> exp)
        : const_exp(std::move(exp)) {}

    void Dump() const override;
};

class ConstExpAST : public BaseAST {
public:
    std::unique_ptr<ExpAST> expression; // 表达式

    explicit ConstExpAST(std::unique_ptr<ExpAST> exp)
        : expression(std::move(exp)) {}

    void Dump() const override;
    std::optional<int> evaluateConstant(SymbolTable& symbol_table) const override;
};

class LValAST : public BaseAST {
public:
    std::string ident; // 标识符

    explicit LValAST(const std::string& id)
        : ident(id) {}

    void Dump() const override;
    std::optional<int> evaluateConstant(SymbolTable& symbol_table) const override;
};

class VarDefAST : public BaseAST {
public:
    std::string ident; // 标识符
    std::optional<std::unique_ptr<ConstInitValAST>> const_init_val; // 可选的常量初始化值

    VarDefAST(const std::string& identifier, std::unique_ptr<ConstInitValAST> init_val)
        : ident(identifier), const_init_val(std::move(init_val)) {}
    VarDefAST(const std::string& identifier)
        : ident(identifier), const_init_val(std::nullopt) {}
    
    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const;
};

class VarDeclAST : public BaseAST {
public:
    BType btype; // 基本类型
    std::vector<std::unique_ptr<VarDefAST>> var_defs; // 变量定义列表

    VarDeclAST(BType type, std::vector<std::unique_ptr<VarDefAST>> defs)
        : btype(type), var_defs(std::move(defs)) {}
    explicit VarDeclAST(BType type)
        : btype(type) {}
    VarDeclAST(BType type, std::unique_ptr<VarDefAST> def)
        : btype(type) {
        var_defs.push_back(std::move(def));
    }
    void pushVarDef(std::unique_ptr<VarDefAST> def) {
        var_defs.push_back(std::move(def));
    }

    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const;
};

