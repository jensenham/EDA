# CLAUDE.md

本文件为 Claude Code (claude.ai/code) 在此代码库中工作提供指导。

## 项目概述

这是一个基于 Claude Code 的 RISC-V RTL 自动生成项目。系统接收自然语言需求，自动生成、编译和验证 RISC-V 模块 RTL。

**核心工作流**：自然语言需求 → **全自动** → Chisel 代码 → FIRRTL → Verilog → Verilator lint → Testbench → 仿真 → 验证报告

## 构建命令

```bash
# 编译 Chisel 代码
sbt compile

# 使用 CIRCT 从 Chisel 生成 Verilog（输出到 generated/）
sbt "runMain circt.stage.ChiselMain --module riscv.<模块名> --target-dir generated --target verilog"

# 复制生成的 Verilog 到 src/verilog/
cp generated/<模块名>.v src/verilog/<模块名>.v

# Verilog lint（不允许 ERROR）
verilator --lint-only --Wall --Wno-style src/verilog/<模块名>.v

# 运行仿真
bash sim/scripts/sim_<模块名>.sh
```

**注意**：所有 Chisel 模块必须在 `package riscv` 中。模块类名在命令中作为 `<模块名>`（例如 `src/chisel/ALU.scala` 对应 `riscv.ALU`）。

### Chisel 到 Verilog 信号名称映射（关键）

Chisel 生成 Verilog 时，信号名称会发生变化：
- Chisel 中的 `io_xxx` → Verilog 中的 `io_io_xxx`（双 io 前缀！）
- `clock` → `clock`
- `reset` → `reset`

**C++ testbench 必须使用 Verilog 信号名称**，而非 Chisel 名称：
```cpp
// 正确：
dut->io_io_a = 5;
dut->io_io_result;  // 读取结果
dut->clock = !dut->clock;

// 错误（信号找不到）：
dut->io_a = 5;      // ❌
dut->io_result;     // ❌
```

## Claude Code 配置

`.claude/` 目录包含 AI 配置：

- `settings.json` — 规则配置
- `rules/rtl-generation-rules.md` — RTL 生成规则
- `skills/rtl-generation-skill/SKILL.md` — RTL 生成（Chisel → Verilog）
- `skills/testbench-generation-skill/SKILL.md` — Testbench 生成（TDD：先写测试）
- `skills/rtl-verification-skill/SKILL.md` — 验证和仿真

**注意**：`rtl-verification-skill` 目录名有个拼写错误（多了个 c），但 SKILL.md frontmatter 中的 skill 名称是正确的。

**调用技能**：使用 `/skill rtl-generation-skill`、`/skill testbench-generation-skill` 或 `/skill rtl-verification-skill`。**当请求模块生成时，系统自动按序调用所有技能完成完整流程**。

## 架构

### 工具链
- **Chisel 6.0.0**：硬件构造语言（基于 Scala）
- **sbt**：Scala 构建工具（scalaVersion 2.13.12）
- **firtool**：FIRRTL 编译器 → Verilog
- **Verilator >= 4.200**：Lint 和仿真

### TDD 工作流（4 步）

遵循 TDD 原则：**用户只需描述需求，系统自动完成"需求理解→测试→实现→验证"的完整流程**。

0. **需求理解** → 解析自然语言，必要时澄清，生成结构化需求文档
1. **testbench-generation-skill** → `/skill testbench-generation-skill` — 生成测试台（预期失败）
2. **rtl-generation-skill** → `/skill rtl-generation-skill` — 生成 RTL（应通过测试）
3. **rtl-verification-skill** → `/skill rtl-verification-skill` — 运行仿真并生成报告

### 流水线流程

**完整自动化流程**：当用户请求生成模块时（如"生成 ALU"），应自动执行以下完整 TDD 流程，无需用户干预：

**0. 需求理解（需求澄清）**：
   - 解析用户的自然语言描述，提取：模块名称、接口信号、功能列表
   - 识别 RV32I 相关术语（ALU、寄存器文件、控制单元等）
   - 需求不明确时，主动向用户澄清：数据位宽、接口信号要求、性能要求、特殊功能需求
   - 生成结构化需求文档 → `docs/requirements_<模块>.md`（如尚未存在）

**1. testbench-generation-skill（5 步）**：
   - 分析需求文档
   - 生成 SystemVerilog 测试台 → `src/testbench/<模块>_tb.sv`
   - 生成 C++ 包装器 → `src/testbench/<模块>_tb.cpp`
   - 使用 verilator 编译测试台（`--cc --exe`）
   - 生成仿真脚本 → `sim/scripts/sim_<模块>.sh`

**2. rtl-generation-skill（3 步）**：
   - 读取已有需求文档 `docs/requirements_<模块>.md`
   - Chisel 代码生成 → `src/chisel/<模块>.scala`
   - Chisel 编译检查（`sbt compile`）
   - FIRRTL → Verilog：`sbt runMain` → `generated/*.fir`，然后 `firtool` → `src/verilog/<模块>.v`

**3. rtl-verification-skill（6 步）**：
   - 检查 RTL 和 testbench 是否存在
   - Verilator lint（`verilator --lint-only`）
   - 编译仿真模型（`verilator --cc --exe --build`）
   - 运行仿真（`./sim_build`）
   - 分析结果
   - 生成报告 → `docs/verification_<模块>.md`

**关键原则**：用户只需描述需求（如"生成一个支持 ADD/SUB/AND/OR/XOR/SLT 的 ALU"），系统自动完成从需求到验证的全部流程。

### 关键工具命令
```bash
# 完整模块生成（按顺序调用 3 个技能）
# 1. testbench: /skill testbench-generation-skill
# 2. rtl: /skill rtl-generation-skill
# 3. verification: /skill rtl-verification-skill

# 手动工作流
sbt compile                                           # 编译 Chisel
sbt "runMain circt.stage.ChiselMain --module riscv.<模块> --target-dir generated --target verilog"
cp generated/<模块>.v src/verilog/<模块>.v           # 复制到 verilog 目录
verilator --lint-only --Wall --Wno-style src/verilog/<模块>.v
bash sim/scripts/sim_<模块>.sh                       # 运行仿真
```

### 关键目录
- `src/chisel/` — Chisel 源代码（scalaVersion 2.13.12, Chisel 6.0.0）
- `src/verilog/` — 生成的 Verilog
- `src/testbench/` — SystemVerilog testbench + C++ 包装器
- `sim/logs/` — Lint 和仿真日志
- `sim/scripts/` — 仿真脚本（`sim_<模块>.sh`）
- `generated/` — firtool/ChiselMain 输出的 FIRRTL
- `docs/` — 需求文档和验证报告

### RV32I 接口标准

所有模块使用：
- 32 位数据宽度
- `clock` 和 `reset` 信号
- IO bundle 命名：`io_<信号>`（例如 `io_a`、`io_result`）
- 通过 opcode 输入选择操作（例如 `4.W` 最多支持 16 种操作）

## 验证格式

Lint：`verilator --lint-only src/verilog/*.v` — 不允许 ERROR

仿真输出格式：
```
[TEST 1] ADD: a=5 b=3 | expect=8 got=8 | PASS
RESULT: PASSED X/X tests
```

报告：`docs/verification_<模块名>.md`
