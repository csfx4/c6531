#include "cpu.hpp"

int main(int argc, char* argv[]) {
    std::string firmware = "program.hex";
    if (argc > 1) firmware = argv[1];

    CPU core;
    if (!core.load_program(firmware)) {
        std::cout << "Error: Could not load " << firmware << std::endl;
        std::cout << "\nPress Enter to exit...";
        std::cin.get();
        return 1;
    }

    while (core.get_pc() < core.ram_size()) {
        uint65_t instr = core.read_ram(core.get_pc());
        if (instr.low == 0 && instr.high == 0) break;
        DecodedInstruction d = decode(instr);
        execute(core, d);
    }

    core.dump_state();

    std::cout << "\nExecution finished. Press Enter to close this window...";
    std::cin.get();

    return 0;
}
