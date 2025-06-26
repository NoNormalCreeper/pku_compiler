#pragma once

#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "string_format.h"

// Forward declarations for all AST node classes
class BaseAST;
class CompUnitAST;
class FuncDefAST;
class FuncTypeAST;
class BlockAST;
class BlockItemAST;
class StmtAST;
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

    NumberAST(int val);

    void Dump() const override;
    std::string toKoopa() const override;
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

// Stmt
class StmtAST : public BaseAST {
public:
    // Stmt -> "return" Exp ";"
    // std::unique_ptr<NumberAST> number;
    std::unique_ptr<ExpAST> expression;

    std::vector<std::string> generated_instructions; // 存储中间过程用于计算的 IR 指令

    StmtAST(std::unique_ptr<ExpAST> exp);

    void Dump() const override;
    std::string toKoopa() const override;
    std::string toKoopa() ;
};

// Block
class BlockAST : public BaseAST {
public:
    std::unique_ptr<StmtAST> stmt;

    BlockAST(std::unique_ptr<StmtAST> s);

    void Dump() const override;
    std::string toKoopa() const override;
};

class BlockItemAST : public BaseAST {
public:
    // BlockItem 可以是声明或语句
    std::variant<std::unique_ptr<DeclAST>, std::unique_ptr<StmtAST>> item;

    BlockItemAST(std::unique_ptr<DeclAST> decl)
        : item(std::move(decl)) {}
    BlockItemAST(std::unique_ptr<StmtAST> stmt)
        : item(std::move(stmt)) {}
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<FuncTypeAST> func_type;
    std::string ident;
    std::unique_ptr<BlockAST> block;

    FuncDefAST(std::unique_ptr<FuncTypeAST> type, const std::string& id, std::unique_ptr<BlockAST> blk);

    void Dump() const override;
    std::string toKoopa() const override;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
    // 用智能指针管理对象
    std::unique_ptr<FuncDefAST> func_def;

    CompUnitAST(std::unique_ptr<FuncDefAST> func);

    void Dump() const override;
    std::string toKoopa() const override;
};

/// 一元表达式

class PrimaryExpAST : public BaseAST {
public:
    std::variant<std::unique_ptr<ExpAST>, NumberAST> expression;

    PrimaryExpAST(std::unique_ptr<ExpAST> exp)
        : expression(std::move(exp)) {}
    PrimaryExpAST(NumberAST number)
        : expression(std::move(number)) {}

    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
};

class UnaryExpAST : public BaseAST {
public:
    std::variant<std::unique_ptr<PrimaryExpAST>, std::unique_ptr<UnaryExpOpAndExpAST>> expression;

    UnaryExpAST(std::unique_ptr<PrimaryExpAST> primary_exp)
        : expression(std::move(primary_exp)) {}
    UnaryExpAST(std::unique_ptr<UnaryExpOpAndExpAST> unary_exp_op_and_exp)
        : expression(std::move(unary_exp_op_and_exp)) {}
    
    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
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
};

class ExpAST : public BaseAST {
public:
    std::unique_ptr<LOrExpAST> expression;

    ExpAST(std::unique_ptr<LOrExpAST> add_exp)
        : expression(std::move(add_exp)) {}
    
    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
};

class MulExpOpAndExpAST : public BaseAST {
public:
    MulOp op; // 乘法运算符
    std::unique_ptr<MulExpAST> first_expression;
    std::unique_ptr<UnaryExpAST> latter_expression;

    MulExpOpAndExpAST(MulOp operation, std::unique_ptr<MulExpAST> first_exp, std::unique_ptr<UnaryExpAST> latter_exp)
        : op(operation), first_expression(std::move(first_exp)), latter_expression(std::move(latter_exp)) {}

    // void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
};

class MulExpAST : public BaseAST {
public:
    std::variant<std::unique_ptr<UnaryExpAST>, std::unique_ptr<MulExpOpAndExpAST>> expression;
    
    MulExpAST(std::unique_ptr<UnaryExpAST> unary_exp)
        : expression(std::move(unary_exp)) {}
    MulExpAST(std::unique_ptr<MulExpOpAndExpAST> mul_exp_op_and_exp)
        : expression(std::move(mul_exp_op_and_exp)) {}

    // void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
};

class AddExpOpAndMulExpAST : public BaseAST {
public:
    AddOp op; // 加法运算符
    std::unique_ptr<AddExpAST> first_expression;
    std::unique_ptr<MulExpAST> latter_expression;

    AddExpOpAndMulExpAST(AddOp operation, std::unique_ptr<AddExpAST> first_exp, std::unique_ptr<MulExpAST> latter_exp)
        : op(operation), first_expression(std::move(first_exp)), latter_expression(std::move(latter_exp)) {}

    // void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
};

class AddExpAST : public BaseAST {
public:
    std::variant<std::unique_ptr<MulExpAST>, std::unique_ptr<AddExpOpAndMulExpAST>> expression;

    AddExpAST(std::unique_ptr<MulExpAST> mul_exp)
        : expression(std::move(mul_exp)) {}
    AddExpAST(std::unique_ptr<AddExpOpAndMulExpAST> add_exp_op_and_mul_exp)
        : expression(std::move(add_exp_op_and_mul_exp)) {}
    
    // void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
};

class RelExpOpAndAddExpAST : public BaseAST {
public:
    RelOp op; // 关系运算符
    std::unique_ptr<RelExpAST> first_expression;
    std::unique_ptr<AddExpAST> latter_expression;

