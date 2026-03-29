---
name: testbench-generation-skill
description: Generate C++ testbench and simulation scripts for RISC-V modules
---
# Testbench生成技能

## 技能概述
根据需求规格文档，生成 C++ 测试平台和仿真脚本。遵循 TDD 原则：先写测试，再写RTL。

**重要**：本技能只生成 C++ testbench（Verilator 原生），不使用 SystemVerilog testbench。

## 触发条件
- 用户要求"生成Testbench"、"写测试"、"TDD"
- 用户提供了需求规格文档 `docs/requirements_<模块名>.md`
- 用户要求"先写测试"

## 执行步骤

### 步骤1：分析需求规格
**目标**：从需求文档提取测试所需信息

**操作**：
1. 读取 `docs/requirements_<模块名>.md`
2. 提取：
   - 模块名称和接口信号
   - 功能列表（操作类型）
   - 数据位宽（默认32位）
   - 特殊要求（zero标志位等）

**输出**：清晰的测试需求清单

### 步骤2：生成C++测试包装器
**目标**：生成Verilator仿真所需的C++测试台

**操作**：
1. 创建 `src/testbench/<模块名>_tb.cpp`
2. **必须使用 Verilog 信号名**（Chisel `io_a` → Verilog `io_io_a`）
3. 包含：
   - `V<模块名>.h` 头文件（Verilator 生成）
   - `verilated.h`
   - 时钟和复位初始化
   - 测试用例和结果验证

**信号名映射（重要）**：
```
Chisel 信号名          →  Verilog/C++ 信号名
-----------------------------------------
io_a                  →  io_io_a
io_b                  →  io_io_b
io_opcode             →  io_io_opcode
io_result             →  io_io_result
io_zero               →  io_io_zero
clock                 →  clock
reset                 →  reset
```

**C++ Testbench模板**：
```cpp
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "verilated.h"
#include "V<模块名>.h"

// 操作码定义（必须与Chisel模块一致）
#define OP_ADD 0
#define OP_SUB 1
#define OP_AND 2
#define OP_OR  3
#define OP_XOR 4
#define OP_SLT 5

const char* op_name[] = {"ADD", "SUB", "AND", "OR", "XOR", "SLT"};

int test_count = 0;
int pass_count = 0;

void test_op(V<模块名>* dut, int op, uint32_t a, uint32_t b, uint32_t expect) {
  // 应用激励
  dut->io_io_a = a;
  dut->io_io_b = b;
  dut->io_io_opcode = op;

  // 时钟 toggles
  dut->clock = !dut->clock;
  dut->eval();
  dut->clock = !dut->clock;
  dut->eval();

  uint32_t got = dut->io_io_result;

  test_count++;
  if (got == expect) {
    printf("[TEST %d] %s: a=0x%x b=0x%x | expect=0x%x got=0x%x | PASS\n",
           test_count, op_name[op], a, b, expect, got);
    pass_count++;
  } else {
    printf("[TEST %d] %s: a=0x%x b=0x%x | expect=0x%x got=0x%x | FAIL\n",
           test_count, op_name[op], a, b, expect, got);
  }
}

int main(int argc, char** argv) {
  Verilated::commandArgs(argc, argv);
  Verilated::traceEverOn(true);

  V<模块名>* dut = new V<模块名>;

  // 初始化
  dut->clock = 0;
  dut->reset = 1;
  dut->io_io_a = 0;
  dut->io_io_b = 0;
  dut->io_io_opcode = 0;

  // 复位
  for (int i = 0; i < 5; i++) {
    dut->clock = !dut->clock;
    dut->eval();
  }
  dut->reset = 0;

  // 测试用例调用
  // test_op(dut, OP_ADD, 5, 3, 8);
  // ...

  printf("RESULT: %s %d/%d tests\n",
         pass_count == test_count ? "PASSED" : "FAILED",
         pass_count, test_count);

  dut->final();
  delete dut;
  return 0;
}
```

**输出**：`src/testbench/<模块名>_tb.cpp`

### 步骤3：生成仿真脚本
**目标**：创建可重复执行的仿真脚本

**操作**：
生成 `sim/scripts/sim_<模块名>.sh`：
```bash
#!/bin/bash
MODULE_NAME="<模块名>"

# 1. 检查文件
# 2. 语法检查
# 3. 编译仿真模型（必须加 --build）
# 4. 执行仿真
# 5. 收集结果
```

**注意**：编译时必须使用 `--build` 标志：
```bash
verilator --cc --exe --build \
  src/verilog/<模块名>.v \
  src/testbench/<模块名>_tb.cpp \
  -o sim_build/<模块名>/obj_dir/<模块名>_sim
```

**输出**：`sim/scripts/sim_<模块名>.sh`

## TDD工作流

```
1. 调用 testbench-generation-skill 生成测试
   → 此时RTL尚未生成或为空壳，仿真应失败（预期行为）

2. 调用 rtl-generation-skill 生成RTL
   → 根据Testbench的接口期望实现功能

3. 调用 rtl-verification-skill 验证
   → Testbench应全部通过
```

## 质量要求

### 测试覆盖率
- 每个操作码至少2个测试用例
- 边界条件：0、最大值、溢出、负数（有符号）
- zero标志位测试（结果为0和不为0）

### 输出格式
必须符合验证格式标准：
```
[TEST 1] ADD: a=0x5 b=0x3 | expect=0x8 got=0x8 | PASS
[TEST 2] SUB: a=0xa b=0x7 | expect=0x3 got=0x3 | PASS
...
RESULT: PASSED 20/20 tests
```

### 常见错误处理
- **信号名错误**：`dut->io_a` 编译报错 → 应使用 `dut->io_io_a`
- **端口连接错误**：检查 Verilog 端口列表，使用正确的信号名
- **仿真超时**：检查复位序列和时钟生成
