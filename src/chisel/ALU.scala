// ALU - Arithmetic Logic Unit for RISC-V RV32I
// Supports: ADD, SUB, AND, OR, XOR, SLT with zero flag

package riscv

import chisel3._
import chisel3.util._

class ALU extends Module {
  val io = IO(new Bundle {
    val io_a      = Input(UInt(32.W))    // First operand
    val io_b      = Input(UInt(32.W))    // Second operand
    val io_opcode = Input(UInt(4.W))     // Operation select (0=ADD, 1=SUB, 2=AND, 3=OR, 4=XOR, 5=SLT)
    val io_result = Output(UInt(32.W))   // Operation result
    val io_zero   = Output(Bool())        // Zero flag
  })

  // Default values
  io.io_result := 0.U(32.W)
  io.io_zero := false.B

  // ALU operations using switch statement
  switch(io.io_opcode) {
    is(0.U) {  // ADD
      io.io_result := io.io_a + io.io_b
    }
    is(1.U) {  // SUB
      io.io_result := io.io_a - io.io_b
    }
    is(2.U) {  // AND
      io.io_result := io.io_a & io.io_b
    }
    is(3.U) {  // OR
      io.io_result := io.io_a | io.io_b
    }
    is(4.U) {  // XOR
      io.io_result := io.io_a ^ io.io_b
    }
    is(5.U) {  // SLT
      // Signed comparison: result = (a < b) ? 1 : 0
      io.io_result := Mux(io.io_a(31) === io.io_b(31),
                          Mux(io.io_a < io.io_b, 1.U, 0.U),
                          Mux(io.io_a(31) === 1.U, 1.U, 0.U))
    }
  }

  // Zero flag: set when result equals zero
  io.io_zero := io.io_result === 0.U(32.W)
}
