"""Generate src/sin.v: a 10-bit, 8192-sample, single-period sine lookup table.

Output is offset binary: 0x200 = zero, 0x3FF = +peak, 0x000 = -peak.
Run from the repo root:  python tools/gen_sine_rom.py
"""
import math

ADDR_BITS = 13             # 2^13 = 8192 samples per period
DATA_BITS = 10             # 10-bit output
N = 2 ** ADDR_BITS
MID = 2 ** (DATA_BITS - 1)  # 512
MAX = 2 ** DATA_BITS - 1    # 1023

values = [min(MAX, round(MID + MID * math.sin(2 * math.pi * i / N))) for i in range(N + 1)]

lines = [
    "module sin (",
    f"    input wire [{ADDR_BITS - 1}:0] INPUT,      ",
    f"    output reg [{DATA_BITS - 1}:0] OUTPUT",
    ");",
    "",
    f"initial OUTPUT = {DATA_BITS}'b{'0' * DATA_BITS};  ",
    f"reg [{DATA_BITS - 1}:0] memory [0:{N}];",
    "",
    "initial begin",
]
for start in range(0, N + 1, 10):
    row = " ".join(f"memory[{i}] = {DATA_BITS}'h{values[i]:03x};" for i in range(start, min(start + 10, N + 1)))
    lines.append("    " + row)
lines += ["end", "", "always @(*) begin  ", "    OUTPUT = memory[INPUT];", "end", "", "endmodule"]

with open("src/sin.v", "w") as f:
    f.write("\n".join(lines) + "\n")
print(f"wrote src/sin.v ({N} samples, {DATA_BITS}-bit)")
