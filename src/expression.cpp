/*
实现表达式相关的 IR 生成 和 目标代码生成
*/

#include "ast.h"
#include <variant>

std::string ExpAST::toKoopa(std::vector<std::string>& generated_instructions)
{
    if (expression) {
        return expression->toKoopa(generated_instructions);
    }
    return "";
}

std::string PrimaryExpAST::toKoopa(std::vector<std::string>& generated_instructions)
{
    if (std::holds_alternative<NumberAST>(expression)) {
        return std::get<NumberAST>(expression).toKoopa();
    } 
    return std::get<std::unique_ptr<ExpAST>>(expression)->toKoopa(generated_instructions);
}

std::string UnaryExpAST::toKoopa(std::vector<std::string>& generated_instructions)
{
    if (std::holds_alternative<std::unique_ptr<PrimaryExpAST>>(expression)) {
        return std::get<std::unique_ptr<PrimaryExpAST>>(expression)->toKoopa(generated_instructions);
    } 
    return std::get<std::unique_ptr<UnaryExpOpAndExpAST>>(expression)->toKoopa(generated_instructions);

}

std::string UnaryExpOpAndExpAST::toKoopa(std::vector<std::string>& generated_instructions)
{
    switch (op) {
        case UNARY_OP_POSITIVE:
            return latter_expression->toKoopa(generated_instructions);
        case UNARY_OP_NEGATIVE: {
            auto exp = latter_expression->toKoopa(generated_instructions);
            auto new_var = BaseAST::getNewTempVar();
            generated_instructions.push_back(stringFormat("%%%d = sub 0, %s", new_var, exp));
            return stringFormat("%%%d", new_var);
        }
        case UNARY_OP_NOT: {
            auto exp = latter_expression->toKoopa(generated_instructions);
            auto new_var = BaseAST::getNewTempVar();
            generated_instructions.push_back(stringFormat("%%%d = eq %s, 0", new_var, exp));
            return stringFormat("%%%d", new_var);
        }
    }

}

std::string AddExpAST::toKoopa(std::vector<std::string>& generated_instructions)
{
    if (std::holds_alternative<std::unique_ptr<AddExpOpAndMulExpAST>>(expression)) {
        return std::get<std::unique_ptr<AddExpOpAndMulExpAST>>(expression)->toKoopa(generated_instructions);
    }
    return std::get<std::unique_ptr<MulExpAST>>(expression)->toKoopa(generated_instructions);
}

std::string MulExpAST::toKoopa(std::vector<std::string>& generated_instructions)
{
    if (std::holds_alternative<std::unique_ptr<MulExpOpAndExpAST>>(expression)) {
        return std::get<std::unique_ptr<MulExpOpAndExpAST>>(expression)->toKoopa(generated_instructions);
    }
    return std::get<std::unique_ptr<UnaryExpAST>>(expression)->toKoopa(generated_instructions);
}

std::string AddExpOpAndMulExpAST::toKoopa(std::vector<std::string>& generated_instructions)
{
    auto first_exp = first_expression->toKoopa(generated_instructions);
    auto second_exp = latter_expression->toKoopa(generated_instructions);
    auto new_var = BaseAST::getNewTempVar();
    
    switch (op) {
        case ADD_OP_ADD:
            generated_instructions.push_back(stringFormat("%%%d = add %s, %s", new_var, first_exp, second_exp));
            break;
        case ADD_OP_SUB:
            generated_instructions.push_back(stringFormat("%%%d = sub %s, %s", new_var, first_exp, second_exp));
            break;
    }
    
    return stringFormat("%%%d", new_var);
}

