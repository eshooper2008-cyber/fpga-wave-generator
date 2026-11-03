import math

num_samples = 2 ** 13

with open('sine_init.txt', 'w') as f:
    for i in range(num_samples):
        angle = 2 * math.pi * i / num_samples
        sine_val = math.sin(angle)

        fixed_point = round(512 + 512 * sine_val)
        if fixed_point > 1023:
            fixed_point = 1023

        f.write(f"    memory[{i}] = 10'h{fixed_point:03x};\n")
