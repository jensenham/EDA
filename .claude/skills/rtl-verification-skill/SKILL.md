---
name: rtl-verification-skill
description: Verify RTL through simulation and generate verification report
---
# RTL验证技能

## 技能概述
执行仿真验证RTL功能正确性，生成验证报告。需在RTL和Testbench生成后执行。

## 触发条件
- 用户要求"验证"、"仿真"、"运行测试"
- 已存在 `src/verilog/<模块名>.v` 和 `src/testbench/<模块名>_tb.cpp`（C++ testbench）
- 用户完成RTL生成后需要验证

## 执行步骤

### 步骤1：检查必要文件
**目标**：确认RTL和Testbench已生成

**检查项**：
- `src/verilog/<模块名>.v` - RTL源码
- `src/testbench/<模块名>_tb.cpp` - C++测试包装器（**注意**：使用C++ testbench，不需要SV testbench）

**缺失处理**：
- 如RTL缺失：提示用户先执行rtl-generation-skill
- 如Testbench缺失：提示用户先执行testbench-generation-skill

### 步骤2：语法检查
**目标**：确保Verilog无语法错误

**命令**：
```bash
verilator --lint-only \
  --Wall \
  --Wno-style \
  src/verilog/<模块名>.v \
  2>&1 | tee sim/logs/lint_<模块名>.log
```

**判断标准**：
- 有ERROR：验证失败，需修复RTL
- 无ERROR，有WARNING：记录但继续

**输出**：`sim/logs/lint_<模块名>.log`

### 步骤3：编译仿真模型
**目标**：构建可执行的仿真模型

**命令**：
```bash
SIM_BUILD_DIR="sim_build/<模块名>"
mkdir -p "$SIM_BUILD_DIR"
cd "$SIM_BUILD_DIR"

# 注意：必须添加 --build 标志才会生成可执行文件
verilator --cc --exe --build \
  --trace \
  --assert \
  -Wall \
  -Wno-style \
  "../../src/verilog/<模块名>.v" \
  "../../src/testbench/<模块名>_tb.cpp" \
  -o "<模块名>_sim" \
  2>&1 | tee compile.log
```

**输出**：`sim_build/<模块名>/obj_dir/<模块名>_sim`

### 步骤4：执行仿真
**目标**：运行测试并收集结果

**命令**：
```bash
./sim_build/<模块名>/obj_dir/<模块名>_sim 2>&1 | tee sim/logs/sim_<模块名>.log
```

**输出**：`sim/logs/sim_<模块名>.log`

### 步骤5：分析仿真结果
**目标**：解析仿真输出，判断测试通过/失败

**操作**：
1. 检查是否有仿真崩溃（Segmentation Fault等）
2. 解析输出中的 `[TEST N]` 行
3. 统计 PASS/FAIL 数量
4. 检查最终 `RESULT:` 行

**判断标准**：
- `RESULT: PASSED X/X tests` → 全部通过
- `RESULT: FAILED X/X tests` → 有失败
- 无RESULT行 → 仿真异常

### 步骤6：生成验证报告
**目标**：汇总验证结果，生成Markdown报告

**报告格式**：
```markdown
# <模块名> 验证报告
生成时间: <timestamp>

## 1. 模块信息
- 模块名称: <模块名>
- RTL源码: `src/verilog/<模块名>.v`
- Testbench: `src/testbench/<模块名>_tb.cpp`（C++ testbench）

## 2. 语法检查结果
- 状态: <通过/失败>
- 无ERROR: <是/否>
- WARNING数: <N>

## 3. 仿真验证结果
### 测试结果摘要
| 测试项 | 状态 |
|--------|------|
| <OP1> | <PASS/FAIL> |
| <OP2> | <PASS/FAIL> |

### 详细结果
```
<仿真输出最后20行>
```

### 统计
- 总测试数: <N>
- 通过: <M>
- 失败: <K>

## 4. 总结
- 总体状态: <✅ 通过 / ❌ 失败>
- 下一阶段建议: <如有问题，给出修复建议>
```

**输出**：`docs/verification_<模块名>.md`

## 错误处理

### 仿真崩溃
- 检查复位时序是否正确
- 检查端口连接是否匹配
- 检查是否存在未初始化的信号

### 测试失败
- 对比 expect 和 got 值
- 检查操作码定义是否一致
- 检查时钟沿采样是否正确

### 仿真挂起
- 添加超时机制
- 检查是否存在死循环
- 验证复位释放序列

## 完整TDD流程

```
1. testbench-generation-skill  → 生成测试（应失败）
2. rtl-generation-skill        → 生成RTL
3. rtl-verification-skill     → 验证（应通过）
```
