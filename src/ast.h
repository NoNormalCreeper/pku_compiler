#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <variant>

#include "string_format.h"

// Forward declarations for all AST node classes
class BaseAST;
class CompUnitAST;
class FuncDefAST;
class FuncTypeAST;
class BlockAST;
class StmtAST;
class NumberAST;

// 所有 AST 的基类
class BaseAST {
public:
    virtual ~BaseAST() = default;

    virtual void Dump() const = 0;
    virtual std::string toKoopa() const;
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
    // Stmt -> "return" Number ";"
    std::unique_ptr<NumberAST> number;

    StmtAST(std::unique_ptr<NumberAST> num);

    void Dump() const override;
    std::string toKoopa() const override;
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
class ExpAST;
class PrimaryAST;
class UnaryExpOpAndExpAST;
class UnaryExpAST;

enum UnaryOp : std::uint8_t {
    UNARY_OP_POSITIVE, // +
    UNARY_OP_NEGATIVE, // -
    UNARY_OP_NOT, // !
};

class PrimaryExpAST : public BaseAST {
public:
    std::variant<std::unique_ptr<ExpAST>, NumberAST> expression;
};

class UnaryExpAST : public BaseAST {
public:
    std::variant<std::unique_ptr<PrimaryExpAST>, std::unique_ptr<UnaryExpOpAndExpAST>> expression;
};

// UnaryOp UnaryExp
class UnaryExpOpAndExpAST : public BaseAST {
public:
    UnaryOp op; // 一元运算符
    std::unique_ptr<UnaryExpAST> latter_expression;
};
