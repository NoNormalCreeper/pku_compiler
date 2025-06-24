#pragma once

#include <string>
#include <memory>

#include "koopa.h"

// Forward declarations
// struct koopa_raw_program_t;

class KoopaParser {
public:
    KoopaParser();
    ~KoopaParser();
    
    // 禁用拷贝构造和赋值
    KoopaParser(const KoopaParser&) = delete;
    KoopaParser& operator=(const KoopaParser&) = delete;
    
    // 允许移动构造和赋值
    KoopaParser(KoopaParser&&) = default;
    KoopaParser& operator=(KoopaParser&&) = default;
    
    const koopa_raw_program_t* parseToRawProgram(const std::string& input);
    std::string compileToAssembly(const std::string& input);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};


