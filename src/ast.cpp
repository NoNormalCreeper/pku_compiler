#include "ast.h"
#include "string_format.h"
#include "symbol_table.h"
#include <cmath>
#include <optional>
#include <vector>

// 全局符号表指针定义
SymbolTable* BaseAST::global_symbol_table = nullptr;

// BaseAST implementations
std::string BaseAST::toKoopa() const
{
    return "/* koopa not implemented */";
}

void BaseAST::Dump() const
{
    std::cout << "BaseAST { /* not implemented */ }";
}

// 默认的常量求值实现 - 大部分节点不是常量
std::optional<int> BaseAST::evaluateConstant(SymbolTable& symbol_table) const
{
    return std::nullopt;
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

std::optional<int> NumberAST::evaluateConstant(SymbolTable& /* symbol_table */) const
{
    return value;  // 数字字面量总是常量
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
ReturnExpStmtAST::ReturnExpStmtAST(std::unique_ptr<ExpAST> exp)
    : expression(std::move(exp))
{
}

void ReturnExpStmtAST::Dump() const
{
    std::cout << "ReturnStmtAST { return ";
    if (expression) {
        expression->Dump();
    } else {
        std::cout << "null";
    }
    std::cout << "; }";
}

// const版本，符合基类接口
std::string ReturnExpStmtAST::toKoopa() const
{
    // 对于const版本，我们需要调用非const版本
    // 这里使用const_cast，在这种情况下是安全的
    return const_cast<ReturnExpStmtAST*>(this)->toKoopa();
}

std::string ReturnExpStmtAST::toKoopa() 
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

void StmtAST::Dump() const
{
    std::cout << "StmtAST { ";
    std::visit([&](const auto& stmt_ptr) {
        if (stmt_ptr) {
            stmt_ptr->Dump();
        } else {
            std::cout << "null";
        }
    }, statement);
    std::cout << " }";
}

void LValEqExpStmtAST::Dump() const
{
    std::cout << "LValEqExpStmtAST { ";
    if (lval) {
        lval->Dump();
    } else {
        std::cout << "null";
    }
    std::cout << " = ";
    if (expression) {
        expression->Dump();
    } else {
        std::cout << "null";
    }
    std::cout << "; }";
}

std::string StmtAST::toKoopa() const
{
    return std::visit([&](const auto& stmt_ptr) -> std::string {
        if constexpr (std::is_same_v<std::decay_t<decltype(stmt_ptr)>, std::unique_ptr<LValEqExpStmtAST>>) {
            // For LValEqExpStmtAST, we need to handle it differently since it requires symbol table
            // This is a fallback that shouldn't normally be called
            return "/* LValEqExpStmtAST requires symbol table */";
        } else if constexpr (std::is_same_v<std::decay_t<decltype(stmt_ptr)>, std::unique_ptr<ReturnExpStmtAST>>) {
            return stmt_ptr->toKoopa();
        }
        return "/* koopa not implemented for this statement */";
    }, statement);
}

std::string StmtAST::toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const
{
    return std::visit([&](const auto& stmt_ptr) -> std::string {
        if constexpr (std::is_same_v<std::decay_t<decltype(stmt_ptr)>, std::unique_ptr<LValEqExpStmtAST>>) {
            return stmt_ptr->toKoopa(generated_instructions, symbol_table);
        } else if constexpr (std::is_same_v<std::decay_t<decltype(stmt_ptr)>, std::unique_ptr<ReturnExpStmtAST>>) {
            // ReturnExpStmtAST has its own generated_instructions member, so just call the existing method
            return stmt_ptr->toKoopa();
        }
        return "/* koopa not implemented for this statement */";
    }, statement);
}

std::string LValEqExpStmtAST::toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const
{
    if (!lval || !expression) {
        throw std::runtime_error("LValEqExpStmtAST: lval or expression is null");
    }

    // 获取左值标识符
    const auto& var_name = lval->ident;

    // 生成右值表达式的 Koopa IR
    auto exp = expression->toKoopa(generated_instructions);

    // 检查符号表中是否存在该变量
    const auto& symbol_item = symbol_table.getSymbol(var_name);
    if (!symbol_item.has_value() || symbol_item->symbol_type != SymbolType::VAR) {
        throw std::runtime_error(stringFormat("Variable '%s' not defined", var_name.c_str()));
    }

    // 生成 store 指令
    generated_instructions.push_back(stringFormat("store %s, @%s", exp, var_name));

    return ""; // 返回空字符串，因为已经将指令添加到 generated_instructions 中

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
        try {
            if (item) {
                item->Dump();
            } else {
                std::cout << "null";
            }
        } catch (const std::exception& e) {
            std::cout << "Error dumping BlockItem: " << e.what();
        }
        std::cout << ", ";
    }
    std::cout << " }";
}

std::string BlockAST::toKoopa() const
{
    // if (stmt) {
    //     return stringFormat("%s", stmt->toKoopa());
    // }
    // std::string result;
    // for (const auto& item : block_items) {
    //     result += item->toKoopa();
    //     result += "\n";
    // }
    return "";
}

// BlockAST带符号表的toKoopa实现
std::string BlockAST::toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const
{
    std::string result;
    
    // 为块创建新的作用域
    symbol_table.enterScope();
    
    for (const auto& item : block_items) {
        result += item->toKoopa(generated_instructions, symbol_table);
    }
    
    // 退出作用域
    symbol_table.exitScope();
    
    return result;
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

std::string FuncDefAST::toKoopa(std::vector<std::string>& generated_instructions) const
{
    auto is_entry_fun = (ident == "main" && func_type->type_name == "int");
    
    // 清空指令列表以开始新的函数
    generated_instructions.clear();
    
    std::string block_koopa;
    // 使用全局符号表生成函数体
    if (BaseAST::global_symbol_table != nullptr) {
        block_koopa = block->toKoopa(generated_instructions, *BaseAST::global_symbol_table);
    } else {
        // 如果没有全局符号表，使用原来的方式
        block_koopa = block->toKoopa();
    }
    
    // 将生成的指令添加到结果中
    std::string instructions_str;
    for (const auto& instr : generated_instructions) {
        instructions_str += instr + "\n";
    }
    
    return stringFormat("fun @%s(%s): %s {\n%s%s%s}",
        ident, // 标识符
        "", // 参数列表，暂时留空
        func_type->toKoopa(), // 返回类型
        is_entry_fun ? "\%entry:\n" : "", // 如果是入口函数，添加 entry
        instructions_str, // 变量声明等指令
        block_koopa // 函数体其他内容
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
    std::vector<std::string> generated_instructions;
    return toKoopa(generated_instructions);
}

std::string CompUnitAST::toKoopa(std::vector<std::string>& generated_instructions) const
{
    if (func_def) {
        return func_def->toKoopa(generated_instructions);
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
    std::visit([&](const auto& expr_ptr) {
        if (expr_ptr) {
            expr_ptr->Dump();
        } else {
            std::cout << "null";
        }
    }, expression);
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
        const auto& decl_ptr = std::get<std::unique_ptr<DeclAST>>(item);
        if (decl_ptr) {
            decl_ptr->Dump();
            // 如果是声明，输出声明类型
            if (std::holds_alternative<std::unique_ptr<ConstDeclAST>>(decl_ptr->declaration)) {
                std::cout << " (const declaration)";
            } else if (std::holds_alternative<std::unique_ptr<VarDeclAST>>(decl_ptr->declaration)) {
                std::cout << " (var declaration)";
            }
        } else {
            std::cout << "null declaration";
        }
    } else {
        const auto& stmt_ptr = std::get<std::unique_ptr<StmtAST>>(item);
        if (stmt_ptr) {
            stmt_ptr->Dump();
        } else {
            std::cout << "null statement";
        }
    }
    std::cout << " }";
}

// BlockItemAST的toKoopa实现
auto BlockItemAST::toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const -> std::string
{
    return std::visit([&](const auto& item_ptr) -> std::string {
        if constexpr (std::is_same_v<std::decay_t<decltype(item_ptr)>, std::unique_ptr<DeclAST>>) {
            // 这是一个声明，需要处理常量定义并添加到符号表
            // 处理常量声明
            if (std::holds_alternative<std::unique_ptr<ConstDeclAST>>(item_ptr->declaration)) {
                auto& const_decl = std::get<std::unique_ptr<ConstDeclAST>>(item_ptr->declaration);
                const_decl->processConstDecl(symbol_table);
            } else if (std::holds_alternative<std::unique_ptr<VarDeclAST>>(item_ptr->declaration)) {
                // 处理变量声明
                auto& var_decl = std::get<std::unique_ptr<VarDeclAST>>(item_ptr->declaration);
                return var_decl->toKoopa(generated_instructions, symbol_table);
            }
            return "";  // 常量声明不生成IR代码
        } else {
            // 这是一个语句，生成相应的IR代码
            return item_ptr->toKoopa(generated_instructions, symbol_table);
        }
    }, item);
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
    std::visit([&](const auto& decl_ptr) {
        if (decl_ptr) {
            decl_ptr->Dump();
        } else {
            std::cout << "null";
        }
    }, declaration);
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

void VarDeclAST::Dump() const
{
    const auto btype_display_name = [](const BType& type_param) {
        switch (type_param) {
            case BT_INT: return "int";
        }
        return "int"; // default
    };
    
    std::cout << "VarDeclAST { " << btype_display_name(btype) << " ";
    for (size_t i = 0; i < var_defs.size(); ++i) {
        var_defs[i]->Dump();
        if (i < var_defs.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << " }";
}

void VarDefAST::Dump() const
{
    std::cout << "VarDefAST { " << ident;
    if (const_init_val.has_value()) {
        std::cout << " = ";
        const_init_val.value()->Dump();
    }
    std::cout << " }";
}

std::string VarDeclAST::toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const
{
    const auto& type_name = btype == BT_INT ? "i32" : "/* unknown */";
    
    // 遍历，alloc
    for (const auto& var_def : var_defs) {
        std::string var_name = var_def->ident;
        
        // 判断、存入符号表 (先添加变量到符号表，不管是否有初始化值)
        const auto& new_symbol = SymbolTableItem(
            SymbolType::VAR, type_name, var_name, std::nullopt
        );
        if (!symbol_table.addSymbol(new_symbol)) {
            throw std::runtime_error(stringFormat("Variable '%s' already defined", var_name.c_str()));
        }
        
        // 生成 alloc 指令
        generated_instructions.push_back(stringFormat("  @%s = alloc %s", var_name.c_str(), type_name));
        
        // 处理初始化值
        if (var_def->const_init_val.has_value()) {
            // 尝试常量求值
            std::optional<int> init_val = var_def->const_init_val.value()->const_exp->evaluateConstant(symbol_table);
            
            if (init_val.has_value()) {
                // 常量初始化值
                generated_instructions.push_back(stringFormat("  store %d, @%s", init_val.value(), var_name.c_str()));
            } else {
                // 非常量初始化值，需要生成表达式的 IR 代码
                auto init_exp = var_def->const_init_val.value()->const_exp->expression->toKoopa(generated_instructions);
                generated_instructions.push_back(stringFormat("  store %s, @%s", init_exp.c_str(), var_name.c_str()));
            }
        }
    }

    return "";
}

std::string DeclAST::toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const
{
    return std::visit([&](const auto& decl_ptr) -> std::string {
        if constexpr (std::is_same_v<std::decay_t<decltype(decl_ptr)>, std::unique_ptr<ConstDeclAST>>) {
            // 处理常量声明
            decl_ptr->processConstDecl(symbol_table);
            return "";  // 常量声明不生成IR代码
        } else if constexpr (std::is_same_v<std::decay_t<decltype(decl_ptr)>, std::unique_ptr<VarDeclAST>>) {
            // 处理变量声明
            return decl_ptr->toKoopa(generated_instructions, symbol_table);
        }
        return "";  // 默认返回空字符串
    }, declaration);
}

void OptionalExpStmtAST::Dump() const
{
    std::cout << "OptionalExpStmtAST { ";
    if (expression.has_value()) {
        expression->get()->Dump();
    } else {
        std::cout << "nullopt";
    }
    std::cout << "; }";
}

std::string OptionalExpStmtAST::toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const
{
    return "/* ... */";
}

void BlockStmtAST::Dump() const
{
    std::cout << "BlockStmtAST { ";
    block->Dump();
    std::cout << " }";
}

std::string BlockStmtAST::toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const
{
    return block->toKoopa(generated_instructions, symbol_table);
}
