/*
实现表达式相关的 IR 生成 和 目标代码生成
*/

#include "ast.h"
#include <variant>

std::string ExpAST::toKoopa(std::vector<std::string>& generated_instructions)
{
    if (unary_exp) {
        return unary_exp->toKoopa(generated_instructions);
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
