#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <stack>
#include <unordered_map>

enum class SymbolType {
    CONST,      // 常量
    VAR,        // 变量
    FUNC        // 函数
};

class SymbolTableItem {
public:
    SymbolType symbol_type; // 符号类型
    std::string type; // 数据类型 (int, void等)
    std::string identifier; // 标识符
    std::optional<int> value; // 可选的值，用于常量
    std::optional<int> scope_identifier; // 作用域标识符，用于区分同名变量，构造时不初始化
    bool is_const; // 是否为常量

    // 默认构造函数
    SymbolTableItem() 
        : symbol_type(SymbolType::VAR), type(""), identifier(""), value(std::nullopt), is_const(false) {}

    SymbolTableItem(SymbolType sym_type, const std::string& data_type, const std::string& identifier, 
                   std::optional<int> value = std::nullopt, bool is_const = false)
        : symbol_type(sym_type), type(data_type), identifier(identifier), value(value), is_const(is_const) {}
    
    auto operator==(const SymbolTableItem& other) const {
        return symbol_type == other.symbol_type && type == other.type && 
               identifier == other.identifier && value == other.value && is_const == other.is_const;
    }
    
    auto operator!=(const SymbolTableItem& other) const{
        return !(*this == other);
    }
};

// 支持作用域的符号表
class SymbolTable {
private:
    // 使用栈来管理嵌套的作用域，每个作用域是一个哈希表
    std::stack<std::unordered_map<std::string, SymbolTableItem>> scopes;
    int current_scope_level = 0; // 当前作用域级别

public:
    SymbolTable() {
        // 初始化全局作用域
        enterScope();
    }

    // 进入新的作用域
    void enterScope();
    
    // 退出当前作用域
    void exitScope();
    
    // 在当前作用域添加符号
    bool addSymbol(SymbolTableItem& item);
    
    // 查找符号（从当前作用域向外查找）
    std::optional<SymbolTableItem> getSymbol(const std::string& identifier) const;

    // 获取当前作用域的编号
    int getCurrentScopeLevel() const {
        return static_cast<int>(scopes.size());
    }
    
    // 检查当前作用域是否已存在该标识符
    bool existsInCurrentScope(const std::string& identifier) const;

    // 兼容旧接口
    void addItem(SymbolTableItem& item) { addSymbol(item); }
    std::optional<SymbolTableItem> getItem(const std::string& identifier) const { return getSymbol(identifier); }
};