std::string MulExpOpAndExpAST::toKoopa(std::vector<std::string>& generated_instructions)
{
    auto first_exp = first_expression->toKoopa(generated_instructions);
    auto second_exp = latter_expression->toKoopa(generated_instructions);
    auto new_var = BaseAST::getNewTempVar();
    
    switch (op) {
        case MUL_OP_MUL:
            generated_instructions.push_back(stringFormat("%%%d = mul %s, %s", new_var, first_exp, second_exp));
            break;
        case MUL_OP_DIV:
            generated_instructions.push_back(stringFormat("%%%d = div %s, %s", new_var, first_exp, second_exp));
            break;
        case MUL_OP_MOD:
            generated_instructions.push_back(stringFormat("%%%d = mod %s, %s", new_var, first_exp, second_exp));
            break;
    }
    
    return stringFormat("%%%d", new_var);
}

std::string RelExpAST::toKoopa(std::vector<std::string>& generated_instructions)
{
    return std::visit([&](auto& expr) -> std::string {
        return expr->toKoopa(generated_instructions);
    }, expression);
}

std::string RelExpOpAndAddExpAST::toKoopa(std::vector<std::string>& generated_instructions)
{
    auto first_exp = first_expression->toKoopa(generated_instructions);
    auto second_exp = latter_expression->toKoopa(generated_instructions);
    auto new_var = BaseAST::getNewTempVar();
    
    switch (op) {
        case REL_OP_LT:
            generated_instructions.push_back(stringFormat("%%%d = lt %s, %s", new_var, first_exp, second_exp));
            break;
        case REL_OP_LE:
            generated_instructions.push_back(stringFormat("%%%d = le %s, %s", new_var, first_exp, second_exp));
            break;
        case REL_OP_GT:
            generated_instructions.push_back(stringFormat("%%%d = gt %s, %s", new_var, first_exp, second_exp));
            break;
        case REL_OP_GE:
            generated_instructions.push_back(stringFormat("%%%d = ge %s, %s", new_var, first_exp, second_exp));
            break;
    }
    
    return stringFormat("%%%d", new_var);
}

std::string EqExpAST::toKoopa(std::vector<std::string>& generated_instructions)
{
    return std::visit([&](auto& expr) -> std::string {
        return expr->toKoopa(generated_instructions);
    }, expression);
}

std::string EqExpOpAndRelExpAST::toKoopa(std::vector<std::string>& generated_instructions)
{
    auto first_exp = first_expression->toKoopa(generated_instructions);
    auto second_exp = latter_expression->toKoopa(generated_instructions);
    auto new_var = BaseAST::getNewTempVar();
    
    switch (op) {
        case EQ_OP_EQ:
            generated_instructions.push_back(stringFormat("%%%d = eq %s, %s", new_var, first_exp, second_exp));
            break;
        case EQ_OP_NE:
            generated_instructions.push_back(stringFormat("%%%d = ne %s, %s", new_var, first_exp, second_exp));
            break;
    }
    
    return stringFormat("%%%d", new_var);
}

std::string LAndExpAST::toKoopa(std::vector<std::string>& generated_instructions)
{
    return std::visit([&](auto& expr) -> std::string {
        return expr->toKoopa(generated_instructions);
    }, expression);
}

std::string LAndExpOpAndEqExpAST::toKoopa(std::vector<std::string>& generated_instructions)
{
    auto first_exp = first_expression->toKoopa(generated_instructions);
    auto second_exp = latter_expression->toKoopa(generated_instructions);
    auto new_var = BaseAST::getNewTempVar();
    
    generated_instructions.push_back(stringFormat("%%%d = and %s, %s", new_var, first_exp, second_exp));
    
    return stringFormat("%%%d", new_var);
}

std::string LOrExpAST::toKoopa(std::vector<std::string>& generated_instructions)
{
    return std::visit([&](auto& expr) -> std::string {
        return expr->toKoopa(generated_instructions);
    }, expression);
}

std::string LOrExpOpAndLAndExpAST::toKoopa(std::vector<std::string>& generated_instructions)
{
    auto first_exp = first_expression->toKoopa(generated_instructions);
    auto second_exp = latter_expression->toKoopa(generated_instructions);
    auto new_var = BaseAST::getNewTempVar();
    
    generated_instructions.push_back(stringFormat("%%%d = or %s, %s", new_var, first_exp, second_exp));
    
    return stringFormat("%%%d", new_var);
}
