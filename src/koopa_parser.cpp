#include "koopa_parser.h"

#include <cassert>
#include <memory>
#include <string>
#include "koopa.h"

// RAII wrapper for koopa_program_t
class KoopaProgram {
public:
    KoopaProgram() = default;

    ~KoopaProgram() {
        if (program_ != nullptr) {
            koopa_delete_program(program_);
        }
    }

    // 禁用拷贝
    KoopaProgram(const KoopaProgram&) = delete;
    KoopaProgram& operator=(const KoopaProgram&) = delete;

    // 允许移动
    KoopaProgram(KoopaProgram&& other) noexcept : program_(other.program_) {
        other.program_ = nullptr;
    }

    KoopaProgram& operator=(KoopaProgram&& other) noexcept {
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
    KoopaRawProgramBuilder() : builder_(koopa_new_raw_program_builder()) {
        assert(builder_ != nullptr);
    }

    ~KoopaRawProgramBuilder() {
        if (builder_ != nullptr) {
            koopa_delete_raw_program_builder(builder_);
        }
    }

    // 禁用拷贝
    KoopaRawProgramBuilder(const KoopaRawProgramBuilder&) = delete;
    KoopaRawProgramBuilder& operator=(const KoopaRawProgramBuilder&) = delete;

    // 允许移动
    KoopaRawProgramBuilder(KoopaRawProgramBuilder&& other) noexcept : builder_(other.builder_) {
        other.builder_ = nullptr;
    }

    KoopaRawProgramBuilder& operator=(KoopaRawProgramBuilder&& other) noexcept {
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
    koopa_raw_program_t raw_program_{};

public:
    const koopa_raw_program_t* parseToRawProgram(const std::string& input) {
        // 解析输入字符串为 program
        koopa_error_code_t ret = koopa_parse_from_string(input.c_str(), program_.get());
        assert(ret == KOOPA_EC_SUCCESS);

        // 将 Koopa IR 程序转换为 raw program
        raw_program_ = koopa_build_raw_program(builder_.get(), *program_.get());

        return &raw_program_;
    }
};

// KoopaParser implementations
KoopaParser::KoopaParser() : pImpl(std::make_unique<Impl>()) {}

KoopaParser::~KoopaParser() = default;

const koopa_raw_program_t* KoopaParser::parseToRawProgram(const std::string& input) {
    return pImpl->parseToRawProgram(input);
}
