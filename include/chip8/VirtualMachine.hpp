#pragma once
#include "chip8/Types.hpp"
#include "chip8/Constants.hpp"
#include "chip8/Timers.hpp"

namespace chip8 {
  struct VirtualMachine {
    ByteArray<RAM_SIZE> memory;
    ByteArray<REGISTER_COUNT> registers;
    Address programCounter;
    Address I; // address register
    Timers timers;
    Stack stack;
    RandomNumberGenerator rng;
    GraphicsBuffer graphics;

    VirtualMachine()
      : memory{}
      , registers{}
      , programCounter{0}
      , I{0}
      , timers{}
      , stack{}
      , rng{[](Byte seed) { return 7; }}
      , graphics{}
    {
      memory.fill(0);
      registers.fill(0);
      graphics.fill(0);
    }
  };
}