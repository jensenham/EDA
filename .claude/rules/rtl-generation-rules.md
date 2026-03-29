# RTL生成规则

## 1. 需求解析规则

### 1.1 自然语言解析
- 识别用户需求中的关键功能点
- 提取模块名称、接口信号、功能列表
- 识别RV32I相关术语（ALU、寄存器文件、控制单元等）

### 1.2 需求澄清
- 当需求不明确时，主动询问用户：
  - 数据位宽（默认32位）
  - 接口信号要求
  - 性能要求
  - 特殊功能需求

### 1.3 需求规范化
- 将自然语言需求转换为结构化规格文档
- 包含：模块名、接口定义、功能列表、测试要求

## 2. Chisel代码生成规则

### 2.1 代码结构规范
```
// 文件头注释
package riscv

import chisel3._
import chisel3.util._

// 模块定义
class ModuleName extends Module {
  val io = IO(new Bundle {
    // 接口定义
  })

  // 内部逻辑
}
```

### 2.2 接口规范
- 所有RV32I模块必须包含：
  - clock: Clock
  - reset: Reset
  - 数据输入输出（32位）
  - 控制信号
- 信号命名遵循蛇形命名法（snake_case）

### 2.3 功能实现规范
- 使用Chisel内置操作符实现逻辑
- 添加必要的寄存器用于时序逻辑
- 实现完整的组合逻辑和时序逻辑分离

### 2.4 代码质量要求
- 添加适当的注释
- 模块化设计，功能分离
- 错误处理机制
- 可测试性设计

## 3. 编译转换规则

### 3.1 编译流程
1. 执行 `sbt compile` 编译Chisel代码
2. 生成FIRRTL中间表示
3. 使用 `firtool` 转换为Verilog
4. 输出到 `src/verilog/` 目录

### 3.2 转换参数
```bash
firtool input.fir \
  --lowering-options=disallowLocalVariables \
  --emit-metadata \
  --output-annotation-file=metadata.json \
  -o output.v
```

## 4. 语法检查规则

### 4.1 Verilator检查
```bash
verilator --lint-only \
  --Wall \
  --Wno-style \
  --Wno-fatal \
  src/verilog/*.v
```

### 4.2 错误处理
- ERROR必须修复
- WARNING需要评估，在报告中说明
- 自动尝试修复常见语法错误

## 5. Testbench生成规则

### 5.1 测试平台结构
```systemverilog
module module_name_tb;
  // 时钟和复位生成
  // DUT实例化
  // 测试激励
  // 结果检查
  // 仿真控制
endmodule
```

### 5.2 测试用例生成
- 根据功能列表生成对应测试
- 包含边界测试
- 包含随机测试
- 包含错误情况测试

### 5.3 测试激励
- 系统时钟生成（默认100MHz）
- 复位序列
- 功能测试序列
- 结果验证

## 6. 仿真验证规则

### 6.1 仿真执行
```bash
verilator --cc --exe --build \
  -j 0 \
  --trace \
  --assert \
  -Wall \
  -Wno-style \
  src/verilog/module.v \
  src/testbench/module_tb.cpp \
  -o sim_build
```

### 6.2 结果收集
- 收集所有测试结果
- 统计通过/失败数量
- 记录仿真时间
- 生成详细日志

### 6.3 报告生成
- 格式符合要求
- 包含所有测试结果
- 包含语法检查结果
- 包含性能统计

## 7. 文件管理规则

### 7.1 命名规范
- Chisel文件: `模块名.scala`
- Verilog文件: `模块名.v`
- Testbench: `模块名_tb.sv`
- 仿真脚本: `sim_模块名.sh`
- 报告文件: `verification_模块名.md`

### 7.2 目录结构
- 严格遵循项目目录结构
- 生成文件放入对应目录
- 保持目录整洁

## 8. 错误处理规则

### 8.1 编译错误
- 分析错误信息
- 尝试自动修复
- 如无法修复，提示用户

### 8.2 仿真错误
- 分析失败原因
- 调整测试激励
- 重新仿真

### 8.3 流程错误
- 记录错误日志
- 提供恢复建议
- 继续执行或终止

## 9. 完整流程自动化规则

### 9.1 自动触发条件
当用户请求生成模块时（如"生成 ALU"、"生成寄存器文件"等），无论是否明确要求，都应自动执行完整 TDD 流程：
- testbench-generation-skill（先）
- rtl-generation-skill（中）
- rtl-verification-skill（后）

### 9.2 流程自动串联
- **无需用户干预**：每个步骤完成后自动进入下一步
- **自动进入 testbench**：RTL 编译成功后自动触发 testbench 生成
- **自动进入 verification**：RTL 生成后自动触发验证

### 9.3 错误处理与自动修复
| 阶段 | 失败时操作 |
|------|-----------|
| testbench 编译 | 修复 testbench 代码并重试 |
| RTL 编译 | 修复 Chisel 代码并重试 |
| RTL lint | 分析错误并自动修复 Verilog |
| 仿真失败 | 修复 RTL 代码直到通过 |

### 9.4 人工干预时机
仅在以下情况才向用户求助：
- 需求不明确且无法推断
- 多次重试后仍无法解决的问题
- 需要确认的特殊功能或接口