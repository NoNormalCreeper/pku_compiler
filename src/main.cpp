#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

#include "ast.h"
#include "koopa_parser.h"
#include "symbol_table.h"

using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE* yyin;
extern int yyparse(unique_ptr<BaseAST>& ast);

int main(int argc, const char* argv[])
{
    // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
    // compiler 模式 输入文件 -o 输出文件
    assert(argc == 5);
    auto mode = argv[1];
    auto input = argv[2];
    auto output = argv[4];

    // 初始化全局符号表
    SymbolTable global_symbol_table;
    BaseAST::global_symbol_table = &global_symbol_table;

    // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
    yyin = fopen(input, "r");
    assert(yyin);

    // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
    unique_ptr<BaseAST> ast;
    auto ret = yyparse(ast);
    assert(!ret);

    // 输出解析得到的 AST, 其实就是个字符串
    //   cout << *ast << endl;
    // dump AST
    ast->Dump();
    cout << endl;
    
    auto koopa_code = ast->toKoopa();
    cout << koopa_code;
    cout << endl;

    // 写入输出文件
    FILE* out = fopen(output, "w");
    assert(out);
    
    assert(!koopa_code.empty());

    auto mode_str = string(mode);
    cout << "Mode: " << mode_str << endl;
    if (mode_str == "-koopa") {
        fprintf(out, "%s", koopa_code.c_str());
    } else if (mode_str == "-riscv") {
        auto koopa_parser = make_unique<KoopaParser>();
        // const auto* raw_program = koopa_parser->parseToRawProgram(koopa_code);
        // assert(raw_program);
        auto assembly = koopa_parser->compileToAssembly(koopa_code);

        cout << assembly << endl;
        fprintf(out, "%s", assembly.c_str());
    } else {
        cerr << "Unknown mode: " << mode_str << endl;
        fclose(out);
        return 1;
    }
    fclose(out);

    return 0;
}