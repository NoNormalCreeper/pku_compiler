# pku_compiler
个人学习 北大编译原理实践 课程的项目文件

[![wakatime](https://wakatime.com/badge/user/595d4312-2ac6-4d72-a323-befb05c7c3ac/project/5abb7d3a-d43e-46e0-ac2f-eefe866f701e.svg)](https://wakatime.com/badge/user/595d4312-2ac6-4d72-a323-befb05c7c3ac/project/5abb7d3a-d43e-46e0-ac2f-eefe866f701e)

# 进度

## Lv1. `main` 函数
`WRONG_ANSWER (6/7)`
> 发现是没有添加处理 block comments 的规则

**`PASSED (7/7)`**

## Lv2. 初试目标代码生成
**`PASSED (7/7)`**

## Lv3. 表达式

`WRONG ANSWER (27/28)`

> 最后一个测试点错误应该是开了过多寄存器导致的，需要进行优化，有些表达式可以不开新的寄存器，直接存到其中一个。

**`PASSED (28/28)`**

- [x] Lv3.1. 一元表达式
- [x] Lv3.2. 算术表达式
- [x] Lv3.3. 比较和逻辑表达式

## Lv4. 常量和变量

> 改了好久的寄存器分配逻辑

**`PASSED (14/14)`**

- [x] Lv4.1 常量
- [x] Lv4.2 变量和赋值

## Lv5. 语句块和作用域

> 有些地方还是没有正确使用 `x0` 寄存器代替 `0` 值，以后慢慢重构吧（

**`PASSED (7/7)`**

## Lv6. `if` 语句

> 顺带修复了一下之前没注意到的 IR 生成中，嵌套作用域导致的变量名冲突问题，和多个 return 导致基本块结束指令重复的问题。

> 感觉上面说的 `0` 和 `x0` 问题需要一个统一的解决方案，而不是每个地方都要进行特判。

**`PASSED (8/8)`**

- [X] Lv6.1. 处理 `if/else`
    - [X] IR 生成
    - [X] 目标代码生成
- [X] Lv6.2. 短路求值

# Lv7. `while` 语句

- [ ] Lv7.1. 处理 `while`
- [ ] Lv7.2. `break` 和 `continue`

# Lv8. 函数和全局变量


# Lv9. 数组
