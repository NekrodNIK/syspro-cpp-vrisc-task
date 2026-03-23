#include "emulator.hpp"
#include <gtest/gtest.h>

TEST(SeparateInstructionSuite, MovTest) {
  auto result = Emulator::emulate(R"(
    Mov R0 30
  )");
  ASSERT_EQ(result, 30);
  
  result = Emulator::emulate(R"(
    Mov R0 -4200
    Mov R1 R0
  )");
  ASSERT_EQ(result, -4200);
}

TEST(SeparateInstructionSuite, AddTest) {
  auto result = Emulator::emulate(R"(
    Mov R0 10
    Mov R1 30
    Add R0 R1
  )");
  ASSERT_EQ(result, 40);

  result = Emulator::emulate(R"(
    Mov R0 10
    Add R0 -30
  )");
  ASSERT_EQ(result, -20);
}

TEST(SeparateInstructionSuite, SubTest) {
  auto result = Emulator::emulate(R"(
    Mov R0 10
    Mov R1 30
    Sub R0 R1
  )");
  ASSERT_EQ(result, -20);
  
  result = Emulator::emulate(R"(
    Mov R0 10
    Sub R0 -30
  )");
  ASSERT_EQ(result, 40);
}

TEST(SeparateInstructionSuite, MulTest) {
  auto result = Emulator::emulate(R"(
    Mov R0 10
    Mov R1 30
    Mul R0 R1
  )");
  ASSERT_EQ(result, 300);
  
  result = Emulator::emulate(R"(
    Mov R0 7
    Mul R0 6
  )");
  ASSERT_EQ(result, 42);
}

TEST(SeparateInstructionSuite, DivTest) {
  auto result = Emulator::emulate(R"(
    Mov R0 30
    Mov R1 -10
    Div R0 R1
  )");
  ASSERT_EQ(result, -3);
  
  result = Emulator::emulate(R"(
    Mov R0 -4200
    Div R0 -100
  )");
  ASSERT_EQ(result, 42);
}

TEST(SeparateInstructionSuite, StoreAndLoadTest) {
  auto result = Emulator::emulate(R"(
    Mov R1 42
    Store R1 100
    Load R0 100
  )");
  ASSERT_EQ(result, 42);
}

TEST(SeparateInstructionSuite, JumpTest) {
  auto result = Emulator::emulate(R"(
    Mov R0 1
    Jmp 3
    Mov R0 99
    Mov R0 42
  )");
  ASSERT_EQ(result, 42);
}

TEST(SeparateInstructionSuite, JumpzTest) {
  auto result = Emulator::emulate(R"(
    Mov R0 1
    Jmpz 5
    Sub R0 1
    Jmpz 5
    Mov R0 100    
    Add R0 42
  )");
  ASSERT_EQ(result, 42);
}

TEST(ExampleSuite, ExampleTest) {
  auto result = Emulator::emulate(R"(
    Mov R0 5
    Mov R1 1    
    Jmpz 6  
              
    Mul R1 R0   
    Sub R0 1    
    Jmp 2       

    Mov R0 R1
  )");
  ASSERT_EQ(result, 120);
}