    RelExpOpAndAddExpAST(RelOp operation, std::unique_ptr<RelExpAST> first_exp, std::unique_ptr<AddExpAST> latter_exp)
        : op(operation), first_expression(std::move(first_exp)), latter_expression(std::move(latter_exp)) {}

    // void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
};

class RelExpAST : public BaseAST {
public:
    std::variant<std::unique_ptr<AddExpAST>, std::unique_ptr<RelExpOpAndAddExpAST>> expression;

    RelExpAST(std::unique_ptr<AddExpAST> add_exp)
        : expression(std::move(add_exp)) {}
    RelExpAST(std::unique_ptr<RelExpOpAndAddExpAST> rel_exp)
        : expression(std::move(rel_exp)) {}
    
    // void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
};

class EqExpOpAndRelExpAST : public BaseAST {
public:
    EqOp op; // 相等运算符
    std::unique_ptr<EqExpAST> first_expression;
    std::unique_ptr<RelExpAST> latter_expression;

    EqExpOpAndRelExpAST(EqOp operation, std::unique_ptr<EqExpAST> first_exp, std::unique_ptr<RelExpAST> latter_exp)
        : op(operation), first_expression(std::move(first_exp)), latter_expression(std::move(latter_exp)) {}
    
    // void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
};

class EqExpAST : public BaseAST {
public:
    std::variant<std::unique_ptr<RelExpAST>, std::unique_ptr<EqExpOpAndRelExpAST>> expression;

    EqExpAST(std::unique_ptr<RelExpAST> rel_exp)
        : expression(std::move(rel_exp)) {}
    EqExpAST(std::unique_ptr<EqExpOpAndRelExpAST> eq_exp_op_and_rel_exp)
        : expression(std::move(eq_exp_op_and_rel_exp)) {}
    
    // void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
};

class LAndExpOpAndEqExpAST : public BaseAST {
public:
    std::unique_ptr<LAndExpAST> first_expression;
    std::unique_ptr<EqExpAST> latter_expression;

    LAndExpOpAndEqExpAST(std::unique_ptr<LAndExpAST> first_exp, std::unique_ptr<EqExpAST> latter_exp)
        : first_expression(std::move(first_exp)), latter_expression(std::move(latter_exp)) {}
    
    // void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
};

class LAndExpAST : public BaseAST {
public:
    std::variant<std::unique_ptr<EqExpAST>, std::unique_ptr<LAndExpOpAndEqExpAST>> expression;

    LAndExpAST(std::unique_ptr<EqExpAST> eq_exp)
        : expression(std::move(eq_exp)) {}
    LAndExpAST(std::unique_ptr<LAndExpOpAndEqExpAST> land_exp_op_and_eq_exp)
        : expression(std::move(land_exp_op_and_eq_exp)) {}

    // void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
};

class LOrExpOpAndLAndExpAST : public BaseAST {
public:
    std::unique_ptr<LOrExpAST> first_expression;
    std::unique_ptr<LAndExpAST> latter_expression;

    LOrExpOpAndLAndExpAST(std::unique_ptr<LOrExpAST> first_exp, std::unique_ptr<LAndExpAST> latter_exp)
        : first_expression(std::move(first_exp)), latter_expression(std::move(latter_exp)) {}
    
    // void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
};

class LOrExpAST : public BaseAST {
public:
    std::variant<std::unique_ptr<LAndExpAST>, std::unique_ptr<LOrExpOpAndLAndExpAST>> expression;

    LOrExpAST(std::unique_ptr<LAndExpAST> land_exp)
        : expression(std::move(land_exp)) {}
    LOrExpAST(std::unique_ptr<LOrExpOpAndLAndExpAST> lor_exp_op_and_land_exp)
        : expression(std::move(lor_exp_op_and_land_exp)) {}

    // void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
};

class DeclAST : public BaseAST {
public:
    std::unique_ptr<ConstDeclAST> const_decl;

    DeclAST(std::unique_ptr<ConstDeclAST> decl)
        : const_decl(std::move(decl)) {}
};

class ConstDeclAST : public BaseAST {
public:
    BType btype; // 基本类型
    std::vector<std::unique_ptr<ConstDefAST>> const_defs; // 常量定义列表

    ConstDeclAST(BType type, std::vector<std::unique_ptr<ConstDefAST>> defs)
        : btype(type), const_defs(std::move(defs)) {}
    ConstDeclAST(BType type)
        : btype(type) {}
};

class ConstDefAST : public BaseAST {
public:
    std::string ident; // 标识符
    std::unique_ptr<ConstInitValAST> const_init_val; // 初始化值

    ConstDefAST(const std::string& id, std::unique_ptr<ConstInitValAST> init_val)
        : ident(id), const_init_val(std::move(init_val)) {}
};

class ConstInitValAST : public BaseAST {
public:
    std::unique_ptr<ConstExpAST> const_exp; // 常量表达式

    ConstInitValAST(std::unique_ptr<ConstExpAST> exp)
        : const_exp(std::move(exp)) {}
};

class ConstExpAST : public BaseAST {
public:
    std::unique_ptr<ExpAST> expression; // 表达式

    ConstExpAST(std::unique_ptr<ExpAST> exp)
        : expression(std::move(exp)) {}
};

class LValAST : public BaseAST {
public:
    std::string ident; // 标识符

    LValAST(const std::string& id)
        : ident(id) {}
};


