#include "koopa_parser.h"

#include "koopa.h"
#include "string_format.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// RAII wrapper for koopa_program_t
class KoopaProgram {
public:
    KoopaProgram() = default;

    ~KoopaProgram()
    {
        if (program_ != nullptr) {
            koopa_delete_program(program_);
        }
    }

    // 禁用拷贝
    KoopaProgram(const KoopaProgram&) = delete;
    KoopaProgram& operator=(const KoopaProgram&) = delete;

    // 允许移动
    KoopaProgram(KoopaProgram&& other) noexcept
        : program_(other.program_)
    {
        other.program_ = nullptr;
    }

    KoopaProgram& operator=(KoopaProgram&& other) noexcept
    {
        if (this != &other) {
            if (program_ != nullptr) {
                koopa_delete_program(program_);
            }
            program_ = other.program_;
            other.program_ = nullptr;
        }
        return *this;
    }

    koopa_program_t* get() { return &program_; }
    const koopa_program_t* get() const { return &program_; }

private:
    koopa_program_t program_ = nullptr;
};

// RAII wrapper for koopa_raw_program_builder_t
class KoopaRawProgramBuilder {
public:
    KoopaRawProgramBuilder()
        : builder_(koopa_new_raw_program_builder())
    {
        assert(builder_ != nullptr);
    }

    ~KoopaRawProgramBuilder()
    {
        if (builder_ != nullptr) {
            koopa_delete_raw_program_builder(builder_);
        }
    }

    // 禁用拷贝
    KoopaRawProgramBuilder(const KoopaRawProgramBuilder&) = delete;
    KoopaRawProgramBuilder& operator=(const KoopaRawProgramBuilder&) = delete;

    // 允许移动
    KoopaRawProgramBuilder(KoopaRawProgramBuilder&& other) noexcept
        : builder_(other.builder_)
    {
        other.builder_ = nullptr;
    }

    KoopaRawProgramBuilder& operator=(KoopaRawProgramBuilder&& other) noexcept
    {
        if (this != &other) {
            if (builder_ != nullptr) {
                koopa_delete_raw_program_builder(builder_);
            }
            builder_ = other.builder_;
            other.builder_ = nullptr;
        }
        return *this;
    }

    koopa_raw_program_builder_t get() { return builder_; }

private:
    koopa_raw_program_builder_t builder_;
};

// PImpl implementation
class KoopaParser::Impl {
private:
    KoopaProgram program_;
    KoopaRawProgramBuilder builder_;
    koopa_raw_program_t raw_program_ {};
    int temp_var_count_ = 0;
    int serial_num_ = 0; // 用于生成唯一的临时 ID
    std::vector<std::string> generated_instructions_;
    std::unordered_map<const void*, std::string> value_to_register_; // 值到寄存器的映射
    std::unordered_map<const void*, int> value_to_offset_; // 值到栈偏移的映射
    std::unordered_map<std::string, int> var_to_offset_; // 变量名到函数栈内偏移量的映射
    int current_stack_offset_ = 0; // 当前栈偏移
    int total_stack_size_ = 0; // 总栈空间大小

public:
    int getSerialNum()
    {
        // 返回并递增
        return serial_num_++;
    }

    int getNewTempVar()
    {
        // 只使用 t0, t1, t2 三个寄存器，循环重用
        int reg_num = temp_var_count_ % 3;
        temp_var_count_++;
        return reg_num;
    }

    int clearTempVarCounter()
    {
        int old_count = temp_var_count_;
        temp_var_count_ = 0;
        value_to_register_.clear();
        value_to_offset_.clear();
        var_to_offset_.clear(); // 清空变量映射
        current_stack_offset_ = 0;
        total_stack_size_ = 0;
        return old_count;
    }

    auto& getGeneratedInstructions()
    {
        return generated_instructions_;
    }

    void addVarToOffset(const std::string& var_name, int offset)
    {
        var_to_offset_[var_name] = offset;
    }

    int getVarOffset(const std::string& var_name) {
        // 如果变量名在映射中，直接返回
        auto it = var_to_offset_.find(var_name);
        if (it != var_to_offset_.end()) {
            return it->second;
        }

        // 否则新增一条记录，每个变量占用4字节
        int offset = current_stack_offset_;
        var_to_offset_[var_name] = offset;
        current_stack_offset_ += 4;
        return offset;
    }

