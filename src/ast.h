#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>

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
    virtual std::string toKoopa() const {
        return "";
    }
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

    std::string toKoopa() const override
    {
        return std::to_string(value);
    }
};

// FuncType
class FuncTypeAST : public BaseAST {
public:
    std::string type_name; // "int"
    std::string ident; // "main"

    FuncTypeAST(const std::string &name) : type_name(name) {}

    void Dump() const override
    {
        std::cout << "FuncTypeAST { " << type_name << " }";
    }

    std::string toKoopa() const override
    {
        return koopa_type_map.at(type_name);
    }

private:
    std::map<std::string, std::string> koopa_type_map = {
        {"int", "i32"},
        {"void", "void"},
    };
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

    std::string toKoopa() const override
    {
        if (number) {
            return stringFormat("ret %s\n", number->toKoopa());
        }
        return "ret void\n"; // 如果没有数字，返回 void
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

    std::string toKoopa() const override
    {
        if (stmt) {
            return stringFormat("  %s", stmt->toKoopa());
        }
        return "";
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

    std::string toKoopa() const override
    {
        auto is_entry_fun = (ident == "main" && func_type->type_name == "int");
        // std::cout << "[DEBUG] func_type: " << func_type->type_name << ", ident: " << ident << " , isEntry" << isEntryFun << std::endl;
        return stringFormat("fun @%s(%s): %s {\n%s%s}", 
            ident,  // 标识符
            "",  // 参数列表，暂时留空
            func_type->toKoopa(), // 返回类型
            is_entry_fun ? "\%entry:\n" : "", // 如果是入口函数，添加 entry
            block->toKoopa() // 函数体
        );
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

    std::string toKoopa() const override
    {
        if (func_def) {
            return func_def->toKoopa();
        }
        return "";
    }
};