import os
import json
output_file = "first_lines.txt"

freqs = {}

with open(output_file, "w") as output:
    for filename in os.listdir():
        if os.path.isfile(filename) \
                and 'sqlite3' not in filename \
                and output_file not in filename:
            with open(filename, "r") as file:
                first_line = str(file.readline())

                if not freqs.get(first_line):
                    freqs[first_line] = 1
                else:
                    freqs[first_line] += 1

                output.write(f"{filename}:\n")
                output.write(f"   {first_line}\n")

print(f"First lines are saved in {output_file}")
print(json.dumps(freqs, indent=4))
