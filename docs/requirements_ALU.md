# ALU Requirements

## Module Name
ALU (Arithmetic Logic Unit)

## Interface Signals
| Signal | Width | Direction | Description |
|--------|-------|-----------|-------------|
| io_a | 32bit | Input | First operand |
| io_b | 32bit | Input | Second operand |
| io_opcode | 4bit | Input | Operation select (0=ADD, 1=SUB, 2=AND, 3=OR, 4=XOR, 5=SLT) |
| io_result | 32bit | Output | Operation result |
| io_zero | 1bit | Output | Zero flag (1 when result == 0) |

## Function List
| Opcode | Operation | Description |
|--------|-----------|-------------|
| 0 | ADD | io_result = io_a + io_b |
| 1 | SUB | io_result = io_a - io_b |
| 2 | AND | io_result = io_a & io_b |
| 3 | OR | io_result = io_a \| io_b |
| 4 | XOR | io_result = io_a ^ io_b |
| 5 | SLT | io_result = (io_a < io_b) ? 1 : 0 (signed comparison) |

## RV32I Compliance
- 32-bit data path
- Two's complement arithmetic for ADD/SUB/SLT
- Zero flag: io_zero = (io_result == 0)

## Verification Plan
- Test all 6 operations with representative values
- Test zero flag with result = 0 and result != 0 cases
- Test boundary conditions (min/max signed values)
