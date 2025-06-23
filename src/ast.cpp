#include "ast.h"
#include <cmath>

// BaseAST implementations
std::string BaseAST::toKoopa() const
{
    return "/* koopa not implemented */";
}

void BaseAST::Dump() const
{
    std::cout << "BaseAST { /* not implemented */ }";
}

// NumberAST implementations
NumberAST::NumberAST(int val)
    : value(val)
{
}

void NumberAST::Dump() const
{
    std::cout << "NumberAST { " << value << " }";
}

std::string NumberAST::toKoopa() const
{
    return std::to_string(value);
}

// FuncTypeAST implementations
FuncTypeAST::FuncTypeAST(const std::string& name)
    : type_name(name)
{
}

void FuncTypeAST::Dump() const
{
    std::cout << "FuncTypeAST { " << type_name << " }";
}

std::string FuncTypeAST::toKoopa() const
{
    return koopa_type_map.at(type_name);
}

// StmtAST implementations
StmtAST::StmtAST(std::unique_ptr<ExpAST> exp)
    : expression(std::move(exp))
{
}

void StmtAST::Dump() const
{
    std::cout << "StmtAST { return ";
    if (expression) {
        expression->Dump();
    } else {
        std::cout << "null";
    }
    std::cout << "; }";
}

std::string StmtAST::toKoopa() const
{
    if (expression) {
        return stringFormat("ret %s\n", expression->toKoopa());
    }
    return "ret void\n"; // 如果没有数字，返回 void
}

// BlockAST implementations
BlockAST::BlockAST(std::unique_ptr<StmtAST> s)
    : stmt(std::move(s))
{
}

void BlockAST::Dump() const
{
    std::cout << "BlockAST { ";
    if (stmt) {
        stmt->Dump();
    } else {
        std::cout << "empty";
    }
    std::cout << " }";
}

std::string BlockAST::toKoopa() const
{
    if (stmt) {
        return stringFormat("  %s", stmt->toKoopa());
    }
    return "";
}

// FuncDefAST implementations
FuncDefAST::FuncDefAST(std::unique_ptr<FuncTypeAST> type, const std::string& id, std::unique_ptr<BlockAST> blk)
    : func_type(std::move(type))
    , ident(id)
    , block(std::move(blk))
{
}

void FuncDefAST::Dump() const
{
    std::cout << "FuncDefAST { ";
    func_type->Dump();
    std::cout << ", " << ident << ", ";
    block->Dump();
    std::cout << " }";
}

std::string FuncDefAST::toKoopa() const
{
    auto is_entry_fun = (ident == "main" && func_type->type_name == "int");
    // std::cout << "[DEBUG] func_type: " << func_type->type_name << ", ident: " << ident << " , isEntry" << isEntryFun << std::endl;
    return stringFormat("fun @%s(%s): %s {\n%s%s}",
        ident, // 标识符
        "", // 参数列表，暂时留空
        func_type->toKoopa(), // 返回类型
        is_entry_fun ? "\%entry:\n" : "", // 如果是入口函数，添加 entry
        block->toKoopa() // 函数体
    );
}

// CompUnitAST implementations
CompUnitAST::CompUnitAST(std::unique_ptr<FuncDefAST> func)
    : func_def(std::move(func))
{
}

void CompUnitAST::Dump() const
{
    std::cout << "CompUnitAST { ";
    func_def->Dump();
    std::cout << " }";
}

std::string CompUnitAST::toKoopa() const
{
    if (func_def) {
        return func_def->toKoopa();
    }
    return "";
}