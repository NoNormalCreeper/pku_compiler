#include "symbol_table.h"

// 定义静态成员变量
int SymbolTable::global_variable_counter = 0;

void SymbolTable::enterScope() {
    scopes.emplace(); // 添加新的作用域
    current_scope_level++;
}

void SymbolTable::exitScope() {
    if (!scopes.empty()) {
        scopes.pop(); // 移除当前作用域
        current_scope_level--;
    }
}

bool SymbolTable::addSymbol(SymbolTableItem& item) {
    if (scopes.empty()) {
        return false; // 没有作用域
    }
    
    auto& current_scope = scopes.top();

    // 设置唯一的作用域标识符
    item.scope_identifier = getNextGlobalVariableId();
    
    // 检查当前作用域是否已存在该标识符
    if (current_scope.find(item.identifier) != current_scope.end()) {
        return false; // 重复定义
    }
    
    current_scope[item.identifier] = item;
    return true;
}

std::optional<SymbolTableItem> SymbolTable::getSymbol(const std::string& identifier) const {
    // 从当前作用域开始向外层查找
    std::stack<std::unordered_map<std::string, SymbolTableItem>> temp_stack = scopes;
    
    while (!temp_stack.empty()) {
        const auto& current_scope = temp_stack.top();
        auto it = current_scope.find(identifier);
        if (it != current_scope.end()) { 
            return it->second;
        }
        temp_stack.pop();
    }
    
    return std::nullopt; // 未找到
}

bool SymbolTable::existsInCurrentScope(const std::string& identifier) const {
    if (scopes.empty()) {
        return false;
    }
    
    const auto& current_scope = scopes.top();
    return current_scope.find(identifier) != current_scope.end();
}