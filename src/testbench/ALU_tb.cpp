#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "verilated.h"
#include "VALU.h"

// Operation codes (must match Chisel module)
#define OP_ADD 0
#define OP_SUB 1
#define OP_AND 2
#define OP_OR  3
#define OP_XOR 4
#define OP_SLT 5

// Operation names
const char* op_name[] = {"ADD", "SUB", "AND", "OR", "XOR", "SLT"};

int test_count = 0;
int pass_count = 0;

void test_op(VALU* dut, int op, uint32_t a, uint32_t b, uint32_t expect) {
  // Apply stimulus
  dut->io_io_a = a;
  dut->io_io_b = b;
  dut->io_io_opcode = op;

  // Toggle clock to process
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

  VALU* dut = new VALU;

  // Initialize
  dut->clock = 0;
  dut->reset = 1;
  dut->io_io_a = 0;
  dut->io_io_b = 0;
  dut->io_io_opcode = 0;

  // Reset cycle
  for (int i = 0; i < 5; i++) {
    dut->clock = !dut->clock;
    dut->eval();
  }
  dut->reset = 0;
  dut->clock = 0;
  dut->eval();

  printf("========================================\n");
  printf("ALU Testbench Simulation Start\n");
  printf("========================================\n");

  // ===== ADD Tests =====
  test_op(dut, OP_ADD, 0, 0, 0);         // 0+0=0
  test_op(dut, OP_ADD, 5, 3, 8);          // 5+3=8
  test_op(dut, OP_ADD, 100, 200, 300);     // 100+200=300

  // ===== SUB Tests =====
  test_op(dut, OP_SUB, 5, 3, 2);          // 5-3=2
  test_op(dut, OP_SUB, 3, 5, 0xFFFFFFFE); // 3-5=-2
  test_op(dut, OP_SUB, 0, 0, 0);           // 0-0=0
  test_op(dut, OP_SUB, 10, 10, 0);         // 10-10=0

  // ===== AND Tests =====
  test_op(dut, OP_AND, 0xFF, 0x0F, 0x0F);                    // 0xFF&0x0F=0x0F
  test_op(dut, OP_AND, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);   // all ones
  test_op(dut, OP_AND, 0xAAAAAAAA, 0x55555555, 0);           // result=0

  // ===== OR Tests =====
  test_op(dut, OP_OR, 0xF0, 0x0F, 0xFF);                     // 0xF0|0x0F=0xFF
  test_op(dut, OP_OR, 0, 0, 0);                              // 0|0=0
  test_op(dut, OP_OR, 0xAAAAAAAA, 0x55555555, 0xFFFFFFFF);    // result=0xFFFFFFFF

  // ===== XOR Tests =====
  test_op(dut, OP_XOR, 0xFF, 0x0F, 0xF0);                    // 0xFF^0x0F=0xF0
  test_op(dut, OP_XOR, 0xAAAAAAAA, 0xAAAAAAAA, 0);            // same^same=0
  test_op(dut, OP_XOR, 0xFF, 0xFF, 0);                       // same^same=0

  // ===== SLT Tests (signed comparison) =====
  test_op(dut, OP_SLT, 5, 3, 0);         // 5<3? No -> 0
  test_op(dut, OP_SLT, 3, 5, 1);         // 3<5? Yes -> 1
  test_op(dut, OP_SLT, 0xFFFFFFFF, 0, 1); // -1<0? Yes -> 1
  test_op(dut, OP_SLT, 0, 0, 0);         // 0<0? No -> 0

  printf("========================================\n");
  printf("RESULT: %s %d/%d tests\n",
         pass_count == test_count ? "PASSED" : "FAILED",
         pass_count, test_count);
  printf("========================================\n");

  dut->final();
  delete dut;

  return 0;
}
