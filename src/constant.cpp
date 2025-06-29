/*
实现常量求值相关功能
*/

#include "ast.h"
#include <variant>

// PrimaryExpAST的常量求值
std::optional<int> PrimaryExpAST::evaluateConstant(SymbolTable& symbol_table) const
{
    if (std::holds_alternative<NumberAST>(expression)) {
        return std::get<NumberAST>(expression).evaluateConstant(symbol_table);
    } else if (std::holds_alternative<std::unique_ptr<LValAST>>(expression)) {
        return std::get<std::unique_ptr<LValAST>>(expression)->evaluateConstant(symbol_table);
    } else {
        // 括号表达式
        return std::get<std::unique_ptr<ExpAST>>(expression)->evaluateConstant(symbol_table);
    }
}

// UnaryExpAST的常量求值
std::optional<int> UnaryExpAST::evaluateConstant(SymbolTable& symbol_table) const
{
    return std::visit([&](const auto& expr) -> std::optional<int> {
        return expr->evaluateConstant(symbol_table);
    }, expression);
}

// UnaryExpOpAndExpAST的常量求值
std::optional<int> UnaryExpOpAndExpAST::evaluateConstant(SymbolTable& symbol_table) const
{
    auto operand_value = latter_expression->evaluateConstant(symbol_table);
    if (!operand_value.has_value()) {
        return std::nullopt;
    }
    
    switch (op) {
        case UNARY_OP_POSITIVE:
            return operand_value.value();
        case UNARY_OP_NEGATIVE:
            return -operand_value.value();
        case UNARY_OP_NOT:
            return operand_value.value() == 0 ? 1 : 0;
    }
    return std::nullopt;
}

// LValAST的常量求值 - 查找符号表中的常量值
std::optional<int> LValAST::evaluateConstant(SymbolTable& symbol_table) const
{
    auto symbol = symbol_table.getSymbol(ident);
    if (symbol.has_value() && symbol->is_const && symbol->value.has_value()) {
        return symbol->value.value();
    }
    return std::nullopt;  // 不是常量或未找到
}

// ConstExpAST的常量求值
std::optional<int> ConstExpAST::evaluateConstant(SymbolTable& symbol_table) const
{
    return expression->evaluateConstant(symbol_table);
}

// ConstDefAST处理常量定义
std::optional<int> ConstDefAST::processConstDef(SymbolTable& symbol_table) const
{
    // 求值常量初始化表达式
    auto init_value = const_init_val->const_exp->evaluateConstant(symbol_table);
    if (!init_value.has_value()) {
        // 常量初始化表达式必须是编译时常量
        return std::nullopt;
    }
    
    // 添加到符号表 - 使用原始标识符
    SymbolTableItem item(SymbolType::CONST, "int", ident, init_value.value(), true);
    if (!symbol_table.addSymbol(item)) {
        // 符号重定义
        return std::nullopt;
    }
    
    return init_value;
}

// ConstDeclAST处理常量声明
void ConstDeclAST::processConstDecl(SymbolTable& symbol_table) const
{
    for (const auto& const_def : const_defs) {
        const_def->processConstDef(symbol_table);
    }
}

// ExpAST的常量求值
std::optional<int> ExpAST::evaluateConstant(SymbolTable& symbol_table) const
{
    if (expression) {
        return expression->evaluateConstant(symbol_table);
    }
    return std::nullopt;
}

// AddExpAST的常量求值
std::optional<int> AddExpAST::evaluateConstant(SymbolTable& symbol_table) const
{
    return std::visit([&](const auto& expr) -> std::optional<int> {
        return expr->evaluateConstant(symbol_table);
    }, expression);
}

// MulExpAST的常量求值
std::optional<int> MulExpAST::evaluateConstant(SymbolTable& symbol_table) const
{
    return std::visit([&](const auto& expr) -> std::optional<int> {
        return expr->evaluateConstant(symbol_table);
    }, expression);
}

// AddExpOpAndMulExpAST的常量求值
std::optional<int> AddExpOpAndMulExpAST::evaluateConstant(SymbolTable& symbol_table) const
{
    auto first_value = first_expression->evaluateConstant(symbol_table);
    auto second_value = latter_expression->evaluateConstant(symbol_table);
    
    if (!first_value.has_value() || !second_value.has_value()) {
        return std::nullopt;
    }
    
    switch (op) {
        case ADD_OP_ADD:
            return first_value.value() + second_value.value();
        case ADD_OP_SUB:
            return first_value.value() - second_value.value();
    }
    return std::nullopt;
}

