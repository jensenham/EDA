# 基于Claude Code的RISC-V RTL自动生成系统

## 项目概述

本项目是基于Claude Code的RISC-V模块RTL自动生成Agent，实现了从自然语言到可仿真RTL的全自动流水线。系统通过配置Rules、Skills和Hooks，能够自动完成需求解析、Chisel代码生成、编译转换、语法检查、测试平台生成和仿真验证。

## 项目结构

```
作业1/
├── .claude/                    # Claude Code配置文件
│   ├── rules/                 # 规则定义
│   │   └── rtl-generation-rules.md
│   ├── skills/               # 技能模板
│   │   └── rtl-generation-skill.md
│   └── hooks/                # 钩子脚本
│       ├── pre-commit-hook.sh
│       └── post-generation-hook.sh
├── src/                      # 源代码
│   ├── chisel/              # Chisel源码
│   │   └── ALU.scala        # 示例ALU模块
│   ├── verilog/             # 生成的Verilog
│   └── testbench/           # 测试平台
│       ├── ALU_tb.sv        # ALU测试平台
│       └── ALU_tb.cpp       # C++测试包装器
├── sim/                     # 仿真相关
│   ├── logs/               # 仿真日志
│   └── scripts/            # 仿真脚本
│       └── sim_ALU.sh      # ALU仿真脚本
├── docs/                    # 文档
├── build.sbt               # Scala构建配置
├── CLAUDE.md              # 项目指导文件
├── auto_generate_rtl.sh   # 自动生成脚本
└── README.md              # 本文件
```

## 功能特性

### 全自动流水线
1. **需求解析**：解析自然语言需求，提取模块功能和接口要求
2. **代码生成**：自动生成符合RV32I标准的Chisel代码
3. **编译转换**：使用sbt和firtool将Chisel编译为Verilog
4. **语法检查**：使用Verilator进行语法检查
5. **测试生成**：自动生成测试平台和测试用例
6. **仿真验证**：执行仿真并生成验证报告

### 支持的模块类型
- ALU（算术逻辑单元）
- 寄存器文件
- 控制单元
- 其他RISC-V子模块

### 支持的功能
- ADD/SUB/AND/OR/XOR/SLT等算术逻辑操作
- zero标志位
- 32位数据宽度
- RV32I标准接口

## 快速开始

### 1. 环境准备
```bash
# 安装必要工具
# Node.js 18+ 和 Claude Code CLI
# Java 11+ 和 sbt
# firtool（FIRRTL编译器）
# Verilator >= 4.200
```

### 2. 运行自动生成
```bash
cd "$(dirname "$0")"
./auto_generate_rtl.sh
```

### 3. 输入需求
当脚本提示时，输入自然语言需求，例如：
```
请你帮我生成一个ALU，需要支持ADD/SUB/AND/OR/XOR/SLT，含 zero 标志位
```

### 4. 查看结果
生成的文件将保存在以下目录：
- `src/chisel/` - Chisel源代码
- `src/verilog/` - 生成的Verilog
- `src/testbench/` - 测试平台
- `docs/` - 需求文档和验证报告
- `sim/logs/` - 仿真日志

## 详细使用

### 使用Claude Code集成
系统配置了完整的Claude Code规则、技能和钩子：

1. **Rules (CLAUDE.md)**：定义项目结构、技术栈和全自动流水线
2. **Skills**：提供RTL生成技能模板，指导AI完成各阶段任务
3. **Hooks**：
   - `pre-commit-hook.sh`：提交前自动检查
   - `post-generation-hook.sh`：生成后自动验证

### 手动执行验证
```bash
# 执行ALU仿真
cd "$(dirname "$0")"
bash sim/scripts/sim_ALU.sh

# 查看验证报告
cat docs/verification_ALU.md
```

### 自定义模块生成
1. 修改`auto_generate_rtl.sh`脚本中的需求解析逻辑
2. 在`src/chisel/`中添加新的Chisel模板
3. 在`src/testbench/`中添加对应的测试平台
4. 更新`.claude/rules/`和`.claude/skills/`中的配置

## 验证标准

### 语法检查
```bash
verilator --lint-only src/verilog/*.v
```
- 无ERROR输出
- WARNING需在报告中说明

### 仿真结果格式
```
[TEST 1] ADD: a=5 b=3 | expect=8 got=8 | PASS
[TEST 2] SUB: a=10 b=7 | expect=3 got=3 | PASS
...
RESULT: PASSED X/X tests
```

## 配置说明

### Claude Code配置
系统已预配置：
- 项目结构规范
- 文件路径规范
- 代码生成规则
- 验证流程

### 工具链配置
确保以下工具可用：
```bash
# 检查工具版本
node --version      # >= 18
java -version       # >= 11
sbt --version       # 最新版
firtool --version   # 最新版
verilator --version # >= 4.200
```

### API配置（如使用Claude API）
```bash
export ANTHROPIC_BASE_URL=<API地址>
export ANTHROPIC_API_KEY=<API密钥>
```

## 示例输出

### 生成的Chisel代码
```scala
class ALU extends Module {
  val io = IO(new Bundle {
    val a = Input(UInt(32.W))
    val b = Input(UInt(32.W))
    val alu_op = Input(UInt(4.W))
    val result = Output(UInt(32.W))
    val zero = Output(Bool())
  })
  // ... 实现逻辑
}
```

### 验证报告
```markdown
# ALU验证报告
## 1. 模块信息
- 模块名称: ALU
- 生成时间: 2026-03-19

## 2. 验证结果
- 语法检查: 通过
- 仿真测试: PASSED 10/10 tests
- 总体状态: ✅ 通过
```

## 扩展开发

### 添加新模块类型
1. 创建Chisel模板：`src/chisel/NewModule.scala`
2. 创建测试平台：`src/testbench/NewModule_tb.sv`
3. 更新需求解析逻辑
4. 更新规则和技能配置

### 集成AI自然语言解析
当前版本使用简单关键词匹配，可扩展为：
1. 集成Claude API进行深度需求解析
2. 使用LLM生成更复杂的代码逻辑
3. 实现多轮需求澄清对话

### 增强验证流程
1. 添加形式验证
2. 添加性能分析
3. 添加覆盖率分析
4. 添加功耗分析

## 注意事项

1. **API安全**：不要提交API密钥到代码仓库
2. **学术诚信**：报告需自主撰写，代码参考需注明来源
3. **工具版本**：确保使用正确版本的工具链
4. **路径配置**：根据实际环境调整文件路径

## 故障排除

### 常见问题
1. **sbt编译失败**：检查Java版本和sbt配置
2. **verilator语法错误**：检查生成的Verilog语法
3. **仿真失败**：检查测试激励和接口连接

### 日志文件
- 编译日志：`sim/logs/compile_*.log`
- 语法检查：`sim/logs/lint_*.log`
- 仿真日志：`sim/logs/sim_*.log`
- 验证报告：`docs/verification_*.md`

## 许可证

本项目用于复旦大学EDA课程作业，遵循课程要求。

## 作者

- 项目框架：Claude Code自动生成
- 示例代码：基于Chisel官方示例
- 验证脚本：自定义开发