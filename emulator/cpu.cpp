#include "cpu.hpp"
#include <fstream>
#include <iomanip>
#include <cmath>

CPU::CPU(size_t mem_size) {
    pc = 0;
    current_mode = CpuMode::MODE_65;
    ram = new std::vector<uint65_t>(mem_size, uint65_t(0, 0));
    for (int i = 0; i < 32; ++i) registers[i] = uint65_t(0, 0);
}

CPU::~CPU() {
    delete ram;
}

uint65_t CPU::apply_mode_mask(uint65_t val) {
    if (current_mode == CpuMode::MODE_31) {
        val.low &= 0x7FFFFFFF;
        val.high = 0;
    } else {
        val.high &= 1;
    }
    return val;
}

bool CPU::load_program(const std::string& hex_filename) {
    std::ifstream file(hex_filename);
    if (!file.is_open()) return false;

    std::string line;
    size_t addr = 0;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        uint64_t high_val = 0;
        uint64_t low_val = 0;

        if (line.length() >= 17) {
            std::string high_str = line.substr(0, line.length() - 16);
            std::string low_str = line.substr(line.length() - 16);
            
            high_val = std::stoull(high_str, nullptr, 16);
            low_val = std::stoull(low_str, nullptr, 16);
        } else {
            low_val = std::stoull(line, nullptr, 16);
        }

        uint65_t word;
        word.low = low_val;
        word.high = (high_val & 1);

        if (addr < ram->size()) {
            (*ram)[addr++] = word;
        }
    }
    return true;
}

void CPU::set_reg(uint8_t idx, uint65_t val) {
    if (idx < 32) registers[idx] = apply_mode_mask(val);
}

uint65_t CPU::get_reg(uint8_t idx) const {
    return idx < 32 ? registers[idx] : uint65_t(0, 0);
}

void CPU::write_ram(size_t addr, uint65_t val) {
    if (addr < ram->size()) (*ram)[addr] = apply_mode_mask(val);
}

uint65_t CPU::read_ram(size_t addr) {
    return addr < ram->size() ? (*ram)[addr] : uint65_t(0, 0);
}

void CPU::dump_state() {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 4; ++j) {
            int idx = i * 4 + j;
            std::cout << "R" << idx << ":" << std::hex << registers[idx].high << "_" 
                      << std::setw(16) << std::setfill('0') << registers[idx].low << " ";
        }
        std::cout << std::endl;
    }
}

DecodedInstruction decode(uint65_t instr) {
    DecodedInstruction d;
    uint64_t parts_low = (instr.low >> 57) & 0x7F;
    uint64_t parts_high = (instr.high & 1) << 7;
    d.opcode = (uint8_t)(parts_high | parts_low);

    d.r_dst = (uint8_t)((instr.low >> 52) & 0x1F);
    d.r_src1 = (uint8_t)((instr.low >> 47) & 0x1F);
    d.r_src2 = (uint8_t)((instr.low >> 42) & 0x1F);
    d.imm = instr.low & 0x3FFFFFFFFFF;
    return d;
}

double c65_to_double(uint65_t val) {
    uint8_t sign = (uint8_t)(val.high & 1);
    uint16_t exponent = (uint16_t)((val.low >> 52) & 0xFFF);
    uint64_t mantissa = val.low & 0xFFFFFFFFFFFFF;
    double res = ldexp((double)mantissa, exponent - 2047);
    return sign ? -res : res;
}

uint65_t double_to_c65(double val) {
    if (val == 0.0) return uint65_t(0, 0);
    uint8_t sign = (val < 0) ? 1 : 0;
    int exp_raw;
    double mant_raw = frexp(std::fabs(val), &exp_raw);
    uint16_t exponent = (uint16_t)(exp_raw + 2047) & 0xFFF;
    uint64_t mantissa = (uint64_t)(mant_raw * ((uint64_t)1 << 52)) & 0xFFFFFFFFFFFFF;
    
    uint65_t res;
    res.low = mantissa;
    res.low |= ((uint64_t)(exponent & 0xFFF)) << 52;
    res.high = sign & 1;
    return res;
}

void execute(CPU& cpu, const DecodedInstruction& instr) {
    uint64_t current_pc = cpu.get_pc();
    bool pc_changed = false;

    switch (instr.opcode) {
        case 0x01:
            cpu.set_reg(instr.r_dst, cpu.get_reg(instr.r_src1) + cpu.get_reg(instr.r_src2));
            break;
        case 0x02:
            if (instr.imm == 31) cpu.set_mode(CpuMode::MODE_31);
            if (instr.imm == 65) cpu.set_mode(CpuMode::MODE_65);
            break;
        case 0x03:
            cpu.set_reg(instr.r_dst, uint65_t(instr.imm));
            break;
        case 0x04:
            if (instr.imm < cpu.ram_size()) {
                cpu.write_ram(instr.imm, cpu.get_reg(instr.r_src1));
            }
            break;
        case 0x05: 
            {
                double v1 = c65_to_double(cpu.get_reg(instr.r_src1));
                double v2 = c65_to_double(cpu.get_reg(instr.r_src2));
                cpu.set_reg(instr.r_dst, double_to_c65(v1 + v2));
            }
            break;
        case 0x06:
            cpu.set_pc(instr.imm);
            pc_changed = true;
            break;
        case 0x07:
            if (cpu.get_reg(instr.r_src1) == uint64_t(0)) {
                cpu.set_pc(instr.imm);
                pc_changed = true;
            }
            break;
        default:
            break;
    }

    if (!pc_changed) {
        cpu.set_pc(current_pc + 1);
    }
}
