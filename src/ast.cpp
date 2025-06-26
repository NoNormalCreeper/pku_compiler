#include "ast.h"
#include "string_format.h"
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

// const版本，符合基类接口
std::string StmtAST::toKoopa() const
{
    // 对于const版本，我们需要调用非const版本
    // 这里使用const_cast，在这种情况下是安全的
    return const_cast<StmtAST*>(this)->toKoopa();
}

std::string StmtAST::toKoopa() 
{
    if (expression) {
        // 重置临时变量计数器
        BaseAST::resetTempVarCounter();
        // 清空指令列表
        generated_instructions.clear();

        std::string result = "";
        auto exp = expression->toKoopa(generated_instructions);
        if (!generated_instructions.empty()) {
            for (const auto& instr : generated_instructions) {
                result += stringFormat("  %s\n", instr);
            }
        }
        
        return stringFormat("%s  ret %s\n", result, exp);
    }
    return "  ret void\n"; // 如果没有数字，返回 void
}

// BlockAST implementations
// BlockAST::BlockAST(std::unique_ptr<StmtAST> s)
//     : stmt(std::move(s))
// {
// }

void BlockAST::Dump() const
{
    std::cout << "BlockAST { ";
    for (const auto& item : block_items) {
        item->Dump();
        std::cout << ", ";
    }
    std::cout << " }";
}

