import json

opcode_json_file = 'tmp_python\\Opcodes.json'
with open(opcode_json_file, 'r') as f:
    opcode_data = json.load(f)
opcodes_by_group_unprefixed={}
for opcode, data in opcode_data["unprefixed"].items():
    print(f"Opcode: {opcode}, Data: {data}")
    if "group" not in data:
        print(f"Opcode {opcode} does not have a group.")
        continue

