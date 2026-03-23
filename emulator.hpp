#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

class Emulator {
public:
  using Word = int32_t;

private:
  struct EmulatorState {
    static constexpr size_t regs_size = 4;
    static constexpr size_t ram_size = 1024;
    Word regs[regs_size];
    Word ram[ram_size];
    size_t pc = 0;
  };

  class Instruction {
  public:
    virtual void eval(EmulatorState& st) = 0;
    virtual ~Instruction() {};
  };

  enum Register { R0, R1, R2, R3 };

  struct RR {
    Register rd, rs;
    RR(Register rd, Register rs) : rd(rd), rs(rs) {}
  };
  struct RI {
    Register rd;
    Word imm;
    RI(Register rd, Word imm) : rd(rd), imm(imm) {};
  };
  struct RM {
    Register rd;
    Word imm;
    RM(Register rd, Word imm) : rd(rd), imm(imm) {};
  };
  struct MR {
    Register rs;
    Word imm;
    MR(Register rs, Word imm) : rs(rs), imm(imm) {};
  };
  struct J {
    Word imm;
    J(Word imm) : imm(imm) {};
  };

  struct Load : public RM, public Instruction {
    Load(Register rd, Word imm) : RM(rd, imm) {};
    void eval(EmulatorState& st) override {
      st.regs[rd] = st.ram[imm];
      ++st.pc;
    }
  };
  struct Store : public MR, public Instruction {
    Store(Register rs, Word imm) : MR(rs, imm) {};
    void eval(EmulatorState& st) override {
      st.ram[imm] = st.regs[rs];
      ++st.pc;
    }
  };
  struct Jmp : public J, public Instruction {
    Jmp(Word imm) : J(imm) {};
    void eval(EmulatorState& st) override { st.pc = imm; }
  };
  struct Jmpz : public J, public Instruction {
    Jmpz(Word imm) : J(imm) {};
    void eval(EmulatorState& st) override {
      if (st.regs[Register::R0] == 0) {
        st.pc = imm;
      } else {
        ++st.pc;
      }
    }
  };

  template <typename Op>
  struct SimpleRR : public RR, public Instruction {
    SimpleRR(Register rd, Register rs) : RR(rd, rs) {}
    void eval(EmulatorState& st) override {
      Op::apply(st.regs[rd], st.regs[rs]);
      ++st.pc;
    }
  };
  template <typename Op>
  struct SimpleRI : public RI, public Instruction {
    SimpleRI(Register rd, Word imm) : RI(rd, imm) {}
    void eval(EmulatorState& st) override {
      Op::apply(st.regs[rd], imm);
      ++st.pc;
    }
  };

  struct _Mov {
    static void apply(Word& rd, const Word& rhs) { rd = rhs; }
  };
  struct _Add {
    static void apply(Word& rd, const Word& rhs) { rd += rhs; }
  };
  struct _Sub {
    static void apply(Word& rd, const Word& rhs) { rd -= rhs; }
  };
  struct _Mul {
    static void apply(Word& rd, const Word& rhs) { rd *= rhs; }
  };
  struct _Div {
    static void apply(Word& rd, const Word& rhs) { rd /= rhs; }
  };

  using MovRR = SimpleRR<_Mov>;
  using MovRI = SimpleRI<_Mov>;
  using AddRR = SimpleRR<_Add>;
  using AddRI = SimpleRI<_Add>;
  using SubRR = SimpleRR<_Sub>;
  using SubRI = SimpleRI<_Sub>;
  using MulRR = SimpleRR<_Mul>;
  using MulRI = SimpleRI<_Mul>;
  using DivRR = SimpleRR<_Div>;
  using DivRI = SimpleRI<_Div>;

  enum class TokenType {
    Ident,
    Number,
  };

  struct Token {
    TokenType type;
    std::string value;
  };

  static std::vector<Token> lexer(const std::string& input) {
    std::vector<Token> result{{TokenType::Ident, ""}};

    for (char ch : input) {
      if (std::isspace(ch)) {
        if (!result.back().value.empty()) {
          result.push_back({TokenType::Ident, ""});
        }
        continue;
      }

      switch (result.back().type) {
      case TokenType::Ident:
        if (result.back().value.empty()) {
          if (ch == '-' || std::isdigit(ch)) {
            result.back().type = TokenType::Number;
          } else if (!std::isalpha(ch)) {
            throw std::invalid_argument("invalid token");
          }
        } else {
          if (!std::isalnum(ch)) {
            throw std::invalid_argument("invalid token");
          }
        }
        break;
      case TokenType::Number:
        if (!std::isdigit(ch)) {
          throw std::invalid_argument("invalid token");
        }
        break;
      }
      result.back().value += ch;
    }
    if (result.back().value.empty()) {
      result.pop_back();
    }

    return result;
  }

