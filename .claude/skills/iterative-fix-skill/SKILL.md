---
name: iterative-fix-skill
description: 迭代修复 RTL 代码和测试中的错误
---
# 迭代修复技能

## 技能概述

在 RTL 开发过程中，自动化修复编译错误、仿真失败和 lint 警告。遵循"分析错误→修复→验证"的迭代流程。

## 触发条件

- Chisel/Verilog 编译失败
- 仿真结果与预期不符
- Verilator lint 报告 ERROR
- 用户要求"修复"、"调试"、"迭代"

## 执行步骤

### 步骤1：错误分类

根据错误来源分类：

| 错误类型 | 来源 | 典型表现 |
|----------|------|----------|
| 编译错误 | Chisel 语法 | `sbt compile` 失败 |
| 转换错误 | firtool | FIRRTL → Verilog 失败 |
| Lint 错误 | Verilator | `verilator --lint-only` 报告 ERROR |
| 仿真错误 | RTL 逻辑 | 仿真结果与预期不符 |

### 步骤2：分析与定位

**Chisel 编译错误**：
1. 读取错误信息
2. 定位到具体文件和行号
3. 分析错误原因

**Verilator lint 错误**：
1. 读取 `sim/logs/lint_*.log`
2. 定位 Verilog 文件和行号
3. 分析语法或语义问题

**仿真失败**：
1. 读取 `sim/logs/sim_*.log`
2. 对比预期值与实际值
3. 确定是哪部分逻辑出错

### 步骤3：自动修复

**Chisel 修复策略**：
- 类型不匹配 → 添加 `.asUInt()`、`.asBool()` 等类型转换
- 端口连接错误 → 检查 IO bundle 连接
- 缺少依赖 → 检查 import 语句

**Verilog 修复策略**：
- 语法错误 → 修复对应行
- 信号未定义 → 检查端口声明
- 位宽不匹配 → 添加位宽扩展或截断

**仿真修复策略**：
- 结果错误 → 检查 RTL 逻辑实现
- 时序问题 → 添加必要的寄存器
- 信号命名错误 → 确认 testbench 使用正确的 Verilog 信号名

### 步骤4：验证修复

修复后重新执行：

```bash
# Chisel 编译
sbt compile

# Verilog lint
verilator --lint-only --Wall --Wno-style src/verilog/<模块名>.v

# 仿真
bash sim/scripts/sim_<模块名>.sh
```

### 步骤5：迭代记录

记录修复历史到 `docs/fix_log_<模块名>.md`：

```markdown
# <模块名> 修复日志

## 迭代 1
- **时间**：2026-03-28
- **错误**：Chisel 编译失败 - 类型不匹配
- **原因**：UInt 与 SInt 操作混用
- **修复**：添加 `.asSInt()` 类型转换
- **状态**：✅ 通过

## 迭代 2
- **时间**：2026-03-28
- **错误**：仿真 SLT 结果错误
- **原因**：有符号比较逻辑错误
- **修复**：修正比较条件
- **状态**：✅ 通过
```

## 错误处理策略

| 错误类别 | 最大迭代次数 | 失败处理 |
|----------|-------------|----------|
| Chisel 编译 | 3 | 回滚并报告用户 |
| Verilog lint | 2 | 自动修复 |
| 仿真失败 | 5 | 分析后修复 RTL |

## 输出

- 修复后的源代码文件
- `docs/fix_log_<模块名>.md` - 修复日志

## 质量保证

- 修复后必须通过编译
- lint 不得有 ERROR
- 仿真必须通过所有测试用例
- 保留修复历史便于追溯
