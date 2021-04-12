#pragma once
#include "utils/Utils.h"

#include <iostream>
#include <cstdint>
#include <string_view>
#include <sstream>
#include <string>
#include <array>

namespace gbemu {


	class SharpSM83 {
	public:
		SharpSM83() {}
		~SharpSM83() {}

		void tick() {}

		std::string registersOut()
		{
			std::stringstream stream{};

			stream << "CPU registers:\n";

			stream << "A: "; toHexOutput(stream, A); stream << " F: "; toHexOutput(stream, static_cast<uint8_t>(AF & 0x00FF)); stream << "\n";

			stream << "B: "; toHexOutput(stream, B); stream << " C: "; toHexOutput(stream, C); stream << "\n";
			stream << "D: "; toHexOutput(stream, D); stream << " E: "; toHexOutput(stream, E); stream << "\n";
			stream << "H: "; toHexOutput(stream, H); stream << " L: "; toHexOutput(stream, L); stream << "\n";

			stream << "SP: "; toHexOutput(stream, SP); stream << "\n";
			stream << "PC: "; toHexOutput(stream, PC); stream << "\n";

			return stream.str();
		}
	private:
		uint8_t NOP(); uint8_t LD(); uint8_t INC(); uint8_t RLA(); uint8_t RLCA();
		uint8_t ADD(); uint8_t JR(); uint8_t DEC(); uint8_t RRA(); uint8_t RRCA();
		uint8_t SUB(); uint8_t OR(); uint8_t AND(); uint8_t XOR(); uint8_t PUSH();
		uint8_t ADC(); uint8_t JP(); uint8_t POP(); uint8_t RST(); uint8_t CALL();
		uint8_t SBC(); uint8_t DI(); uint8_t RET(); uint8_t CPL(); uint8_t RETI();
		uint8_t CCF(); uint8_t EI(); uint8_t LDH(); uint8_t DAA(); uint8_t HALT();
		uint8_t SCF();

	public:
		union {
			uint16_t AF{};
			
			struct {
				struct {
					uint8_t Unused : 4;
					uint8_t C : 1;
					uint8_t H : 1;
					uint8_t N : 1;
					uint8_t Z : 1;
				} Flags;

				uint8_t A;
			};
		};

		union {
			uint16_t BC{};

			struct {
				uint8_t C;
				uint8_t B;
			};
		};

		union {
			uint16_t DE{};

			struct {
				uint8_t E;
				uint8_t D;
			};
		};

		union {
			uint16_t HL{};

			struct {
				uint8_t L;
				uint8_t H;
			};
		};

		uint16_t SP{}, PC{};


		//8-bit registers lookup
		const std::array<uint8_t*, 8> REG = 
		{
			reinterpret_cast<uint8_t*>(&BC) + 1, reinterpret_cast<uint8_t*>(&BC) + 0, // &B, &C
			reinterpret_cast<uint8_t*>(&DE) + 1, reinterpret_cast<uint8_t*>(&DE) + 0, // &D, &E
			reinterpret_cast<uint8_t*>(&HL) + 1, reinterpret_cast<uint8_t*>(&HL) + 0, // &H, &L
			reinterpret_cast<uint8_t*>(&HL) + 0, reinterpret_cast<uint8_t*>(&AF) + 1  // &HL ([HL]), &A
		};

		//Register pair lookup with SP
		const std::array<uint16_t*, 4> REGP_SP = { &BC, &DE, &HL, &SP };

		//Register pair lookup with AF
		const std::array<uint16_t*, 4> REGP_AF = { &BC, &DE, &HL, &AF };
	};


	struct instruction {
		std::string_view mnemonic;
	};

	/// <summary>
	/// Struct to support easy opcode decomposition;
	/// </summary>
	struct opcode {
		union {
			uint8_t code;

			struct {

				uint8_t z : 3;

				union {

					uint8_t y : 3;

					struct {

						uint8_t q : 1;

						uint8_t p : 2;
					};
				};

				uint8_t x : 2;
			};
		};
	};
}