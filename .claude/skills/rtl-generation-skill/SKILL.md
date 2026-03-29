---
name: rtl-generation-skill
description: Generate RTL (Chisel → Verilog) for RISC-V modules
---
# RTL生成技能

## 技能概述
根据自然语言需求生成RISC-V模块的RTL代码。遵循TDD原则：应与testbench-generation-skill配合使用，先有测试再有RTL。

## 触发条件
- 用户输入包含"生成RTL"、"生成ALU"、"生成寄存器"等关键词
- 用户明确要求生成硬件模块
- 用户描述RISC-V相关功能需求

## 执行步骤

### 步骤1：需求解析与澄清
**目标**：理解用户需求，生成规范化需求文档

**操作**：
1. 分析用户自然语言描述
2. 提取关键信息：
   - 模块名称
   - 功能列表
   - 接口要求
   - 性能要求
3. 如需求不明确，主动询问：
   - "请确认数据位宽（默认32位）"
   - "需要哪些控制信号？"
   - "有哪些特殊功能要求？"
4. 生成需求规格文档

**输出**：`docs/requirements_<模块名>.md`

### 步骤2：Chisel代码生成
**目标**：生成符合规范的Chisel代码

**操作**：
1. 根据需求规格设计模块架构
2. 生成Chisel源代码：
   - 包声明和导入
   - 模块类定义
   - IO接口定义
   - 内部逻辑实现
3. 确保代码符合RV32I标准
4. 添加适当注释

**输出**：`src/chisel/<模块名>.scala`

### 步骤2.5：Chisel编译检查
**目标**：验证生成的Chisel代码可编译

**操作**：
1. 执行`sbt compile`编译Chisel代码
2. 如编译失败，分析错误并修复Chisel代码
3. 重新编译直到成功

**命令**：
```bash
cd "$(dirname "$0")/../../"
sbt compile 2>&1 | tee sim/logs/compile_<模块名>.log
```

**错误处理**：
- 如编译失败，定位错误位置
- 修复Chisel代码
- 重新编译验证

**输出**：`sim/logs/compile_<模块名>.log`

### 步骤3：编译与转换
**目标**：将Chisel编译为Verilog

**操作**：
1. 检查并创建必要的目录结构
2. 执行`sbt compile`编译Chisel代码
3. 使用CIRCT将Chisel转换为Verilog
4. 验证转换结果
5. **输出端口映射表**（供testbench使用）

**命令**：
```bash
cd "$(dirname "$0")/../../"

# 生成Verilog（使用CIRCT）
sbt "runMain circt.stage.ChiselMain --module riscv.<模块名> --target-dir generated --target verilog"

# 复制到src/verilog/
cp generated/<模块名>.v src/verilog/<模块名>.v

# 读取生成的Verilog，提取端口列表
grep "^  input\|^  output" src/verilog/<模块名>.v
```

**端口映射输出示例**：
```
RTL 端口映射 (Verilog信号名):
  input  clock       → clock
  input  reset      → reset
  input  [31:0] io_io_a      → io_io_a
  input  [31:0] io_io_b      → io_io_b
  input  [3:0]  io_io_opcode → io_io_opcode
  output [31:0] io_io_result → io_io_result
  output        io_io_zero   → io_io_zero
```

**输出**：`src/verilog/<模块名>.v`

### 步骤4：语法检查
**目标**：检查生成的Verilog语法

**操作**：
1. 使用Verilator进行语法检查
2. 分析检查结果
3. 如发现ERROR，尝试修复
4. 记录WARNING信息

**命令**：
```bash
verilator --lint-only \
  --Wall \
  --Wno-style \
  src/verilog/<模块名>.v \
  2>&1 | tee sim/logs/lint_<模块名>.log
```

**输出**：`sim/logs/lint_<模块名>.log`

## 错误处理

### 常见错误及处理
1. **编译错误**：
   - 检查Chisel语法
   - 验证导入语句
   - 检查依赖版本

2. **语法检查错误**：
   - 分析Verilator错误信息
   - 修复语法问题
   - 重新检查

## TDD工作流

```
1. 用户提供需求
2. 调用 testbench-generation-skill 生成测试（先有测试）
3. 调用 rtl-generation-skill 生成RTL（再写实现）
4. 调用 rtl-verification-skill 验证（最后验证）
```

## 质量保证

### 代码质量
- 遵循编码规范
- 添加适当注释
- 模块化设计

### 接口标准（RV32I）
- 32位数据宽度
- `clock` 和 `reset` 信号
- IO bundle命名：`io_<signal>`
- 操作码通过opcode输入控制
