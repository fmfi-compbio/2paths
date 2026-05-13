import sys

gfa = sys.argv[1]
pgf = sys.argv[2]

with open(pgf) as f:
    pgf_lines = f.readlines()
header_line = pgf_lines[0].strip()


with open(gfa) as gfa_f:
    for line in gfa_f:
        if line.startswith("P"):
            fields = line.strip().split("\t")
            break

vertices = fields[2].split(",")
first = vertices[0].replace("+", "")
last = vertices[-1].replace("+", "")

new_header = f"Header: {header_line} {first} {last}"

# Write the new file with the new header and the rest of the .pgf file
print(new_header)
for line in pgf_lines[1:]:
    print(line, end="")
