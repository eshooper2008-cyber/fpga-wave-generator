import math

num_samples = 2 ** 13

with open('sine_init.txt', 'w') as f:
    for i in range(num_samples):
        angle = 2 * math.pi * i / num_samples
        sine_val = math.sin(angle)

        fixed_point = int(sine_val * (2 ** 27 - 1))
        if fixed_point < 0:
            fixed_point = fixed_point + 2 ** 28

        f.write(f"    rom_memory[{i}] = 28'h{fixed_point:07X};\n")