  static Register parse_register(const std::string& str) {
    if (str == "R0") {
      return Register::R0;
    } else if (str == "R1") {
      return Register::R1;
    } else if (str == "R2") {
      return Register::R2;
    } else if (str == "R3") {
      return Register::R3;
    } else {
      throw std::invalid_argument("Invalid register name");
    }
  }

  static std::vector<std::unique_ptr<Instruction>>
  parse(const std::vector<Token>& tokens) {
    std::vector<std::unique_ptr<Instruction>> result;

    for (size_t i = 0; i < tokens.size();) {
      if (tokens[i].type != TokenType::Ident) {
        throw std::invalid_argument("Invalid token sequence");
      }

      auto mnemonic = tokens[i].value;
      std::unique_ptr<Instruction> instr;

      if (mnemonic == "Mov" || mnemonic == "Add" || mnemonic == "Sub" ||
          mnemonic == "Mul" || mnemonic == "Div" || mnemonic == "Load" ||
          mnemonic == "Store") {
        if (i + 2 >= tokens.size() && tokens[i + 1].type != TokenType::Ident) {
          throw std::invalid_argument("Invalid token sequence");
        }

        auto arg0 = parse_register(tokens[i + 1].value);
        if (tokens[i + 2].type == TokenType::Ident) {
          auto arg1 = parse_register(tokens[i + 2].value);
          if (mnemonic == "Mov") {
            instr = std::make_unique<MovRR>(arg0, arg1);
          } else if (mnemonic == "Add") {
            instr = std::make_unique<AddRR>(arg0, arg1);
          } else if (mnemonic == "Sub") {
            instr = std::make_unique<SubRR>(arg0, arg1);
          } else if (mnemonic == "Mul") {
            instr = std::make_unique<MulRR>(arg0, arg1);
          } else if (mnemonic == "Div") {
            instr = std::make_unique<DivRR>(arg0, arg1);
          }
        } else if (tokens[i + 2].type == TokenType::Number) {
          auto arg1 = std::stoi(tokens[i + 2].value);
          if (mnemonic == "Mov") {
            instr = std::make_unique<MovRI>(arg0, arg1);
          } else if (mnemonic == "Add") {
            instr = std::make_unique<AddRI>(arg0, arg1);
          } else if (mnemonic == "Sub") {
            instr = std::make_unique<SubRI>(arg0, arg1);
          } else if (mnemonic == "Mul") {
            instr = std::make_unique<MulRI>(arg0, arg1);
          } else if (mnemonic == "Div") {
            instr = std::make_unique<DivRI>(arg0, arg1);
          } else if (mnemonic == "Load") {
            instr = std::make_unique<Load>(arg0, arg1);
          } else if (mnemonic == "Store") {
            instr = std::make_unique<Store>(arg0, arg1);
          }
        }
        ++ ++ ++i;
      } else if (mnemonic == "Jmp" || mnemonic == "Jmpz") {
        if (i + 1 >= tokens.size() && tokens[i + 1].type != TokenType::Number) {
          throw std::invalid_argument("Invalid token sequence");
        }

        auto arg = std::stoi(tokens[i + 1].value);
        if (mnemonic == "Jmp") {
          instr = std::make_unique<Jmp>(arg);
        } else if (mnemonic == "Jmpz") {
          instr = std::make_unique<Jmpz>(arg);
        }
        ++ ++i;
      } else {
        throw std::invalid_argument("Invalid mnemonic");
      }

      result.push_back(std::move(instr));
    }

    return result;
  }

public:
  static Word emulate(const std::string& program_text) {
    auto tokens = lexer(program_text);
    auto program = parse(tokens);

    EmulatorState state;
    while (state.pc < program.size()) {
      program[state.pc]->eval(state);
    }
    return state.regs[Register::R0];
  }

  Word operator()(const std::string& program_text) const {
    return emulate(program_text);
  }
};
