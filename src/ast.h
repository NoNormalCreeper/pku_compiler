#pragma once

#include <iostream>
#include <memory>
#include <string>

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
};

// Number
class NumberAST : public BaseAST {
public:
    int value;

    NumberAST(int val) : value(val) {}

    void Dump() const override
    {
        std::cout << "NumberAST { " << value << " }";
    }
};

// FuncType
class FuncTypeAST : public BaseAST {
public:
    std::string type_name; // "int"

    FuncTypeAST(const std::string &name) : type_name(name) {}

    void Dump() const override
    {
        std::cout << "FuncTypeAST { " << type_name << " }";
    }
};

// Stmt
class StmtAST : public BaseAST {
public:
    // Stmt -> "return" Number ";"
    std::unique_ptr<NumberAST> number;

    StmtAST(std::unique_ptr<NumberAST> num) : number(std::move(num)) {}

    void Dump() const override
    {
        std::cout << "StmtAST { return ";
        if (number) {
            number->Dump();
        } else {
            std::cout << "null";
        }
        std::cout << "; }";
    }
};

// Block
class BlockAST : public BaseAST {
public:
    std::unique_ptr<StmtAST> stmt;

    BlockAST(std::unique_ptr<StmtAST> s) : stmt(std::move(s)) {}

    void Dump() const override
    {
        std::cout << "BlockAST { ";
        if (stmt) {
            stmt->Dump();
        } else {
            std::cout << "empty";
        }
        std::cout << " }";
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<FuncTypeAST> func_type;
    std::string ident;
    std::unique_ptr<BlockAST> block;

    FuncDefAST(std::unique_ptr<FuncTypeAST> type, const std::string &id, std::unique_ptr<BlockAST> blk)
        : func_type(std::move(type)), ident(id), block(std::move(blk)) {}

    void Dump() const override
    {
        std::cout << "FuncDefAST { ";
        func_type->Dump();
        std::cout << ", " << ident << ", ";
        block->Dump();
        std::cout << " }";
    }
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
    // 用智能指针管理对象
    std::unique_ptr<FuncDefAST> func_def;

    CompUnitAST(std::unique_ptr<FuncDefAST> func) : func_def(std::move(func)) {}

    void Dump() const override
    {
        std::cout << "CompUnitAST { ";
        func_def->Dump();
        std::cout << " }";
    }
};