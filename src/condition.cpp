#include "ast.h"
#include "string_format.h"
#include <algorithm>

std::string IfElseStmtAST::toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const
{
    std::string koopa_code = "";
    std::vector<std::string> instructions;

    // if 的条件判断部分
    const auto cond_var = BaseAST::getNewTempVar();
    const auto cond_code = condition->toKoopa(instructions);

    instructions.push_back(
        stringFormat("%%%d = %s", cond_var, cond_code)
    );
    instructions.push_back(
        stringFormat("br %%%d, %%then_%d, %%else_%d", cond_var, cond_var, cond_var)
    );

    // if 语句的 if 分支
    instructions.push_back(
        stringFormat("%%then_%d:", cond_var)
    );

    const auto then_code = then_stmt->toKoopa(instructions, symbol_table);
    if (!then_code.empty()) {
        instructions.push_back(then_code);
    }

    instructions.push_back(
        stringFormat("jump %%end_%d", cond_var)
    );    

    // if 语句的 else 分支
    instructions.push_back(
        stringFormat("%%else_%d:", cond_var)
    );

    if (else_stmt.has_value()) {
        const auto else_code = else_stmt.value()->toKoopa(instructions, symbol_table);
        if (!else_code.empty()) {
            instructions.push_back(else_code);
        }
    }
    instructions.push_back(stringFormat("jump %%end_%d", cond_var));

    // if 语句之后的内容, if/else 分支的交汇处
    instructions.push_back(
        stringFormat("%%end_%d:", cond_var)
    );

    // std::for_each(instructions.begin(), instructions.end(),
    //     [&](const std::string& instr) {
    //         koopa_code += instr + "\n";
    //     }
    // );
    generated_instructions.insert(generated_instructions.end(), instructions.begin(), instructions.end());

    return "";
}