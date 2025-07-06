#include "ast.h"
#include "string_format.h"
#include <algorithm>
#include <sstream>


bool containsBasicBlockEnd(const std::string& block_koopa) {
    if (block_koopa.empty()) {
        return false;
    }
    
    // Find the last non-empty line
    size_t last_newline = block_koopa.find_last_of('\n');
    std::string last_line = (last_newline != std::string::npos) ? 
        block_koopa.substr(last_newline + 1) : block_koopa;
    
    return last_line.find("jump") != std::string::npos ||
           last_line.find("ret") != std::string::npos ||
           last_line.find("br") != std::string::npos;
}

bool containsBasicBlockEnd(const std::vector<std::string>& instructions) {
    if (instructions.empty()) {
        return false;
    }
    
    // Check the last instruction
    const auto& last_instruction = instructions.back();
    return last_instruction.find("jump ") != std::string::npos ||
           last_instruction.find("ret ") != std::string::npos ||
           last_instruction.find("br ") != std::string::npos;
}

std::string IfElseStmtAST::toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const
{
    std::string koopa_code = "";
    std::vector<std::string> instructions;

    // if 的条件判断部分
    const auto cond_var = BaseAST::getNewTempVar();
    const auto cond_code = condition->toKoopa(instructions);

    // instructions.push_back(
    //     stringFormat("%%%d = %s", cond_var, cond_code)
    // );
    instructions.push_back(
        stringFormat("br %s, %%then_%d, %%else_%d", cond_code, cond_var, cond_var)
    );

    // if 语句的 if 分支
    instructions.push_back(
        stringFormat("%%then_%d:", cond_var)
    );

    symbol_table.enterScope();
    const auto then_code = then_stmt->toKoopa(instructions, symbol_table);
    symbol_table.exitScope();

    if (!then_code.empty()) {
        instructions.push_back(then_code);
    }

    if (!containsBasicBlockEnd(instructions)) {
        // 如果 then 分支没有结束指令，添加一个跳转到 if 语句之后的部分
        instructions.push_back(stringFormat("jump %%end_%d", cond_var));
    }   

    // if 语句的 else 分支
    instructions.push_back(
        stringFormat("%%else_%d:", cond_var)
    );

    if (else_stmt.has_value()) {
        symbol_table.enterScope();
        const auto else_code = else_stmt.value()->toKoopa(instructions, symbol_table);
        symbol_table.exitScope();
        if (!else_code.empty()) {
            instructions.push_back(else_code);
        }
    }
    // 如果 else 分支没有结束指令，添加一个跳转到 if 语句之后的部分
    if (!containsBasicBlockEnd(instructions)) {
        instructions.push_back(stringFormat("jump %%end_%d", cond_var));
    }
    
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

void WhileStmtAST::Dump() const
{
    std::cout << "WhileStmtAST { while ";
    if (condition) {
        condition->Dump();
    } else {
        std::cout << "null";
    }
    std::cout << "; then ";
    if (body) {
        body->Dump();
    } else {
        std::cout << "null";
    }
    std::cout << " }";
}

std::string WhileStmtAST::toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) const
{
    std::vector<std::string> instructions;
    auto cond_var = BaseAST::getNewTempVar();

    generated_instructions.push_back(
        stringFormat("jump %while_entry_%d", cond_var));
    generated_instructions.push_back(
        stringFormat("%%while_entry_%d:", cond_var));
    
    symbol_table.enterScope();
    // 生成条件判断代码
    const auto cond_code = condition->toKoopa(instructions);
    instructions.push_back(
        stringFormat("br %s, %%while_body_%d, %%while_end_%d", cond_code, cond_var, cond_var)
    );

    // 生成循环体代码
    instructions.push_back(
        stringFormat("%%while_body_%d:", cond_var)
    );
    const auto body_code = body->toKoopa(instructions, symbol_table);
    symbol_table.exitScope();
    if (!body_code.empty()) {
        instructions.push_back(body_code);
    }

    // 如果循环体没有结束指令，添加一个跳转到循环入口
    if (!containsBasicBlockEnd(instructions)) {
        instructions.push_back(stringFormat("jump %%while_entry_%d", cond_var));
    }

    // 循环结束的标签
    instructions.push_back(
        stringFormat("%%while_end_%d:", cond_var)
    );

    // 将生成的指令添加到最终的 IR 代码中
    generated_instructions.insert(generated_instructions.end(), instructions.begin(), instructions.end());
    
    return "";
}