    int getValueOffset(const void* value) {
        auto it = value_to_offset_.find(value);
        if (it != value_to_offset_.end()) {
            return it->second;
        }
        
        // 分配新的栈空间
        int offset = current_stack_offset_;
        value_to_offset_[value] = offset;
        current_stack_offset_ += 4;
        return offset;
    }

    const koopa_raw_program_t* parseToRawProgram(const std::string& input)
    {
        // 解析输入字符串为 program
        koopa_error_code_t ret = koopa_parse_from_string(input.c_str(), program_.get());
        assert(ret == KOOPA_EC_SUCCESS);

        // 将 Koopa IR 程序转换为 raw program
        raw_program_ = koopa_build_raw_program(builder_.get(), *program_.get());

        return &raw_program_;
    }

    std::vector<std::string> Visit(const koopa_raw_program_t& program)
    {
        // 执行一些其他的必要操作
        // ...

        getGeneratedInstructions().clear();
        clearTempVarCounter();

        // 访问所有全局变量
        const auto values = Visit(program.values);
        // 访问所有函数（目前只有一个）
        const auto func = Visit(program.funcs);

        std::vector<std::string> commands = { "  .text" };

        // Add global declaration for main function
        commands.push_back("  .globl main");

        for (const auto& iden : values) {
            commands.push_back(stringFormat("  .globl %s", iden));
        }

        for (const auto& command : func) {
            commands.push_back(command);
        }

        return commands;
    }

    // 访问 raw slice
    std::vector<std::string> Visit(const koopa_raw_slice_t& slice)
    {
        std::vector<std::string> result;
        for (size_t i = 0; i < slice.len; ++i) {
            auto ptr = slice.buffer[i];
            std::vector<std::string> item_result;
            // 根据 slice 的 kind 决定将 ptr 视作何种元素
            // std::cout << "[DEBUG] Visiting slice item " << i
            //           << " of kind " << slice.kind << std::endl;
            switch (slice.kind) {
            case KOOPA_RSIK_FUNCTION:
                // 访问函数
                item_result = Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
                break;
            case KOOPA_RSIK_BASIC_BLOCK:
                // 访问基本块
                item_result = Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
                break;
            case KOOPA_RSIK_VALUE:
                // 访问指令
                item_result = Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
                break;
            default:
                // 我们暂时不会遇到其他内容, 于是不对其做任何处理
                assert(false);
            }
            result.insert(result.end(), item_result.begin(), item_result.end());
        }
        return result;
    }

    // 工具函数，返回 x 与 alignment 对齐后的值
    int alignTo(int x, int alignment)
    {
        return (x + alignment - 1) / alignment * alignment;
    }

    int getPrologueOffset(const koopa_raw_function_t& func)
    {
        // 计算函数 prologue 的偏移量
        // 统计所有需要栈空间的指令数量
        int stack_slots = 0;
        
        for (size_t i = 0; i < func->bbs.len; ++i) {
            const auto *bb = reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[i]);
            for (size_t j = 0; j < bb->insts.len; ++j) {
                const auto *inst = reinterpret_cast<koopa_raw_value_t>(bb->insts.buffer[j]);
                const auto& kind = inst->kind;
                
                if (kind.tag == KOOPA_RVT_ALLOC) {
                    // alloc指令需要栈空间
                    stack_slots++;
                } else if (kind.tag != KOOPA_RVT_RETURN && kind.tag != KOOPA_RVT_STORE 
                          && inst->ty->tag != KOOPA_RTT_UNIT) {
                    // 其他有返回值的指令也需要栈空间
                    stack_slots++;
                }
            }
        }
        
