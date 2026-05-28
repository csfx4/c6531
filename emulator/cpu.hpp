#ifndef CPU_HPP
#define CPU_HPP

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>

struct uint65_t {
    uint64_t low;
    uint64_t high; 

    uint65_t() : low(0), high(0) {}
    uint65_t(uint64_t l) : low(l), high(0) {}
    uint65_t(uint64_t l, uint64_t h) : low(l), high(h & 1) {}

    bool operator==(uint64_t val) const {
        return low == val && high == 0;
    }

    uint65_t operator+(const uint65_t& other) const {
        uint65_t res;
        res.low = low + other.low;
        res.high = (high + other.high + (res.low < low)) & 1;
        return res;
    }
};

enum class CpuMode {
    MODE_31 = 31,
    MODE_65 = 65
};

struct DecodedInstruction {
    uint8_t opcode;
    uint8_t r_dst;
    uint8_t r_src1;
    uint8_t r_src2;
    uint64_t imm;
};

class CPU {
private:
    uint65_t registers[32];
    uint64_t pc;
    CpuMode current_mode;
    std::vector<uint65_t>* ram;

    uint65_t apply_mode_mask(uint65_t val);

public:
    CPU(size_t mem_size = 65536);
    ~CPU();

    bool load_program(const std::string& hex_filename);
    void set_reg(uint8_t idx, uint65_t val);
    uint65_t get_reg(uint8_t idx) const;
    
    uint64_t get_pc() const { return pc; }
    void set_pc(uint64_t val) { pc = val; }
    CpuMode get_mode() const { return current_mode; }
    void set_mode(CpuMode mode) { current_mode = mode; }
    size_t ram_size() const { return ram->size(); }

    void write_ram(size_t addr, uint65_t val);
    uint65_t read_ram(size_t addr);
    void dump_state();
};

DecodedInstruction decode(uint65_t instr);
void execute(CPU& cpu, const DecodedInstruction& instr);

#endif
