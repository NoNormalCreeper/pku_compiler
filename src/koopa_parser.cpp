#include "koopa_parser.h"

#include "koopa.h"
#include "string_format.h"
#include <cassert>
#include <memory>
#include <string>
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

public:
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

    // 访问函数
    std::vector<std::string> Visit(const koopa_raw_function_t& func)
    {
        // 执行一些其他的必要操作
        // ...
        // 访问所有基本块
        std::vector<std::string> result;
        
        // Extract function name (remove @ prefix if present)
        std::string func_name = func->name;
        if (func_name[0] == '@') {
            func_name = func_name.substr(1);
        }
        
        result.push_back(stringFormat("%s:", func_name));

        const auto bbs = Visit(func->bbs);
        for (const auto& bb : bbs) {
            result.push_back(stringFormat("  %s", bb));
        }
        return result;
    }

    // 访问基本块
    std::vector<std::string> Visit(const koopa_raw_basic_block_t& bb)
    {
        // 执行一些其他的必要操作
        // ...
        // 访问所有指令
        return Visit(bb->insts);
    }

    // 访问指令
    std::vector<std::string> Visit(const koopa_raw_value_t& value)
    {
        // 根据指令类型判断后续需要如何访问
        const auto& kind = value->kind;
        switch (kind.tag) {
        case KOOPA_RVT_RETURN:
            // 访问 return 指令
            return Visit(kind.data.ret);
            break;
        case KOOPA_RVT_INTEGER:
            // 访问 integer 指令
            return Visit(kind.data.integer);
            break;
        default:
            // 其他类型暂时遇不到
            assert(false);
        }
    }

    std::vector<std::string> Visit(const koopa_raw_integer_t& integer)
    {
        // 返回整数值的字符串表示
        return { std::to_string(integer.value) };
    }

    std::vector<std::string> Visit(const koopa_raw_return_t& ret)
    {
        // 访问 return 指令的值
        if (ret.value) {
            auto visited = Visit(ret.value);
            assert(visited.size() == 1);

            return { stringFormat("li a0, %s", Visit(ret.value).at(0)), "ret" };
        }
        return { "ret" };
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
