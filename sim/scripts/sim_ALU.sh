#!/bin/bash
MODULE_NAME="ALU"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "========================================"
echo "ALU Simulation Script"
echo "========================================"

# Step 1: Check required files
echo "[1/5] Checking required files..."
if [ ! -f "$PROJECT_ROOT/src/verilog/${MODULE_NAME}.v" ]; then
  echo "ERROR: RTL not found: $PROJECT_ROOT/src/verilog/${MODULE_NAME}.v"
  echo "Please run rtl-generation-skill first."
  exit 1
fi
if [ ! -f "$PROJECT_ROOT/src/testbench/${MODULE_NAME}_tb.cpp" ]; then
  echo "ERROR: Testbench not found: $PROJECT_ROOT/src/testbench/${MODULE_NAME}_tb.cpp"
  exit 1
fi
echo "  All required files found."

# Step 2: Lint RTL
echo "[2/5] Running Verilator lint..."
mkdir -p "$PROJECT_ROOT/sim/logs"
verilator --lint-only \
  --Wall \
  --Wno-style \
  "$PROJECT_ROOT/src/verilog/${MODULE_NAME}.v" \
  2>&1 | tee "$PROJECT_ROOT/sim/logs/lint_${MODULE_NAME}.log"
if [ ${PIPESTATUS[0]} -ne 0 ]; then
  echo "WARNING: Lint found issues (see log)"
fi

# Step 3: Compile simulation model
echo "[3/5] Compiling simulation model..."
SIM_BUILD_DIR="$PROJECT_ROOT/sim_build/${MODULE_NAME}"
rm -rf "$SIM_BUILD_DIR"
mkdir -p "$SIM_BUILD_DIR"

verilator --cc --exe --build \
  --trace \
  --assert \
  -Wall \
  -Wno-style \
  "$PROJECT_ROOT/src/verilog/${MODULE_NAME}.v" \
  "$PROJECT_ROOT/src/testbench/${MODULE_NAME}_tb.cpp" \
  -o "$SIM_BUILD_DIR/obj_dir/${MODULE_NAME}_sim" \
  2>&1 | tee "$SIM_BUILD_DIR/compile.log"

if [ ! -f "$SIM_BUILD_DIR/obj_dir/${MODULE_NAME}_sim" ]; then
  echo "ERROR: Simulation build failed"
  exit 1
fi
echo "  Simulation model built successfully."

# Step 4: Run simulation
echo "[4/5] Running simulation..."
cd "$PROJECT_ROOT"
"$SIM_BUILD_DIR/obj_dir/${MODULE_NAME}_sim" 2>&1 | tee sim/logs/sim_${MODULE_NAME}.log

# Step 5: Show results
echo "[5/5] Simulation complete."
echo "========================================"
echo "Results saved to:"
echo "  - sim/logs/lint_${MODULE_NAME}.log"
echo "  - sim/logs/sim_${MODULE_NAME}.log"
echo "========================================"
