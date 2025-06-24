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
    std::unique_ptr<AddExpAST> add_exp;

    ExpAST(std::unique_ptr<AddExpAST> add_exp)
        : add_exp(std::move(add_exp)) {}
    
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

    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
};

class MulExpAST : public BaseAST {
public:
    std::variant<std::unique_ptr<UnaryExpAST>, std::unique_ptr<MulExpOpAndExpAST>> expression;
    
    MulExpAST(std::unique_ptr<UnaryExpAST> unary_exp)
        : expression(std::move(unary_exp)) {}
    MulExpAST(std::unique_ptr<MulExpOpAndExpAST> mul_exp_op_and_exp)
        : expression(std::move(mul_exp_op_and_exp)) {}

    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
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
};

class AddExpAST : public BaseAST {
public:
    std::variant<std::unique_ptr<MulExpAST>, std::unique_ptr<AddExpOpAndMulExpAST>> expression;

    AddExpAST(std::unique_ptr<MulExpAST> mul_exp)
        : expression(std::move(mul_exp)) {}
    AddExpAST(std::unique_ptr<AddExpOpAndMulExpAST> add_exp_op_and_mul_exp)
        : expression(std::move(add_exp_op_and_mul_exp)) {}
    
    void Dump() const override;
    std::string toKoopa(std::vector<std::string>& generated_instructions);
};


