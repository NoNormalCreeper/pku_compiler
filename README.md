# pku_compiler
个人学习 北大编译原理实践 课程的项目文件

# 进度

## Lv1. `main` 函数
`WRONG_ANSWER (6/7)`
> 发现是没有添加处理 block comments 的规则

**`PASSED (7/7)`**

## Lv2. 初试目标代码生成
**`PASSED (7/7)`**

## Lv3. 表达式

`WRONG ANSWER (19/28)`

```
running test "00_pos" ... PASSED
running test "01_neg_0" ... PASSED
running test "02_neg_2" ... PASSED
running test "03_neg_max" ... PASSED
running test "04_not_0" ... PASSED
running test "05_not_10" ... PASSED
running test "06_complex_unary" ... PASSED
running test "07_add" ... PASSED
running test "08_add_neg" ... PASSED
running test "09_sub" ... PASSED
running test "10_sub_neg" ... PASSED
running test "11_mul" ... PASSED
running test "12_mul_neg" ... PASSED
running test "13_div" ... PASSED
running test "14_div_neg" ... PASSED
running test "15_mod" ... PASSED
running test "16_mod_neg" ... PASSED
...
running test "25_int_min" ... PASSED
running test "26_parentheses" ... PASSED
```

- [x] Lv3.1. 一元表达式
- [x] Lv3.2. 算数表达式
- [ ] Lv3.3. 比较和逻辑表达式