std::string BlockAST::toKoopa() const
{
    // if (stmt) {
    //     return stringFormat("%s", stmt->toKoopa());
    // }
    std::string result;
    for (const auto& item : block_items) {
        result += item->toKoopa();
        result += "\n";
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

void PrimaryExpAST::Dump() const
{
    std::cout << "PrimaryExpAST { ";
    if (std::holds_alternative<std::unique_ptr<ExpAST>>(expression)) {
        std::cout << "'( '";
        std::get<std::unique_ptr<ExpAST>>(expression)->Dump();
        std::cout << "' )'";
    } else if (std::holds_alternative<NumberAST>(expression)) {
        std::get<NumberAST>(expression).Dump();
    } else if (std::holds_alternative<std::unique_ptr<LValAST>>(expression)) {
        std::get<std::unique_ptr<LValAST>>(expression)->Dump();
    }
    std::cout << " }";
}

void UnaryExpAST::Dump() const
{
    std::cout << "UnaryExpAST { ";
    if (std::holds_alternative<std::unique_ptr<PrimaryExpAST>>(expression)) {
        std::get<std::unique_ptr<PrimaryExpAST>>(expression)->Dump();
    } else {
        std::get<std::unique_ptr<UnaryExpOpAndExpAST>>(expression)->Dump();
    }
    std::cout << " }";
}

void UnaryExpOpAndExpAST::Dump() const
{
    const auto op_display_name = [](const UnaryOp& op) {
        switch (op) {
            case UNARY_OP_POSITIVE:
                return "+";
            case UNARY_OP_NEGATIVE:
                return "-";
            case UNARY_OP_NOT:
                return "!";
        }
    };
    std::cout << "UnaryOp { " << op_display_name(op) << " }, ";
    std::cout << "UnaryExp { ";
    latter_expression->Dump();
    std::cout << " }";
}

// BlockItemAST implementation
void BlockItemAST::Dump() const
{
    std::cout << "BlockItemAST { ";
    if (std::holds_alternative<std::unique_ptr<DeclAST>>(item)) {
        std::get<std::unique_ptr<DeclAST>>(item)->Dump();
    } else {
        std::get<std::unique_ptr<StmtAST>>(item)->Dump();
    }
    std::cout << " }";
}

// MulExpOpAndExpAST implementation
void MulExpOpAndExpAST::Dump() const
{
    const auto op_display_name = [](const MulOp& operation) {
        switch (operation) {
            case MUL_OP_MUL: return "*";
            case MUL_OP_DIV: return "/";
            case MUL_OP_MOD: return "%";
        }
        return "*"; // default
    };
    
    std::cout << "MulExpOpAndExpAST { ";
    first_expression->Dump();
    std::cout << " " << op_display_name(op) << " ";
    latter_expression->Dump();
    std::cout << " }";
}

// MulExpAST implementation
void MulExpAST::Dump() const
{
    std::cout << "MulExpAST { ";
    if (std::holds_alternative<std::unique_ptr<UnaryExpAST>>(expression)) {
        std::get<std::unique_ptr<UnaryExpAST>>(expression)->Dump();
    } else {
        std::get<std::unique_ptr<MulExpOpAndExpAST>>(expression)->Dump();
    }
    std::cout << " }";
}

// AddExpOpAndMulExpAST implementation
void AddExpOpAndMulExpAST::Dump() const
{
    const auto op_display_name = [](const AddOp& operation) {
        switch (operation) {
            case ADD_OP_ADD: return "+";
            case ADD_OP_SUB: return "-";
        }
        return "+"; // default
    };
    
    std::cout << "AddExpOpAndMulExpAST { ";
    first_expression->Dump();
    std::cout << " " << op_display_name(op) << " ";
    latter_expression->Dump();
    std::cout << " }";
}

// AddExpAST implementation
void AddExpAST::Dump() const
{
    std::cout << "AddExpAST { ";
    if (std::holds_alternative<std::unique_ptr<MulExpAST>>(expression)) {
        std::get<std::unique_ptr<MulExpAST>>(expression)->Dump();
    } else {
        std::get<std::unique_ptr<AddExpOpAndMulExpAST>>(expression)->Dump();
    }
    std::cout << " }";
}

// RelExpOpAndAddExpAST implementation
void RelExpOpAndAddExpAST::Dump() const
{
    const auto op_display_name = [](const RelOp& operation) {
        switch (operation) {
            case REL_OP_LT: return "<";
            case REL_OP_LE: return "<=";
            case REL_OP_GT: return ">";
            case REL_OP_GE: return ">=";
        }
        return "<"; // default
    };
    
    std::cout << "RelExpOpAndAddExpAST { ";
    first_expression->Dump();
    std::cout << " " << op_display_name(op) << " ";
    latter_expression->Dump();
    std::cout << " }";
}

// RelExpAST implementation
void RelExpAST::Dump() const
{
    std::cout << "RelExpAST { ";
    if (std::holds_alternative<std::unique_ptr<AddExpAST>>(expression)) {
        std::get<std::unique_ptr<AddExpAST>>(expression)->Dump();
    } else {
        std::get<std::unique_ptr<RelExpOpAndAddExpAST>>(expression)->Dump();
    }
    std::cout << " }";
}

// EqExpOpAndRelExpAST implementation
void EqExpOpAndRelExpAST::Dump() const
{
    const auto op_display_name = [](const EqOp& operation) {
        switch (operation) {
            case EQ_OP_EQ: return "==";
            case EQ_OP_NE: return "!=";
        }
        return "=="; // default
    };
    
    std::cout << "EqExpOpAndRelExpAST { ";
    first_expression->Dump();
    std::cout << " " << op_display_name(op) << " ";
    latter_expression->Dump();
    std::cout << " }";
}

// EqExpAST implementation
void EqExpAST::Dump() const
{
    std::cout << "EqExpAST { ";
    if (std::holds_alternative<std::unique_ptr<RelExpAST>>(expression)) {
        std::get<std::unique_ptr<RelExpAST>>(expression)->Dump();
    } else {
        std::get<std::unique_ptr<EqExpOpAndRelExpAST>>(expression)->Dump();
    }
    std::cout << " }";
}

// LAndExpOpAndEqExpAST implementation
void LAndExpOpAndEqExpAST::Dump() const
{
    std::cout << "LAndExpOpAndEqExpAST { ";
    first_expression->Dump();
    std::cout << " && ";
    latter_expression->Dump();
    std::cout << " }";
}

// LAndExpAST implementation
void LAndExpAST::Dump() const
{
    std::cout << "LAndExpAST { ";
    if (std::holds_alternative<std::unique_ptr<EqExpAST>>(expression)) {
        std::get<std::unique_ptr<EqExpAST>>(expression)->Dump();
    } else {
        std::get<std::unique_ptr<LAndExpOpAndEqExpAST>>(expression)->Dump();
    }
    std::cout << " }";
}

// LOrExpOpAndLAndExpAST implementation
void LOrExpOpAndLAndExpAST::Dump() const
{
    std::cout << "LOrExpOpAndLAndExpAST { ";
    first_expression->Dump();
    std::cout << " || ";
    latter_expression->Dump();
    std::cout << " }";
}

// LOrExpAST implementation
void LOrExpAST::Dump() const
{
    std::cout << "LOrExpAST { ";
    if (std::holds_alternative<std::unique_ptr<LAndExpAST>>(expression)) {
        std::get<std::unique_ptr<LAndExpAST>>(expression)->Dump();
    } else {
        std::get<std::unique_ptr<LOrExpOpAndLAndExpAST>>(expression)->Dump();
    }
    std::cout << " }";
}

// DeclAST implementation
void DeclAST::Dump() const
{
    std::cout << "DeclAST { ";
    if (const_decl) {
        const_decl->Dump();
    } else {
        std::cout << "null";
    }
    std::cout << " }";
}

// ConstDeclAST implementation
void ConstDeclAST::Dump() const
{
    const auto btype_display_name = [](const BType& type_param) {
        switch (type_param) {
            case BT_INT: return "int";
        }
        return "int"; // default
    };
    
    std::cout << "ConstDeclAST { const " << btype_display_name(btype) << " ";
    for (size_t i = 0; i < const_defs.size(); ++i) {
        const_defs[i]->Dump();
        if (i < const_defs.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << " }";
}

// ConstDefAST implementation
void ConstDefAST::Dump() const
{
    std::cout << "ConstDefAST { " << ident << " = ";
    if (const_init_val) {
        const_init_val->Dump();
    } else {
        std::cout << "null";
    }
    std::cout << " }";
}

// ConstInitValAST implementation
void ConstInitValAST::Dump() const
{
    std::cout << "ConstInitValAST { ";
    if (const_exp) {
        const_exp->Dump();
    } else {
        std::cout << "null";
    }
    std::cout << " }";
}

// ConstExpAST implementation
void ConstExpAST::Dump() const
{
    std::cout << "ConstExpAST { ";
    if (expression) {
        expression->Dump();
    } else {
        std::cout << "null";
    }
    std::cout << " }";
}

// LValAST implementation
void LValAST::Dump() const
{
    std::cout << "LValAST { " << ident << " }";
}

void ExpAST::Dump() const
{
    std::cout << "ExpAST { ";
    if (expression) {
        expression->Dump();
    } else {
        std::cout << "null";
    }
    std::cout << " }";
}
