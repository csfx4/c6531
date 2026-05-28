import sys
import os

OPCODES = {
    'ADD': 0x01,
    'SETMODE': 0x02,
    'LOAD': 0x03,
    'STORE': 0x04,
    'FADD': 0x05,
    'JMP': 0x06,
    'JZ': 0x07
}

def parse_register(reg_str):
    reg_str = reg_str.strip().upper()
    if reg_str == 'PC':
        return 0
    if reg_str.startswith('R'):
        try:
            reg_num = int(reg_str[1:])
            if 0 <= reg_num <= 31:
                return reg_num
        except ValueError:
            pass
    raise ValueError()

def parse_immediate(imm_str):
    imm_str = imm_str.strip()
    try:
        if imm_str.lower().startswith('0x'):
            return int(imm_str, 16)
        return int(imm_str)
    except ValueError:
        raise ValueError()

def assemble_line(line, line_num):
    if ';' in line:
        line = line.split(';', 1)[0]
    
    line = line.strip()
    if not line:
        return None

    parts = line.replace(',', ' ').split()
    mnemonic = parts[0].upper()
    
    if mnemonic not in OPCODES:
        sys.exit(1)
        
    opcode = OPCODES[mnemonic]
    args = parts[1:]

    reg_dst = 0
    reg_src1 = 0
    reg_src2 = 0
    immediate = 0

    try:
        if mnemonic in ['ADD', 'FADD']:
            if len(args) != 3:
                raise ValueError()
            reg_dst = parse_register(args[0])
            reg_src1 = parse_register(args[1])
            reg_src2 = parse_register(args[2])
            
        elif mnemonic == 'SETMODE':
            if len(args) != 1:
                raise ValueError()
            immediate = parse_immediate(args[0])
            if immediate not in [31, 65]:
                raise ValueError()
                
        elif mnemonic in ['LOAD', 'STORE']:
            if len(args) != 2:
                raise ValueError()
            if mnemonic == 'LOAD':
                reg_dst = parse_register(args[0])
            else:
                reg_src1 = parse_register(args[0])
            immediate = parse_immediate(args[1])
            
        elif mnemonic == 'JMP':
            if len(args) != 1:
                raise ValueError()
            immediate = parse_immediate(args[0])
            
        elif mnemonic == 'JZ':
            if len(args) != 2:
                raise ValueError()
            reg_src1 = parse_register(args[0])
            immediate = parse_immediate(args[1])

    except ValueError:
        sys.exit(1)

    instruction = 0
    instruction |= (opcode & 0xFF) << 57
    instruction |= (reg_dst & 0x1F) << 52
    instruction |= (reg_src1 & 0x1F) << 47
    instruction |= (reg_src2 & 0x1F) << 42
    instruction |= (immediate & 0x3FFFFFFFFFF)

    return instruction

def main():
    if len(sys.argv) < 2:
        sys.exit(1)

    src_path = sys.argv[1]
    dst_path = sys.argv[2] if len(sys.argv) > 2 else "program.hex"

    if not os.path.exists(src_path):
        sys.exit(1)
    
    compiled_words = []
    with open(src_path, 'r', encoding='utf-8') as f:
        for idx, line in enumerate(f, 1):
            word = assemble_line(line, idx)
            if word is not None:
                compiled_words.append(word)

    with open(dst_path, 'w', encoding='utf-8') as f:
        for word in compiled_words:
            f.write(f"{word:017X}\n")

if __name__ == '__main__':
    main()