        total_stack_size_ = alignTo(stack_slots * 4, 16);
        return total_stack_size_; // 对齐到 16 字节
    }

    // 访问函数
    std::vector<std::string> Visit(const koopa_raw_function_t& func)
    {
        // 执行一些其他的必要操作
        // ...

        // 清空之前的指令
        getGeneratedInstructions().clear();
        clearTempVarCounter();

        // 访问所有基本块
        std::vector<std::string> result;

        // Extract function name (remove @ prefix if present)
        std::string func_name = func->name;
        if (func_name[0] == '@') {
            func_name = func_name.substr(1);
        }

        result.push_back(stringFormat("%s:", func_name));

        // 计算 prologue 偏移量
        int prologue_offset = getPrologueOffset(func);
        if (prologue_offset > 0) {
            if (prologue_offset <= 2047) {
                result.push_back(stringFormat("  addi sp, sp, -%d", prologue_offset));
            } else {
                result.push_back(stringFormat("  li t0, -%d", prologue_offset));
                result.push_back(stringFormat("  add sp, sp, t0"));
            }
        }

        const auto bbs = Visit(func->bbs);

        // auto generated = getGeneratedInstructions();
        // auto generated_with_indent = std::vector<std::string>();
        // for (const auto& instr : generated) {
        //     generated_with_indent.push_back(stringFormat("  %s", instr));
        // }
        // result.insert(result.end(), generated_with_indent.begin(), generated_with_indent.end());

        // for (const auto& bb : bbs) {
        //     result.push_back(stringFormat("  %s", bb));
        // }
        // return result;

        // 添加生成的指令到结果中
        auto generated = getGeneratedInstructions();
        for (const auto& instr : generated) {
            result.push_back(stringFormat("  %s", instr));
        }

        return result;
    }

    // 访问基本块
    std::vector<std::string> Visit(const koopa_raw_basic_block_t& bb)
    {
        // 执行一些其他的必要操作
        // ...

        // 生成基本块的标签
        std::string bb_label = extractIdentName(bb->name);
        getGeneratedInstructions().push_back(stringFormat("%s:", bb_label));

        // 访问所有指令
        return Visit(bb->insts);
        return {}; // 返回空向量，因为指令已经添加到 generated_instructions_ 中
    }

    // 访问指令
    std::vector<std::string> Visit(const koopa_raw_value_t& value)
    {
        // 检查是否已经处理过这个值
        auto iter = value_to_register_.find(value);
        if (iter != value_to_register_.end()) {
            // 如果值在栈上，加载到寄存器
            if (iter->second.find("(sp)") != std::string::npos) {
                int reg_num = getNewTempVar();
                getGeneratedInstructions().push_back(
                    stringFormat("lw t%d, %s", reg_num, iter->second));
                return { stringFormat("t%d", reg_num) };
            }
            return { iter->second };
        }

        // 根据指令类型判断后续需要如何访问
        const auto& kind = value->kind;
        std::vector<std::string> result;

        switch (kind.tag) {
        case KOOPA_RVT_RETURN:
            result = Visit(kind.data.ret);
            break;
        case KOOPA_RVT_INTEGER:
            result = Visit(kind.data.integer);
            break;
        case KOOPA_RVT_BINARY:
            result = Visit(kind.data.binary);
            break;
        case KOOPA_RVT_LOAD:
            result = Visit(kind.data.load);
            break;
        case KOOPA_RVT_STORE:
            result = Visit(kind.data.store);
            break;
        case KOOPA_RVT_ALLOC:
            // 访问 alloc 指令 - alloc指令返回变量的栈地址
            if (value->name) {
                int offset = getVarOffset(value->name);
                result = { stringFormat("%d(sp)", offset) };
            }
            break;
        case KOOPA_RVT_BRANCH:
            result = Visit(kind.data.branch);
            break;
        case KOOPA_RVT_JUMP:
            result = Visit(kind.data.jump);
            break;


        default:
            assert(false);
        }

        // 对于有返回值的指令，如果被多次使用，存储到栈中
        if (value->ty->tag != KOOPA_RTT_UNIT && kind.tag != KOOPA_RVT_RETURN && 
            kind.tag != KOOPA_RVT_STORE && kind.tag != KOOPA_RVT_ALLOC) {
            
            if (!result.empty() && value->used_by.len > 1) {
                // 被多次使用，存储到栈
                int offset = getValueOffset(value);
                getGeneratedInstructions().push_back(
                    stringFormat("sw %s, %d(sp)", result.at(0), offset));
                value_to_register_[value] = stringFormat("%d(sp)", offset);
                return { stringFormat("%d(sp)", offset) };
            } else {
                // 只被使用一次，直接缓存寄存器
                if (!result.empty()) {
                    value_to_register_[value] = result[0];
                }
            }
        }

        return result;
    }

    std::string extractIdentName(const std::string& name)
    {
        // 提取标识符名称，去掉 @ 前缀
        if (name.empty() || (name[0] != '@' && name[0] != '%')) {
            return name; // 如果没有 @ / % 前缀，直接返回
        }
        return name.substr(1); // 去掉 @ / % 前缀
    }

    std::string extractIdentName(const char* name)
    {
        return extractIdentName(std::string(name));
    }

    std::vector<std::string> Visit(const koopa_raw_branch_t& branch)
    {
        const auto& condition = Visit(branch.cond);
        const auto condition_name = condition.at(0) == "0" ? "x0" : condition.at(0);

        // 只生成条件跳转指令，不生成标签
        // 标签应该由函数级别的基本块访问器生成

        getGeneratedInstructions().push_back(
            stringFormat("bnez %s, %s", condition_name, extractIdentName(branch.true_bb->name)));
        getGeneratedInstructions().push_back(
            stringFormat("j %s", extractIdentName(branch.false_bb->name)));
        
        return {}; // 返回空向量，因为指令已经添加到 generated_instructions_ 中
    }

    std::vector<std::string> Visit(const koopa_raw_jump_t& target)
    {
        // 访问 jump 指令 - 跳转到指定的基本块
        // 直接添加跳转指令
        getGeneratedInstructions().push_back(
            stringFormat("j %s", extractIdentName(target.target->name)));
        return {}; // 返回空向量，因为指令已经添加到 generated_instructions_ 中
    }

    std::vector<std::string> Visit(const koopa_raw_integer_t& integer)
    {
        // 返回整数值的字符串表示
        // return { std::to_string(integer.value) };
        // 假设所有用到的数字都需要存在寄存器里（以后再来优化）

        if (integer.value == 0) {
            return { "0" };
        }

        // 其他数字分配寄存器
        int new_var = getNewTempVar();
        getGeneratedInstructions().push_back(
            stringFormat("li t%d, %d", new_var, integer.value));
        return { stringFormat("t%d", new_var) };
    }

    std::vector<std::string> Visit(const koopa_raw_return_t& ret)
    {
        // 访问 return 指令的值
        if (ret.value) {
            auto visited = Visit(ret.value);
            assert(visited.size() == 1);

            if (visited.at(0) == "0") {
                // 如果返回值是 0，则直接使用 x0
                getGeneratedInstructions().push_back("li a0, 0");
            } else if (visited.at(0).find("t") != std::string::npos) {
                // 否则将返回值加载到 a0 寄存器
                getGeneratedInstructions().push_back(
                    stringFormat("mv a0, %s", visited.at(0)));
            } else if (visited.at(0).find("(sp)") != std::string::npos) {
                // 如果是栈上的值，需要先加载
                getGeneratedInstructions().push_back(
                    stringFormat("lw a0, %s", visited.at(0)));
            } else {
                // 如果返回值是数字
                getGeneratedInstructions().push_back(
                    stringFormat("li a0, %s", visited.at(0)));
            }
            
            // 添加函数的 epilogue
            if (total_stack_size_ > 0) {
                if (total_stack_size_ <= 2047) {
                    getGeneratedInstructions().push_back(
                        stringFormat("addi sp, sp, %d", total_stack_size_));
                } else {
                    getGeneratedInstructions().push_back(
                        stringFormat("li t0, %d", total_stack_size_));
                    getGeneratedInstructions().push_back("add sp, sp, t0");
                }
            }
            getGeneratedInstructions().push_back("ret");
            return {}; // 返回空，因为已经添加到指令列表中
        }
        getGeneratedInstructions().push_back("ret");
        return {};
    }

    std::tuple<std::string, std::string> initBinaryArgs(const koopa_raw_binary_t& binary)
    {
        // 访问二元运算指令的操作数
        auto lhs_reg = Visit(binary.lhs);
        auto rhs_reg = Visit(binary.rhs);
        assert(lhs_reg.size() == 1 && rhs_reg.size() == 1);

        std::string lhs_final = lhs_reg.at(0);
        std::string rhs_final = rhs_reg.at(0);

        // 如果操作数是栈上的值，需要先加载到寄存器
        if (lhs_final.find("(sp)") != std::string::npos) {
            int temp_reg = getNewTempVar();
            getGeneratedInstructions().push_back(
                stringFormat("lw t%d, %s", temp_reg, lhs_final));
            lhs_final = stringFormat("t%d", temp_reg);
        }

        if (rhs_final.find("(sp)") != std::string::npos) {
            int temp_reg = getNewTempVar();
            getGeneratedInstructions().push_back(
                stringFormat("lw t%d, %s", temp_reg, rhs_final));
            rhs_final = stringFormat("t%d", temp_reg);
        }

        // 处理0值
        if (lhs_final == "0") {
            lhs_final = "x0";
        }
        if (rhs_final == "0") {
            rhs_final = "x0";
        }

        return { lhs_final, rhs_final };
    }

    std::vector<std::string> Visit(const koopa_raw_load_t& load)
    {
        // 访问 load 指令 - 从内存加载到寄存器
        auto src_addr = Visit(load.src);  // 获取源地址
        if (src_addr.empty()) {
            throw std::runtime_error("Load instruction: source address is empty");
        }
        
        // 分配新的寄存器
        int reg_num = getNewTempVar();
        getGeneratedInstructions().push_back(
            stringFormat("lw t%d, %s", reg_num, src_addr.at(0)));
        return { stringFormat("t%d", reg_num) };
    }

    std::vector<std::string> Visit(const koopa_raw_store_t& store)
    {
        // 访问 store 指令
        auto value_result = Visit(store.value);  // 先获取要存储的值
        auto dest_addr = Visit(store.dest);      // 再获取目标地址
        
        if (value_result.empty() || dest_addr.empty()) {
            throw std::runtime_error("Store instruction: value or destination is empty");
        }
        
        std::string value_reg = value_result.at(0);
        
        // 如果值在栈上，需要先加载到寄存器
        if (value_reg.find("(sp)") != std::string::npos) {
            int temp_reg = getNewTempVar();
            getGeneratedInstructions().push_back(
                stringFormat("lw t%d, %s", temp_reg, value_reg));
            value_reg = stringFormat("t%d", temp_reg);
        }
        
        // 处理 0 值
        if (value_reg == "0") {
            value_reg = "x0";
        }
        
        getGeneratedInstructions().push_back(
            stringFormat("sw %s, %s", value_reg, dest_addr.at(0)));
        return {};  // store指令没有返回值
    }

    std::vector<std::string> Visit(const koopa_raw_binary_t& binary)
    {
        // 访问二元运算指令
        auto [lhs, rhs] = initBinaryArgs(binary);

        // 优先重用左操作数的寄存器（如果它是临时寄存器）
        int new_var = getNewTempVar();
        
        switch (binary.op) {
        case KOOPA_RBO_SUB:
            if (lhs == "x0") {
                // 0 - rhs = -rhs
                getGeneratedInstructions().push_back(
                    stringFormat("sub t%d, x0, %s", new_var, rhs));
            } else if (rhs == "x0") {
                // lhs - 0 = lhs，可以直接返回左操作数
                return { lhs };
            } else {
                // 一般情况
                getGeneratedInstructions().push_back(
                    stringFormat("sub t%d, %s, %s", new_var, lhs, rhs));
            }
            break;
        
        case KOOPA_RBO_ADD:
            // 如果左侧或右侧是 0，直接使用另一个操作数
            if (lhs == "x0") {
                return { rhs };
            } else if (rhs == "x0") {
                return { lhs };
            }
            // 一般情况下的加法
            getGeneratedInstructions().push_back(
                stringFormat("add t%d, %s, %s", new_var, lhs, rhs));
            break;
        
        case KOOPA_RBO_MUL:
            // 如果左侧或右侧是 0，直接返回 0
            if (lhs == "x0" || rhs == "x0") {
                return { "x0" };
            }
            // 如果右侧是 1，直接返回左侧
            if (rhs.find("t") == std::string::npos && rhs != "x0") {
                // 检查是否为立即数 1
                try {
                    if (std::stoi(rhs) == 1) {
                        return { lhs };
                    }
                } catch (...) {
                    // 不是数字，继续正常处理
                }
            }
            // 一般情况下的乘法
            getGeneratedInstructions().push_back(
                stringFormat("mul t%d, %s, %s", new_var, lhs, rhs));
            break;

        case KOOPA_RBO_DIV:
        case KOOPA_RBO_MOD:
            {
                std::string op = (binary.op == KOOPA_RBO_DIV) ? "div" : "rem";
                // 如果左侧是 0，直接返回 0
                if (lhs == "x0") {
                    return { "x0" };
                }
                // 如果右侧是 0，抛出异常或处理错误
                if (rhs == "x0") {
                    throw std::runtime_error("Division by zero error");
                }
                getGeneratedInstructions().push_back(
                    stringFormat("%s t%d, %s, %s", op, new_var, lhs, rhs));
            }
            break;

        case KOOPA_RBO_EQ:
            if (rhs == "x0") {
                // 如果右侧是 0，使用 seqz 指令
                getGeneratedInstructions().push_back(
                    stringFormat("seqz t%d, %s", new_var, lhs));
            } else {
                // 一般情况下的相等比较
                getGeneratedInstructions().push_back(
                    stringFormat("xor t%d, %s, %s", new_var, lhs, rhs));
                getGeneratedInstructions().push_back(
                    stringFormat("seqz t%d, t%d", new_var, new_var));
            }
            break;
        
        case KOOPA_RBO_NOT_EQ:
            if (rhs == "x0") {
                // 如果右侧是 0，使用 snez 指令
                getGeneratedInstructions().push_back(
                    stringFormat("snez t%d, %s", new_var, lhs));
            } else {
                // 一般情况下的不等比较
                getGeneratedInstructions().push_back(
                    stringFormat("xor t%d, %s, %s", new_var, lhs, rhs));
                getGeneratedInstructions().push_back(
                    stringFormat("snez t%d, t%d", new_var, new_var));
            }
            break;

        case KOOPA_RBO_LT:
            getGeneratedInstructions().push_back(
                stringFormat("slt t%d, %s, %s", new_var, lhs, rhs));
            break;

        case KOOPA_RBO_GT:
            getGeneratedInstructions().push_back(
                stringFormat("sgt t%d, %s, %s", new_var, lhs, rhs));
            break;

        case KOOPA_RBO_LE:
            // a <= b 等价于 !(a > b)
            getGeneratedInstructions().push_back(
                stringFormat("sgt t%d, %s, %s", new_var, lhs, rhs));
            getGeneratedInstructions().push_back(
                stringFormat("seqz t%d, t%d", new_var, new_var));
            break;

        case KOOPA_RBO_GE:
            // a >= b 等价于 !(a < b)
            getGeneratedInstructions().push_back(
                stringFormat("slt t%d, %s, %s", new_var, lhs, rhs));
            getGeneratedInstructions().push_back(
                stringFormat("seqz t%d, t%d", new_var, new_var));
            break;
        
        case KOOPA_RBO_AND:
            getGeneratedInstructions().push_back(
                stringFormat("and t%d, %s, %s", new_var, lhs, rhs));
            break;

        case KOOPA_RBO_OR:
            getGeneratedInstructions().push_back(
                stringFormat("or t%d, %s, %s", new_var, lhs, rhs));
            break;

        default:
            assert(false); // 未处理的操作符
        }
        return { stringFormat("t%d", new_var) };
    }
};

// KoopaParser implementations
KoopaParser::KoopaParser()
    : pImpl(std::make_unique<Impl>())
{
}

KoopaParser::~KoopaParser() = default;

const koopa_raw_program_t* KoopaParser::parseToRawProgram(const std::string& input)
{
    return pImpl->parseToRawProgram(input);
}

std::string KoopaParser::compileToAssembly(const std::string& input)
{
    auto raw_program = pImpl->parseToRawProgram(input);
    assert(raw_program != nullptr);

    auto commands = pImpl->Visit(*raw_program);

    std::string assembly;
    for (const auto& command : commands) {
        assembly += command + "\n";
    }

    return assembly;
}