// MulExpOpAndExpAST的常量求值
std::optional<int> MulExpOpAndExpAST::evaluateConstant(SymbolTable& symbol_table) const
{
    auto first_value = first_expression->evaluateConstant(symbol_table);
    auto second_value = latter_expression->evaluateConstant(symbol_table);
    
    if (!first_value.has_value() || !second_value.has_value()) {
        return std::nullopt;
    }
    
    switch (op) {
        case MUL_OP_MUL:
            return first_value.value() * second_value.value();
        case MUL_OP_DIV:
            if (second_value.value() == 0) return std::nullopt; // 除零错误
            return first_value.value() / second_value.value();
        case MUL_OP_MOD:
            if (second_value.value() == 0) return std::nullopt; // 模零错误
            return first_value.value() % second_value.value();
    }
    return std::nullopt;
}

// EqExpOpAndRelExpAST的常量求值
std::optional<int> EqExpOpAndRelExpAST::evaluateConstant(SymbolTable& symbol_table) const
{
    auto first_value = first_expression->evaluateConstant(symbol_table);
    auto second_value = latter_expression->evaluateConstant(symbol_table);
    
    if (!first_value.has_value() || !second_value.has_value()) {
        return std::nullopt;
    }
    
    switch (op) {
        case EQ_OP_EQ:
            return first_value.value() == second_value.value() ? 1 : 0;
        case EQ_OP_NE:
            return first_value.value() != second_value.value() ? 1 : 0;
    }
    return std::nullopt;
}

// RelExpOpAndAddExpAST的常量求值
std::optional<int> RelExpOpAndAddExpAST::evaluateConstant(SymbolTable& symbol_table) const
{
    auto first_value = first_expression->evaluateConstant(symbol_table);
    auto second_value = latter_expression->evaluateConstant(symbol_table);
    
    if (!first_value.has_value() || !second_value.has_value()) {
        return std::nullopt;
    }
    
    switch (op) {
        case REL_OP_LT:
            return first_value.value() < second_value.value() ? 1 : 0;
        case REL_OP_LE:
            return first_value.value() <= second_value.value() ? 1 : 0;
        case REL_OP_GT:
            return first_value.value() > second_value.value() ? 1 : 0;
        case REL_OP_GE:
            return first_value.value() >= second_value.value() ? 1 : 0;
    }
    return std::nullopt;
}

// LAndExpOpAndEqExpAST的常量求值
std::optional<int> LAndExpOpAndEqExpAST::evaluateConstant(SymbolTable& symbol_table) const
{
    auto first_value = first_expression->evaluateConstant(symbol_table);
    auto second_value = latter_expression->evaluateConstant(symbol_table);
    
    if (!first_value.has_value() || !second_value.has_value()) {
        return std::nullopt;
    }
    
    // 逻辑与：两个操作数都非零时返回1，否则返回0
    return (first_value.value() != 0 && second_value.value() != 0) ? 1 : 0;
}

// LOrExpOpAndLAndExpAST的常量求值
std::optional<int> LOrExpOpAndLAndExpAST::evaluateConstant(SymbolTable& symbol_table) const
{
    auto first_value = first_expression->evaluateConstant(symbol_table);
    auto second_value = latter_expression->evaluateConstant(symbol_table);
    
    if (!first_value.has_value() || !second_value.has_value()) {
        return std::nullopt;
    }
    
    // 逻辑或：任一操作数非零时返回1，否则返回0
    return (first_value.value() != 0 || second_value.value() != 0) ? 1 : 0;
}

std::optional<int> RelExpAST::evaluateConstant(SymbolTable& symbol_table) const
{
    return std::visit([&](const auto& expr) -> std::optional<int> {
        return expr->evaluateConstant(symbol_table);
    }, expression);
}

// EqExpAST的常量求值
std::optional<int> EqExpAST::evaluateConstant(SymbolTable& symbol_table) const
{
    return std::visit([&](const auto& expr) -> std::optional<int> {
        return expr->evaluateConstant(symbol_table);
    }, expression);
}

// LAndExpAST的常量求值
std::optional<int> LAndExpAST::evaluateConstant(SymbolTable& symbol_table) const
{
    return std::visit([&](const auto& expr) -> std::optional<int> {
        return expr->evaluateConstant(symbol_table);
    }, expression);
}

// LOrExpAST的常量求值
std::optional<int> LOrExpAST::evaluateConstant(SymbolTable& symbol_table) const
{
    return std::visit([&](const auto& expr) -> std::optional<int> {
        return expr->evaluateConstant(symbol_table);
    }, expression);
}
