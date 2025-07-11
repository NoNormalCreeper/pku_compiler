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

void WhileStmtAST::setBodyLoopIds(int loop_id) const
{
    // 递归设置循环体中所有 break 和 continue 语句的 loop_id
    if (body) {
        setStmtLoopIds(body.get(), loop_id);
    }
}

// 新增递归辅助函数，用于遍历所有语句类型
void WhileStmtAST::setStmtLoopIds(StmtAST* stmt, int loop_id) const
{
    if (!stmt) return;

    std::visit([&](const auto& stmt_ptr) {
        if constexpr (std::is_same_v<std::decay_t<decltype(stmt_ptr)>, std::unique_ptr<BreakStmtAST>>) {
            auto* break_stmt = stmt_ptr.get();
            break_stmt->loop_id = loop_id;
        } else if constexpr (std::is_same_v<std::decay_t<decltype(stmt_ptr)>, std::unique_ptr<ContinueStmtAST>>) {
            auto* continue_stmt = stmt_ptr.get();
            continue_stmt->loop_id = loop_id;
        } else if constexpr (std::is_same_v<std::decay_t<decltype(stmt_ptr)>, std::unique_ptr<BlockStmtAST>>) {
            // 递归处理块语句
            auto* block_stmt = stmt_ptr.get();
            if (block_stmt->block) {
                for (const auto& block_item : block_stmt->block->block_items) {
                    if (std::holds_alternative<std::unique_ptr<StmtAST>>(block_item->item)) {
                        const auto& nested_stmt = std::get<std::unique_ptr<StmtAST>>(block_item->item);
                        setStmtLoopIds(nested_stmt.get(), loop_id);
                    }
                }
            }
        } else if constexpr (std::is_same_v<std::decay_t<decltype(stmt_ptr)>, std::unique_ptr<IfElseStmtAST>>) {
            // 递归处理 if-else 语句
            auto* if_else_stmt = stmt_ptr.get();
            if (if_else_stmt->then_stmt) {
                setStmtLoopIds(if_else_stmt->then_stmt.get(), loop_id);
            }
            if (if_else_stmt->else_stmt.has_value()) {
                setStmtLoopIds(if_else_stmt->else_stmt.value().get(), loop_id);
            }
        } else if constexpr (std::is_same_v<std::decay_t<decltype(stmt_ptr)>, std::unique_ptr<WhileStmtAST>>) {
            // 嵌套的 while 循环不需要设置外层的 loop_id，因为它们有自己的 loop_id
            // 这里不做处理，保持嵌套循环的独立性
        }
        // 其他语句类型（LValEqExpStmtAST, ReturnExpStmtAST, OptionalExpStmtAST）不需要特殊处理
    }, stmt->statement);
}

std::string WhileStmtAST::toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table)
{
    std::vector<std::string> instructions;
    int cond_var = loop_id.value_or(loop_id.emplace(BaseAST::getNewTempVar()));
    setBodyLoopIds(cond_var);

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

    // 生成用于 continue 的指向循环入口的跳转指令
    instructions.push_back(
        stringFormat("%%while_continue_%d:", cond_var)
    );
    instructions.push_back(
        stringFormat("jump %%while_entry_%d", cond_var)
    );

    // 循环结束的标签
    instructions.push_back(
        stringFormat("%%while_end_%d:", cond_var)
    );

    // 将生成的指令添加到最终的 IR 代码中
    generated_instructions.insert(generated_instructions.end(), instructions.begin(), instructions.end());

    return "";
}

std::string BreakStmtAST::toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) 
{
    // 生成跳转到循环结束的指令
    if (loop_id.has_value()) {
        generated_instructions.push_back(
            stringFormat("jump %%while_end_%d", loop_id.value())
        );
    } else {
        throw std::runtime_error("BreakStmtAST: loop_id is not set");
    }
    return "";
}

std::string ContinueStmtAST::toKoopa(std::vector<std::string>& generated_instructions, SymbolTable& symbol_table) 
{
    // 生成跳转到循环入口的指令
    if (loop_id.has_value()) {
        generated_instructions.push_back(
            stringFormat("jump %%while_continue_%d", loop_id.value())
        );
    } else {
        throw std::runtime_error("ContinueStmtAST: loop_id is not set");
    }
    return "";
}
