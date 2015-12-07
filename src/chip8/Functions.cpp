#include "chip8/Functions.hpp"
#include "chip8/VirtualMachine.hpp"
#include "chip8/Opcodes.hpp"
#include <array>
#include <functional>

namespace chip8 {

  std::array<std::function<void(VirtualMachine &, Instruction)>, 16> FUNCTION_TABLE { {
    ops::disambiguate0x0,
    ops::jump,
    ops::callSubroutine,
    ops::skipIfEquals,
    ops::skipIfNotEquals,
    ops::skipIfVxEqualsVy,
    ops::setVx,
    ops::addToVx
  } };

  Instruction fetch(VirtualMachine & vm) {
    auto instruction = vm.memory[vm.programCounter++];
    return instruction;
  }

  void execute(VirtualMachine & vm, Instruction instruction) {
    // no-op
    const Instruction OP_MASK = 0xF000;

    // FUNCTION_TABLE[instruction & OP_MASK](vm, instruction);

    switch(instruction & OP_MASK) {
      case 0x0:
        if(0x00E0 == instruction) {
          ops::clearScreen(vm, instruction);
          break;
        } else if(0x00EE == instruction) {
          ops::returnFromSubroutine(vm, instruction);
          break;
        } else {
          ops::callProgramAtAddress(vm, instruction);
          break;
        }
        break;

      case 0x1000:
        ops::jump(vm, instruction);
        break;

      case 0x2000:
        ops::callSubroutine(vm, instruction);
        break;

      case 0x3000:
        ops::skipIfEquals(vm, instruction);
        break;

      case 0x4000:
        ops::skipIfNotEquals(vm, instruction);
        break;

      case 0x5000:
        ops::skipIfVxEqualsVy(vm, instruction);
        break;

      case 0x6000:
        ops::setVx(vm, instruction);
        break;

      case 0x7000:
        ops::addToVx(vm, instruction);
        break;

      default:
      break;
    }
  }
}