#include "catch.hpp"
#include "chip8/VirtualMachine.hpp"
#include "chip8/Opcodes.hpp"
#include <utility>

TEST_CASE( "VM opcode functions", "execution of opcodes" ) {
  chip8::VirtualMachine vm;

  REQUIRE( vm.memory.size() == 4096 );
  REQUIRE( vm.registers.size() == 16 );
  REQUIRE( vm.programCounter == 0 );
  REQUIRE( vm.I == 0 );
  REQUIRE( vm.timers.sound == 0 );
  REQUIRE( vm.timers.delay == 0 );
  REQUIRE( vm.stack.size() == 0 );
  REQUIRE( vm.awaitingKeypress == false );

  SECTION( "ops::jump changes the program counter" ) {
    chip8::ops::jump(vm, 0x1234);

    REQUIRE( vm.programCounter == 0x234 );
  }

  SECTION( "ops::clearScreen clears the graphics buffer" ) {
    // TODO: Use randomized positions and values.
    vm.graphics[0] = 0xFF;
    vm.graphics[15] = 0x4;

    REQUIRE( vm.graphics[0] == 0xFF );
    REQUIRE( vm.graphics[15] == 0x4 );

    chip8::ops::clearScreen(vm, 0x00E0);

    bool isAllZeroes = true;

    for(std::size_t i = 0, size = vm.graphics.size(); i < size; i++) {
      if(vm.graphics[i] != 0) {
        isAllZeroes = false;
      }
    }

    REQUIRE( isAllZeroes == true );
  }

  SECTION( "ops::callSubroutine changes the program counter, pushes old program counter to stack" ) {
    vm.programCounter = 0x42;

    chip8::ops::callSubroutine(vm, 0x2123);

    REQUIRE( vm.programCounter == 0x123 );
    REQUIRE( vm.stack.size() == 1 );
    REQUIRE( vm.stack.top() == 0x42 );
  }

  SECTION( "ops::returnFromSubroutine changes the program counter, pops stack" ) {
    vm.stack.push(0x0867);

    chip8::ops::returnFromSubroutine(vm, 0x00EE);

    REQUIRE( vm.programCounter == 0x0867 );
    REQUIRE( vm.stack.size() == 0 );
  }

  SECTION( "ops::skipIfEquals increments the program counter by 2 if VX == NN" ) {
    vm.registers[0xD] = 0x42;
    auto pc = vm.programCounter;

    chip8::ops::skipIfEquals(vm, 0x3D42);

    REQUIRE( vm.programCounter == (pc + 2) );
  }

  SECTION( "ops::skipIfEquals does not increment the program counter by 2 if VX != NN" ) {
    vm.registers[0xD] = 0x99;
    auto pc = vm.programCounter;

    chip8::ops::skipIfEquals(vm, 0x3D42);

    REQUIRE( vm.programCounter == pc );
  }

  SECTION( "ops::skipIfNotEquals increments the program counter by 2 if VX != NN" ) {
    vm.registers[0xD] = 0x42;
    auto pc = vm.programCounter;

    chip8::ops::skipIfNotEquals(vm, 0x4D99);

    REQUIRE( vm.programCounter == (pc + 2) );
  }

  SECTION( "ops::skipIfNotEquals does not increment the program counter by 2 if VX == NN" ) {
    vm.registers[0xD] = 0x42;
    auto pc = vm.programCounter;

    chip8::ops::skipIfNotEquals(vm, 0x4D42);

    REQUIRE( vm.programCounter == pc );
  }

  SECTION( "ops::skipIfVxEqualsVy increments the program counter by 2 if VX == VY" ) {
    vm.registers[0x3] = 0x42;
    vm.registers[0xD] = 0x42;
    auto pc = vm.programCounter;

    chip8::ops::skipIfVxEqualsVy(vm, 0x53D0);

    REQUIRE( vm.programCounter == (pc + 2) );
  }

  SECTION( "ops::skipIfVxEqualsVy does not increment the program counter by 2 if VX != VY" ) {
    vm.registers[0x3] = 0x42;
    vm.registers[0xD] = 0x31;
    auto pc = vm.programCounter;

    chip8::ops::skipIfVxEqualsVy(vm, 0x53D0);

    REQUIRE( vm.programCounter == pc );
  }

  SECTION( "ops::setVx sets the register at VX to NN" ) {
    chip8::ops::setVx(vm, 0x63D0);

    REQUIRE( vm.registers[0x3] == 0xD0 );
  }

  SECTION( "ops::addToVx adds NN to the register at VX" ) {
    vm.registers[0xA] = 0x11;

    chip8::ops::addToVx(vm, 0x7A22);

    REQUIRE( vm.registers[0xA] == (0x11 + 0x22) );
  }

  SECTION( "ops::setVxToVy sets VX to the value of register VY" ) {
    vm.registers[0x0] = 0x11;
    vm.registers[0x1] = 0x22;

    chip8::ops::setVxToVy(vm, 0x8010);

    REQUIRE( vm.registers[0x0] == 0x22 );
  }

  SECTION( "ops::disambiguate0x8 calls ops::setVxToVy when given 0x8000" ) {
    vm.registers[0x0] = 0x11;
    vm.registers[0x1] = 0x22;

    chip8::ops::disambiguate0x8(vm, 0x8010);

    REQUIRE( vm.registers[0x0] == 0x22 );
  }

  SECTION( "ops::orVxVy sets VX to the value VX | VY" ) {
    vm.registers[0x0] = 0x10;
    vm.registers[0x1] = 0x02;

    chip8::ops::orVxVy(vm, 0x8011);

    REQUIRE( vm.registers[0x0] == (0x10 | 0x02) );
  }

  SECTION( "ops::disambiguate0x8 calls ops::orVxVy when given 0x8001" ) {
    vm.registers[0x0] = 0x10;
    vm.registers[0x1] = 0x02;

    chip8::ops::disambiguate0x8(vm, 0x8011);

    REQUIRE( vm.registers[0x0] == (0x10 | 0x02) );
  }

  SECTION( "ops::andVxVy sets VX to the value VX & VY" ) {
    vm.registers[0x0] = 0x11;
    vm.registers[0x1] = 0x02;

    chip8::ops::andVxVy(vm, 0x8012);

    REQUIRE( vm.registers[0x0] == (0x11 & 0x02) );
  }

  SECTION( "ops::disambiguate0x8 calls ops::andVxVy when given 0x8002" ) {
    vm.registers[0x0] = 0x11;
    vm.registers[0x1] = 0x02;

    chip8::ops::disambiguate0x8(vm, 0x8012);

    REQUIRE( vm.registers[0x0] == (0x11 & 0x02) );
  }

  SECTION( "ops::xorVxVy sets VX to the value VX ^ VY" ) {
    vm.registers[0x0] = 0x11;
    vm.registers[0x1] = 0x02;

    chip8::ops::xorVxVy(vm, 0x8012);

    REQUIRE( vm.registers[0x0] == (0x11 ^ 0x02) );
  }

  SECTION( "ops::disambiguate0x8 calls ops::xorVxVy when given 0x8003" ) {
    vm.registers[0x0] = 0x11;
    vm.registers[0x1] = 0x02;

    chip8::ops::disambiguate0x8(vm, 0x8013);

    REQUIRE( vm.registers[0x0] == (0x11 ^ 0x02) );
  }

  SECTION( "ops::addVxVyUpdateCarry adds VY to VX, VF = 1 if there's a carry" ) {
    vm.registers[0x0] = 0xFF;
    vm.registers[0x1] = 0x01;

    chip8::ops::addVxVyUpdateCarry(vm, 0x8014);

    REQUIRE( vm.registers[0x0] == 0x00 );
    REQUIRE( vm.registers[0xF] == 0x01 );
  }

  SECTION( "ops::addVxVyUpdateCarry adds VY to VX, VF = 0 if there's no carry" ) {
    vm.registers[0x0] = 0xFA;
    vm.registers[0x1] = 0x01;

    chip8::ops::addVxVyUpdateCarry(vm, 0x8014);

    REQUIRE( vm.registers[0x0] == 0xFB );
    REQUIRE( vm.registers[0xF] == 0x00 );
  }

    SECTION( "ops::disambiguate0x8 calls ops::addVxVyUpdateCarry, adds VY to VX, VF = 1 if there's a carry" ) {
    vm.registers[0x0] = 0xFF;
    vm.registers[0x1] = 0x01;

    chip8::ops::disambiguate0x8(vm, 0x8014);

    REQUIRE( vm.registers[0x0] == 0x00 );
    REQUIRE( vm.registers[0xF] == 0x01 );
  }

  SECTION( "ops::disambiguate0x8 calls ops::addVxVyUpdateCarry, adds VY to VX, VF = 0 if there's no carry" ) {
    vm.registers[0x0] = 0xFA;
    vm.registers[0x1] = 0x01;

    chip8::ops::disambiguate0x8(vm, 0x8014);

    REQUIRE( vm.registers[0x0] == 0xFB );
    REQUIRE( vm.registers[0xF] == 0x00 );
  }

  SECTION( "ops::subtractVxVyUpdateCarry subtracts VY from VX, VF = 1 if there's no borrow" ) {
    vm.registers[0x0] = 0xFF;
    vm.registers[0x1] = 0x01;

    chip8::ops::subtractVxVyUpdateCarry(vm, 0x8015);

    REQUIRE( vm.registers[0x0] == 0xFE );
    REQUIRE( vm.registers[0xF] == 0x01 );
  }

  SECTION( "ops::subtractVxVyUpdateCarry subtracts VY from VX, VF = 0 if there is a borrow" ) {
    vm.registers[0x0] = 0x05;
    vm.registers[0x1] = 0x07;

    chip8::ops::subtractVxVyUpdateCarry(vm, 0x8015);

    chip8::Byte expected { static_cast<chip8::Byte>(5 - 7) };

    REQUIRE( vm.registers[0x0] == expected );
    REQUIRE( vm.registers[0xF] == 0x00 );
  }

  SECTION( "ops::subtractVxVyUpdateCarry subtracts VY from VX, VF = 0 if there is a borrow" ) {
    vm.registers[0x0] = 0x00;
    vm.registers[0x1] = 0x01;

    chip8::ops::subtractVxVyUpdateCarry(vm, 0x8015);

    REQUIRE( vm.registers[0x0] == 0xFF );
    REQUIRE( vm.registers[0xF] == 0x00 );
  }

  SECTION( "ops::disambiguate0x8 calls ops::subtractVxVyUpdateCarry, subtracts VY from VX, VF = 1 if there's no borrow" ) {
    vm.registers[0x0] = 0xFF;
    vm.registers[0x1] = 0x01;

    chip8::ops::disambiguate0x8(vm, 0x8015);

    REQUIRE( vm.registers[0x0] == 0xFE );
    REQUIRE( vm.registers[0xF] == 0x01 );
  }

  SECTION( "ops::disambiguate0x8 calls ops::subtractVxVyUpdateCarry, subtracts VY from VX, VF = 0 if there is a borrow" ) {
    vm.registers[0x0] = 0x05;
    vm.registers[0x1] = 0x07;

    chip8::ops::disambiguate0x8(vm, 0x8015);

    chip8::Byte expected { static_cast<chip8::Byte>(5 - 7) };

    REQUIRE( vm.registers[0x0] == expected );
    REQUIRE( vm.registers[0xF] == 0x00 );
  }

  SECTION( "ops::disambiguate0x8 calls ops::subtractVxVyUpdateCarry, subtracts VY from VX, VF = 0 if there is a borrow" ) {
    vm.registers[0x0] = 0x00;
    vm.registers[0x1] = 0x01;

    chip8::ops::disambiguate0x8(vm, 0x8015);

    REQUIRE( vm.registers[0x0] == 0xFF );
    REQUIRE( vm.registers[0xF] == 0x00 );
  }

  SECTION( "ops::rightshiftVx shifts VX right by 1, saves least-significant bit in VF before shifting (0 bit)" ) {
    vm.registers[0x0] = 0b00000010;

    chip8::ops::rightshiftVx(vm, 0x8016);

    REQUIRE( vm.registers[0x0] == 0b00000001 );
    REQUIRE( vm.registers[0xF] == 0 );
  }

  SECTION( "ops::rightshiftVx shifts VX right by 1, saves least-significant bit in VF before shifting (1 bit)" ) {
    vm.registers[0x0] = 0b00001101;

    chip8::ops::rightshiftVx(vm, 0x8016);

    REQUIRE( vm.registers[0x0] == 0b00000110 );
    REQUIRE( vm.registers[0xF] == 1 );
  }

  SECTION( "ops::disambiguate0x8 calls ops::rightshiftVx, shifts VX right by 1, saves least-significant bit in VF before shifting (0 bit)" ) {
    vm.registers[0x0] = 0b00000010;

    chip8::ops::rightshiftVx(vm, 0x8016);

    REQUIRE( vm.registers[0x0] == 0b00000001 );
    REQUIRE( vm.registers[0xF] == 0 );
  }

  SECTION( "ops::disambiguate0x8 calls ops::rightshiftVx, shifts VX right by 1, saves least-significant bit in VF before shifting (1 bit)" ) {
    vm.registers[0x0] = 0b00001101;

    chip8::ops::rightshiftVx(vm, 0x8016);

    REQUIRE( vm.registers[0x0] == 0b00000110 );
    REQUIRE( vm.registers[0xF] == 1 );
  }

  SECTION( "ops::subtractVxFromVyUpdateCarry subtracts VY from VX, VF = 1 if there's no borrow" ) {
    vm.registers[0x0] = 0x01;
    vm.registers[0x1] = 0xFF;

    chip8::ops::subtractVxFromVyUpdateCarry(vm, 0x8017);

    REQUIRE( vm.registers[0x0] == 0xFE );
    REQUIRE( vm.registers[0xF] == 0x01 );
  }

  SECTION( "ops::subtractVxFromVyUpdateCarry subtracts VY from VX, VF = 0 if there is a borrow" ) {
    vm.registers[0x0] = 0x07;
    vm.registers[0x1] = 0x05;

    chip8::ops::subtractVxFromVyUpdateCarry(vm, 0x8017);

    chip8::Byte expected { static_cast<chip8::Byte>(5 - 7) };

    REQUIRE( vm.registers[0x0] == expected );
    REQUIRE( vm.registers[0xF] == 0x00 );
  }

  SECTION( "ops::subtractVxFromVyUpdateCarry subtracts VY from VX, VF = 0 if there is a borrow" ) {
    vm.registers[0x0] = 0x01;
    vm.registers[0x1] = 0x00;

    chip8::ops::subtractVxFromVyUpdateCarry(vm, 0x8017);

    REQUIRE( vm.registers[0x0] == 0xFF );
    REQUIRE( vm.registers[0xF] == 0x00 );
  }

  SECTION( "ops::disambiguate0x8 calls ops::subtractVxFromVyUpdateCarry, subtracts VY from VX, VF = 1 if there's no borrow" ) {
    vm.registers[0x0] = 0x01;
    vm.registers[0x1] = 0xFF;

    chip8::ops::disambiguate0x8(vm, 0x8017);

    REQUIRE( vm.registers[0x0] == 0xFE );
    REQUIRE( vm.registers[0xF] == 0x01 );
  }

  SECTION( "ops::disambiguate0x8 calls ops::subtractVxFromVyUpdateCarry, subtracts VY from VX, VF = 0 if there is a borrow" ) {
    vm.registers[0x0] = 0x07;
    vm.registers[0x1] = 0x05;

    chip8::ops::disambiguate0x8(vm, 0x8017);

    chip8::Byte expected { static_cast<chip8::Byte>(5 - 7) };

    REQUIRE( vm.registers[0x0] == expected );
    REQUIRE( vm.registers[0xF] == 0x00 );
  }

  SECTION( "ops::disambiguate0x8 calls ops::subtractVxFromVyUpdateCarry, subtracts VY from VX, VF = 0 if there is a borrow" ) {
    vm.registers[0x0] = 0x01;
    vm.registers[0x1] = 0x00;

    chip8::ops::disambiguate0x8(vm, 0x8017);

    REQUIRE( vm.registers[0x0] == 0xFF );
    REQUIRE( vm.registers[0xF] == 0x00 );
  }

    SECTION( "ops::leftshiftVx shifts VX left by 1, saves most-significant bit in VF before shifting (0 bit)" ) {
    vm.registers[0x0] = 0b01000000;

    chip8::ops::leftshiftVx(vm, 0x801E);

    REQUIRE( vm.registers[0x0] == 0b10000000 );
    REQUIRE( vm.registers[0xF] == 0 );
  }

  SECTION( "ops::leftshiftVx shifts VX left by 1, saves most-significant bit in VF before shifting (1 bit)" ) {
    vm.registers[0x0] = 0b10001100;

    chip8::ops::leftshiftVx(vm, 0x801E);

    REQUIRE( vm.registers[0x0] == 0b00011000 );
    REQUIRE( vm.registers[0xF] == 1 );
  }

  SECTION( "ops::disambiguate0x8 calls ops::leftshiftVx, shifts VX left by 1, saves most-significant bit in VF before shifting (0 bit)" ) {
    vm.registers[0x0] = 0b01000000;

    chip8::ops::leftshiftVx(vm, 0x801E);

    REQUIRE( vm.registers[0x0] == 0b10000000 );
    REQUIRE( vm.registers[0xF] == 0 );
  }

  SECTION( "ops::disambiguate0x8 calls ops::leftshiftVx, shifts VX left by 1, saves most-significant bit in VF before shifting (1 bit)" ) {
    vm.registers[0x0] = 0b10001100;

    chip8::ops::leftshiftVx(vm, 0x801E);

    REQUIRE( vm.registers[0x0] == 0b00011000 );
    REQUIRE( vm.registers[0xF] == 1 );
  }

  SECTION( "ops::skipIfVxNotEqualsVy increments the program counter by 2 if VX != VY" ) {
    vm.registers[0x3] = 0x42;
    vm.registers[0xD] = 0x24;
    auto pc = vm.programCounter;

    chip8::ops::skipIfVxNotEqualsVy(vm, 0x93D0);

    REQUIRE( vm.programCounter == (pc + 2) );
  }

  SECTION( "ops::skipIfVxNotEqualsVy does not increment the program counter by 2 if VX == VY" ) {
    vm.registers[0x3] = 0x42;
    vm.registers[0xD] = 0x42;
    auto pc = vm.programCounter;

    chip8::ops::skipIfVxNotEqualsVy(vm, 0x93D0);

    REQUIRE( vm.programCounter == pc );
  }

  SECTION( "ops::setIToAddress sets I to the specified address" ) {
    chip8::ops::setIToAddress(vm, 0xA123);

    REQUIRE( vm.I == 0x123 );
  }

  SECTION( "ops::jumpPlusV0 changes the program counter to NNN + V0" ) {
    vm.registers[0] = 0x42;

    chip8::ops::jumpPlusV0(vm, 0xB123);

    REQUIRE( vm.programCounter == (0x123 + 0x42) );
  }

  SECTION( "ops::randomVxModNn sets VX to a random value anded by NN" ) {
    vm.rng = [](chip8::Byte seed) { return 0x42; };

    chip8::ops::randomVxModNn(vm, 0xC013);

    REQUIRE( vm.registers[0] == (0x13 & 0x42) );
  }

  SECTION( "ops::blit draws sprites to the graphics buffer at the correct X position" ) {
    vm.memory[0] = 0b10000001;
    vm.registers[0] = 0x5;
    vm.registers[1] = 0x0;

    chip8::ops::blit(vm, 0xD011);

    REQUIRE( vm.graphics[0] == 0b0000010000001000000000000000000000000000000000000000000000000000 );
  }

  SECTION( "ops::blit draws sprites to the graphics buffer at the correct Y position" ) {
    vm.memory[0] = 0b10000001;
    vm.registers[0] = 0x0;
    vm.registers[1] = 0x3;

    chip8::ops::blit(vm, 0xD011);

    REQUIRE( vm.graphics[3] == 0b1000000100000000000000000000000000000000000000000000000000000000 );
  }

  SECTION( "ops::blit draws sprites to the graphics buffer with the correct height at the correct coordinate" ) {
    vm.memory[0] = 0b11111111;
    vm.memory[1] = 0b10000001;
    vm.memory[2] = 0b10000001;
    vm.memory[3] = 0b10111101;
    vm.registers[0] = 0x1;
    vm.registers[1] = 0x3;

    chip8::ops::blit(vm, 0xD014);

    REQUIRE( vm.graphics[0] == 0b0000000000000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.graphics[1] == 0b0000000000000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.graphics[2] == 0b0000000000000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.graphics[3] == 0b0111111110000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.graphics[4] == 0b0100000010000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.graphics[5] == 0b0100000010000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.graphics[6] == 0b0101111010000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.graphics[7] == 0b0000000000000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.registers[0xF] == 0 );
  }

  SECTION( "ops::blit draws sprites to the graphics buffer, wrapping if necessary horizontally" ) {
    vm.memory[0] = 0b11111111;
    vm.memory[1] = 0b10000001;
    vm.memory[2] = 0b10000001;
    vm.memory[3] = 0b10111101;
    vm.registers[0] = 0x3C; // 60 pixels to the right
    vm.registers[1] = 0x3;

    chip8::ops::blit(vm, 0xD014);

    REQUIRE( vm.graphics[0] == 0b0000000000000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.graphics[1] == 0b0000000000000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.graphics[2] == 0b0000000000000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.graphics[3] == 0b1111000000000000000000000000000000000000000000000000000000001111 );
    REQUIRE( vm.graphics[4] == 0b0001000000000000000000000000000000000000000000000000000000001000 );
    REQUIRE( vm.graphics[5] == 0b0001000000000000000000000000000000000000000000000000000000001000 );
    REQUIRE( vm.graphics[6] == 0b1101000000000000000000000000000000000000000000000000000000001011 );
    REQUIRE( vm.graphics[7] == 0b0000000000000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.registers[0xF] == 0 );
  }

  SECTION( "ops::blit draws sprites to the graphics buffer, wrapping if necessary vertically" ) {
    vm.memory[0] = 0b11111111;
    vm.memory[1] = 0b10000001;
    vm.memory[2] = 0b10000001;
    vm.memory[3] = 0b10111101;
    vm.registers[0] = 0x3C;
    vm.registers[1] = 0x1E; // 30 pixels from the top

    chip8::ops::blit(vm, 0xD014);

    REQUIRE( vm.graphics[0] == 0b0001000000000000000000000000000000000000000000000000000000001000 );
    REQUIRE( vm.graphics[1] == 0b1101000000000000000000000000000000000000000000000000000000001011 );
    REQUIRE( vm.graphics[2] == 0b0000000000000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.graphics[30] == 0b1111000000000000000000000000000000000000000000000000000000001111 );
    REQUIRE( vm.graphics[31] == 0b0001000000000000000000000000000000000000000000000000000000001000 );
    REQUIRE( vm.registers[0xF] == 0 );
  }

  SECTION( "ops::blit inverts pixels and sets VF to 1 if a pixel was turned off" ) {
    vm.memory[0] = 0b11111111;
    vm.memory[1] = 0b10000001;
    vm.memory[2] = 0b10000001;
    vm.memory[3] = 0b10111101;
    vm.registers[0] = 0x1;
    vm.registers[1] = 0x3;

    chip8::ops::blit(vm, 0xD014);

    REQUIRE( vm.registers[0xF] == 0 );

    vm.memory[0] = 0b10111101;

    chip8::ops::blit(vm, 0xD014);

    REQUIRE( vm.graphics[0] == 0b0000000000000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.graphics[1] == 0b0000000000000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.graphics[2] == 0b0000000000000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.graphics[3] == 0b0010000100000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.graphics[4] == 0b0000000000000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.graphics[5] == 0b0000000000000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.graphics[6] == 0b0000000000000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.graphics[7] == 0b0000000000000000000000000000000000000000000000000000000000000000 );
    REQUIRE( vm.registers[0xF] == 1 );
  }

  SECTION( "ops::skipIfKeyIsPressed increments the program counter by 2 if the key stored in VX is pressed" ) {
    vm.registers[0x1] = 2; // Look up key 2.
    vm.keyboard[0x2] = 1; // Turn key 2 on.
    auto pc = vm.programCounter;

    chip8::ops::skipIfKeyIsPressed(vm, 0xE19E);

    REQUIRE( vm.programCounter == (pc + 2) );
  }

  SECTION( "ops::skipIfKeyIsPressed does not increment the program counter by 2 if the key stored in VX is not pressed" ) {
    vm.registers[0x1] = 2; // Look up key 2.
    vm.keyboard[0x2] = 0; // Turn key 2 off.
    auto pc = vm.programCounter;

    chip8::ops::skipIfKeyIsPressed(vm, 0xE19E);

    REQUIRE( vm.programCounter == pc );
  }

  SECTION( "ops::disambiguate0xE calls ops::skipIfKeyIsPressed, increments the program counter by 2 if the key stored in VX is pressed" ) {
    vm.registers[0x1] = 2; // Look up key 2.
    vm.keyboard[0x2] = 1; // Turn key 2 on.
    auto pc = vm.programCounter;

    chip8::ops::disambiguate0xE(vm, 0xE19E);

    REQUIRE( vm.programCounter == (pc + 2) );
  }

  SECTION( "ops::disambiguate0xE calls ops::skipIfKeyIsPressed, does not increment the program counter by 2 if the key stored in VX is not pressed" ) {
    vm.registers[0x1] = 2; // Look up key 2.
    vm.keyboard[0x2] = 0; // Turn key 2 off.
    auto pc = vm.programCounter;

    chip8::ops::disambiguate0xE(vm, 0xE19E);

    REQUIRE( vm.programCounter == pc );
  }

  SECTION( "ops::skipIfKeyIsNotPressed increments the program counter by 2 if the key stored in VX is not pressed" ) {
    vm.registers[0x1] = 2; // Look up key 2.
    vm.keyboard[0x2] = 0; // Turn key 2 off.
    auto pc = vm.programCounter;

    chip8::ops::skipIfKeyIsNotPressed(vm, 0xE1A1);

    REQUIRE( vm.programCounter == (pc + 2) );
  }

  SECTION( "ops::skipIfKeyIsNotPressed does not increment the program counter by 2 if the key stored in VX is pressed" ) {
    vm.registers[0x1] = 2; // Look up key 2.
    vm.keyboard[0x2] = 1; // Turn key 2 on.
    auto pc = vm.programCounter;

    chip8::ops::skipIfKeyIsNotPressed(vm, 0xE1A1);

    REQUIRE( vm.programCounter == pc );
  }

  SECTION( "ops::disambiguate0xE calls ops::skipIfKeyIsNotPressed, increments the program counter by 2 if the key stored in VX is not pressed" ) {
    vm.registers[0x1] = 2; // Look up key 2.
    vm.keyboard[0x2] = 0; // Turn key 2 off.
    auto pc = vm.programCounter;

    chip8::ops::disambiguate0xE(vm, 0xE1A1);

    REQUIRE( vm.programCounter == (pc + 2) );
  }

  SECTION( "ops::disambiguate0xE calls ops::skipIfKeyIsNotPressed, does not increment the program counter by 2 if the key stored in VX is pressed" ) {
    vm.registers[0x1] = 2; // Look up key 2.
    vm.keyboard[0x2] = 1; // Turn key 2 on.
    auto pc = vm.programCounter;

    chip8::ops::disambiguate0xE(vm, 0xE1A1);

    REQUIRE( vm.programCounter == pc );
  }

  SECTION( "ops::setVxToDelayTimer sets VX to the value of the delay timer" ) {
    vm.timers.delay = 0x33;

    REQUIRE( vm.registers[0xA] == 0 );

    chip8::ops::setVxToDelayTimer(vm, 0xFA07);

    REQUIRE( vm.registers[0xA] == 0x33 );
  }

  SECTION( "ops::disambiguate0xF calls ops::setVxToDelayTimer, sets VX to the value of the delay timer" ) {
    vm.timers.delay = 0x33;

    REQUIRE( vm.registers[0xA] == 0 );

    chip8::ops::disambiguate0xF(vm, 0xFA07);

    REQUIRE( vm.registers[0xA] == 0x33 );
  }

  SECTION( "ops::waitForKeyPress sets the wait flag to true, stores VX in nextKeypressRegister" ) {
    chip8::ops::waitForKeyPress(vm, 0xF20A);

    REQUIRE( vm.awaitingKeypress == true );
    REQUIRE( vm.nextKeypressRegister == 0x2 );
  }

  SECTION( "ops::disambiguate0xF calls ops::waitForKeyPress, sets the wait flag to true, stores VX in nextKeypressRegister" ) {
    chip8::ops::disambiguate0xF(vm, 0xF20A);

    REQUIRE( vm.awaitingKeypress == true );
    REQUIRE( vm.nextKeypressRegister == 0x2 );
  }

  SECTION( "ops::setDelayTimer sets the delay timer to the value of VX" ) {
    vm.registers[0x4] = 0x55;

    chip8::ops::setDelayTimer(vm, 0xF415);

    REQUIRE( vm.timers.delay == 0x55 );
  }

  SECTION( "ops::disambiguate0xF calls ops::setDelayTimer, sets the delay timer to the value of VX" ) {
    vm.registers[0x4] = 0x55;

    chip8::ops::disambiguate0xF(vm, 0xF415);

    REQUIRE( vm.timers.delay == 0x55 );
  }

  SECTION( "ops::setSoundTimer sets the sound timer to the value of VX" ) {
    vm.registers[0x3] = 0x77;

    chip8::ops::setSoundTimer(vm, 0xF318);

    REQUIRE( vm.timers.sound == 0x77 );
  }

  SECTION( "ops::disambiguate0xF calls ops::setSoundTimer, sets the sound timer to the value of VX" ) {
    vm.registers[0x3] = 0x77;

    chip8::ops::disambiguate0xF(vm, 0xF318);

    REQUIRE( vm.timers.sound == 0x77 );
  }

  SECTION( "ops::addVxToI adds VX to I, stores result in I" ) {
    vm.I = 0x21;
    vm.registers[0x7] = 0x41;

    chip8::ops::addVxToI(vm, 0xF71E);

    REQUIRE( vm.I == (0x21 + 0x41) );
  }

  SECTION( "ops::disambiguate0xF calls ops::addVxToI, adds VX to I, stores result in I" ) {
    vm.I = 0x21;
    vm.registers[0x7] = 0x41;

    chip8::ops::disambiguate0xF(vm, 0xF71E);

    REQUIRE( vm.I == (0x21 + 0x41) );
  }

  SECTION( "ops::setIToCharacter sets I to the correct memory location" ) {
    vm.registers[0x6] = 0x0;
    chip8::ops::setIToCharacter(vm, 0xF629);
    REQUIRE( vm.I == 0x0 );

    vm.registers[0x6] = 0x1;
    chip8::ops::setIToCharacter(vm, 0xF629);
    REQUIRE( vm.I == 0x5 );

    vm.registers[0x6] = 0x2;
    chip8::ops::setIToCharacter(vm, 0xF629);
    REQUIRE( vm.I == 0x5 * 2 );

    vm.registers[0x6] = 0x3;
    chip8::ops::setIToCharacter(vm, 0xF629);
    REQUIRE( vm.I == 0x5 * 3 );

    vm.registers[0x6] = 0x4;
    chip8::ops::setIToCharacter(vm, 0xF629);
    REQUIRE( vm.I == 0x5 * 4 );

    vm.registers[0x6] = 0x5;
    chip8::ops::setIToCharacter(vm, 0xF629);
    REQUIRE( vm.I == 0x5 * 5 );

    vm.registers[0x6] = 0x6;
    chip8::ops::setIToCharacter(vm, 0xF629);
    REQUIRE( vm.I == 0x5 * 6 );

    vm.registers[0x6] = 0x7;
    chip8::ops::setIToCharacter(vm, 0xF629);
    REQUIRE( vm.I == 0x5 * 7 );

    vm.registers[0x6] = 0x8;
    chip8::ops::setIToCharacter(vm, 0xF629);
    REQUIRE( vm.I == 0x5 * 8 );

    vm.registers[0x6] = 0x9;
    chip8::ops::setIToCharacter(vm, 0xF629);
    REQUIRE( vm.I == 0x5 * 9 );

    vm.registers[0x6] = 0xA;
    chip8::ops::setIToCharacter(vm, 0xF629);
    REQUIRE( vm.I == 0x5 * 10 );

    vm.registers[0x6] = 0xB;
    chip8::ops::setIToCharacter(vm, 0xF629);
    REQUIRE( vm.I == 0x5 * 11 );

    vm.registers[0x6] = 0xC;
    chip8::ops::setIToCharacter(vm, 0xF629);
    REQUIRE( vm.I == 0x5 * 12 );

    vm.registers[0x6] = 0xD;
    chip8::ops::setIToCharacter(vm, 0xF629);
    REQUIRE( vm.I == 0x5 * 13 );

    vm.registers[0x6] = 0xE;
    chip8::ops::setIToCharacter(vm, 0xF629);
    REQUIRE( vm.I == 0x5 * 14 );

    vm.registers[0x6] = 0xF;
    chip8::ops::setIToCharacter(vm, 0xF629);
    REQUIRE( vm.I == 0x5 * 15 );
  }

  SECTION( "ops::disambiguate0xF calls ops::setIToCharacter, sets I to the correct memory location" ) {
    vm.registers[0x6] = 0x0;
    chip8::ops::disambiguate0xF(vm, 0xF629);
    REQUIRE( vm.I == 0x0 );

    vm.registers[0x6] = 0xF;
    chip8::ops::disambiguate0xF(vm, 0xF629);
    REQUIRE( vm.I == 0x5 * 15 );
  }

  SECTION( "ops::storeBcdOfVx stores the binary-coded decimal value of VX at I" ) {
    vm.I = 10;
    vm.registers[0xE] = 128;

    chip8::ops::storeBcdOfVx(vm, 0xFE33);

    REQUIRE( vm.memory[10] == 1 );
    REQUIRE( vm.memory[11] == 2 );
    REQUIRE( vm.memory[12] == 8 );
  }

  SECTION( "ops::disambiguate0xF calls ops::storeBcdOfVx, stores the binary-coded decimal value of VX at I" ) {
    vm.I = 10;
    vm.registers[0xE] = 128;

    chip8::ops::disambiguate0xF(vm, 0xFE33);

    REQUIRE( vm.memory[10] == 1 );
    REQUIRE( vm.memory[11] == 2 );
    REQUIRE( vm.memory[12] == 8 );
  }

  SECTION( "ops::storeV0ToVx store V0 to VX in memory starting at I" ) {
    vm.I = 100;
    vm.registers[0x0] = 8;
    vm.registers[0x1] = 6;
    vm.registers[0x2] = 7;
    vm.registers[0x3] = 5;
    vm.registers[0x4] = 3;
    vm.registers[0x5] = 0;
    vm.registers[0x6] = 9;
    vm.registers[0x7] = 41;
    vm.registers[0x8] = 77;
    vm.registers[0x9] = 128;
    vm.registers[0xA] = 2;
    vm.registers[0xB] = 254;
    vm.registers[0xC] = 99;
    vm.registers[0xD] = 33;
    vm.registers[0xE] = 56;
    vm.registers[0xF] = 1;

    chip8::ops::storeV0ToVx(vm, 0xFF55);

    REQUIRE( vm.memory[100 + 0x0] == 8 );
    REQUIRE( vm.memory[100 + 0x1] == 6 );
    REQUIRE( vm.memory[100 + 0x2] == 7 );
    REQUIRE( vm.memory[100 + 0x3] == 5 );
    REQUIRE( vm.memory[100 + 0x4] == 3 );
    REQUIRE( vm.memory[100 + 0x5] == 0 );
    REQUIRE( vm.memory[100 + 0x6] == 9 );
    REQUIRE( vm.memory[100 + 0x7] == 41 );
    REQUIRE( vm.memory[100 + 0x8] == 77 );
    REQUIRE( vm.memory[100 + 0x9] == 128 );
    REQUIRE( vm.memory[100 + 0xA] == 2 );
    REQUIRE( vm.memory[100 + 0xB] == 254 );
    REQUIRE( vm.memory[100 + 0xC] == 99 );
    REQUIRE( vm.memory[100 + 0xD] == 33 );
    REQUIRE( vm.memory[100 + 0xE] == 56 );
    REQUIRE( vm.memory[100 + 0xF] == 1 );
  }

  SECTION( "ops::disambiguate0xF calls ops::storeV0ToVx, store V0 to VX in memory starting at I" ) {
    vm.I = 100;
    vm.registers[0x0] = 8;
    vm.registers[0x1] = 6;
    vm.registers[0x2] = 7;
    vm.registers[0x3] = 5;
    vm.registers[0x4] = 3;
    vm.registers[0x5] = 0;
    vm.registers[0x6] = 9;
    vm.registers[0x7] = 41;
    vm.registers[0x8] = 77;
    vm.registers[0x9] = 128;
    vm.registers[0xA] = 2;
    vm.registers[0xB] = 254;
    vm.registers[0xC] = 99;
    vm.registers[0xD] = 33;
    vm.registers[0xE] = 56;
    vm.registers[0xF] = 1;

    chip8::ops::disambiguate0xF(vm, 0xFF55);

    REQUIRE( vm.memory[100 + 0x0] == 8 );
    REQUIRE( vm.memory[100 + 0x1] == 6 );
    REQUIRE( vm.memory[100 + 0x2] == 7 );
    REQUIRE( vm.memory[100 + 0x3] == 5 );
    REQUIRE( vm.memory[100 + 0x4] == 3 );
    REQUIRE( vm.memory[100 + 0x5] == 0 );
    REQUIRE( vm.memory[100 + 0x6] == 9 );
    REQUIRE( vm.memory[100 + 0x7] == 41 );
    REQUIRE( vm.memory[100 + 0x8] == 77 );
    REQUIRE( vm.memory[100 + 0x9] == 128 );
    REQUIRE( vm.memory[100 + 0xA] == 2 );
    REQUIRE( vm.memory[100 + 0xB] == 254 );
    REQUIRE( vm.memory[100 + 0xC] == 99 );
    REQUIRE( vm.memory[100 + 0xD] == 33 );
    REQUIRE( vm.memory[100 + 0xE] == 56 );
    REQUIRE( vm.memory[100 + 0xF] == 1 );
  }

  SECTION( "ops::loadV0ToVx loads V0 to VX with values from memory starting at I" ) {
    vm.I = 100;
    vm.memory[100 + 0x0] = 8;
    vm.memory[100 + 0x1] = 6;
    vm.memory[100 + 0x2] = 7;
    vm.memory[100 + 0x3] = 5;
    vm.memory[100 + 0x4] = 3;
    vm.memory[100 + 0x5] = 0;
    vm.memory[100 + 0x6] = 9;
    vm.memory[100 + 0x7] = 41;
    vm.memory[100 + 0x8] = 77;
    vm.memory[100 + 0x9] = 128;
    vm.memory[100 + 0xA] = 2;
    vm.memory[100 + 0xB] = 254;
    vm.memory[100 + 0xC] = 99;
    vm.memory[100 + 0xD] = 33;
    vm.memory[100 + 0xE] = 56;
    vm.memory[100 + 0xF] = 1;

    chip8::ops::loadV0ToVx(vm, 0xFF65);

    REQUIRE( vm.registers[0x0] == 8 );
    REQUIRE( vm.registers[0x1] == 6 );
    REQUIRE( vm.registers[0x2] == 7 );
    REQUIRE( vm.registers[0x3] == 5 );
    REQUIRE( vm.registers[0x4] == 3 );
    REQUIRE( vm.registers[0x5] == 0 );
    REQUIRE( vm.registers[0x6] == 9 );
    REQUIRE( vm.registers[0x7] == 41 );
    REQUIRE( vm.registers[0x8] == 77 );
    REQUIRE( vm.registers[0x9] == 128 );
    REQUIRE( vm.registers[0xA] == 2 );
    REQUIRE( vm.registers[0xB] == 254 );
    REQUIRE( vm.registers[0xC] == 99 );
    REQUIRE( vm.registers[0xD] == 33 );
    REQUIRE( vm.registers[0xE] == 56 );
    REQUIRE( vm.registers[0xF] == 1 );
  }

  SECTION( "ops::disambiguate0xF calls ops::loadV0ToVx, loads V0 to VX with values from memory starting at I" ) {
    vm.I = 100;
    vm.memory[100 + 0x0] = 8;
    vm.memory[100 + 0x1] = 6;
    vm.memory[100 + 0x2] = 7;
    vm.memory[100 + 0x3] = 5;
    vm.memory[100 + 0x4] = 3;
    vm.memory[100 + 0x5] = 0;
    vm.memory[100 + 0x6] = 9;
    vm.memory[100 + 0x7] = 41;
    vm.memory[100 + 0x8] = 77;
    vm.memory[100 + 0x9] = 128;
    vm.memory[100 + 0xA] = 2;
    vm.memory[100 + 0xB] = 254;
    vm.memory[100 + 0xC] = 99;
    vm.memory[100 + 0xD] = 33;
    vm.memory[100 + 0xE] = 56;
    vm.memory[100 + 0xF] = 1;

    chip8::ops::disambiguate0xF(vm, 0xFF65);

    REQUIRE( vm.registers[0x0] == 8 );
    REQUIRE( vm.registers[0x1] == 6 );
    REQUIRE( vm.registers[0x2] == 7 );
    REQUIRE( vm.registers[0x3] == 5 );
    REQUIRE( vm.registers[0x4] == 3 );
    REQUIRE( vm.registers[0x5] == 0 );
    REQUIRE( vm.registers[0x6] == 9 );
    REQUIRE( vm.registers[0x7] == 41 );
    REQUIRE( vm.registers[0x8] == 77 );
    REQUIRE( vm.registers[0x9] == 128 );
    REQUIRE( vm.registers[0xA] == 2 );
    REQUIRE( vm.registers[0xB] == 254 );
    REQUIRE( vm.registers[0xC] == 99 );
    REQUIRE( vm.registers[0xD] == 33 );
    REQUIRE( vm.registers[0xE] == 56 );
    REQUIRE( vm.registers[0xF] == 1 );
  }
}