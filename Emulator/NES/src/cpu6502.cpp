#include "cpu6502.h"
#include "nesBus.h"
#include "emuNESStateInfoTypes.h"

#ifdef LOG_MODE
#include <stdio.h>
#endif

#include <cassert>

namespace nes
{
	constexpr uint16_t base_stack = (uint16_t)0x0100;

	CPU::CPU(SystemBus* nesBus) : bus(nesBus)
	{
		instructionsTable =
		{
			{ "BRK", &CPU::BRK, &CPU::IMM, 7 },{ "ORA", &CPU::ORA, &CPU::IZX, 6 },{ "???", &CPU::XXX, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 8 },{ "???", &CPU::NOP, &CPU::IMP, 3 },{ "ORA", &CPU::ORA, &CPU::ZP0, 3 },{ "ASL", &CPU::ASL, &CPU::ZP0, 5 },{ "???", &CPU::XXX, &CPU::IMP, 5 },{ "PHP", &CPU::PHP, &CPU::IMP, 3 },{ "ORA", &CPU::ORA, &CPU::IMM, 2 },{ "ASL", &CPU::ASL, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 2 },{ "???", &CPU::NOP, &CPU::IMP, 4 },{ "ORA", &CPU::ORA, &CPU::ABS, 4 },{ "ASL", &CPU::ASL, &CPU::ABS, 6 },{ "???", &CPU::XXX, &CPU::IMP, 6 },
			{ "BPL", &CPU::BPL, &CPU::REL, 2 },{ "ORA", &CPU::ORA, &CPU::IZY, 5 },{ "???", &CPU::XXX, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 8 },{ "???", &CPU::NOP, &CPU::IMP, 4 },{ "ORA", &CPU::ORA, &CPU::ZPX, 4 },{ "ASL", &CPU::ASL, &CPU::ZPX, 6 },{ "???", &CPU::XXX, &CPU::IMP, 6 },{ "CLC", &CPU::CLC, &CPU::IMP, 2 },{ "ORA", &CPU::ORA, &CPU::ABY, 4 },{ "???", &CPU::NOP, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 7 },{ "???", &CPU::NOP, &CPU::IMP, 4 },{ "ORA", &CPU::ORA, &CPU::ABX, 4 },{ "ASL", &CPU::ASL, &CPU::ABX, 7 },{ "???", &CPU::XXX, &CPU::IMP, 7 },
			{ "JSR", &CPU::JSR, &CPU::ABS, 6 },{ "AND", &CPU::AND, &CPU::IZX, 6 },{ "???", &CPU::XXX, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 8 },{ "BIT", &CPU::BIT, &CPU::ZP0, 3 },{ "AND", &CPU::AND, &CPU::ZP0, 3 },{ "ROL", &CPU::ROL, &CPU::ZP0, 5 },{ "???", &CPU::XXX, &CPU::IMP, 5 },{ "PLP", &CPU::PLP, &CPU::IMP, 4 },{ "AND", &CPU::AND, &CPU::IMM, 2 },{ "ROL", &CPU::ROL, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 2 },{ "BIT", &CPU::BIT, &CPU::ABS, 4 },{ "AND", &CPU::AND, &CPU::ABS, 4 },{ "ROL", &CPU::ROL, &CPU::ABS, 6 },{ "???", &CPU::XXX, &CPU::IMP, 6 },
			{ "BMI", &CPU::BMI, &CPU::REL, 2 },{ "AND", &CPU::AND, &CPU::IZY, 5 },{ "???", &CPU::XXX, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 8 },{ "???", &CPU::NOP, &CPU::IMP, 4 },{ "AND", &CPU::AND, &CPU::ZPX, 4 },{ "ROL", &CPU::ROL, &CPU::ZPX, 6 },{ "???", &CPU::XXX, &CPU::IMP, 6 },{ "SEC", &CPU::SEC, &CPU::IMP, 2 },{ "AND", &CPU::AND, &CPU::ABY, 4 },{ "???", &CPU::NOP, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 7 },{ "???", &CPU::NOP, &CPU::IMP, 4 },{ "AND", &CPU::AND, &CPU::ABX, 4 },{ "ROL", &CPU::ROL, &CPU::ABX, 7 },{ "???", &CPU::XXX, &CPU::IMP, 7 },
			{ "RTI", &CPU::RTI, &CPU::IMP, 6 },{ "EOR", &CPU::EOR, &CPU::IZX, 6 },{ "???", &CPU::XXX, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 8 },{ "???", &CPU::NOP, &CPU::IMP, 3 },{ "EOR", &CPU::EOR, &CPU::ZP0, 3 },{ "LSR", &CPU::LSR, &CPU::ZP0, 5 },{ "???", &CPU::XXX, &CPU::IMP, 5 },{ "PHA", &CPU::PHA, &CPU::IMP, 3 },{ "EOR", &CPU::EOR, &CPU::IMM, 2 },{ "LSR", &CPU::LSR, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 2 },{ "JMP", &CPU::JMP, &CPU::ABS, 3 },{ "EOR", &CPU::EOR, &CPU::ABS, 4 },{ "LSR", &CPU::LSR, &CPU::ABS, 6 },{ "???", &CPU::XXX, &CPU::IMP, 6 },
			{ "BVC", &CPU::BVC, &CPU::REL, 2 },{ "EOR", &CPU::EOR, &CPU::IZY, 5 },{ "???", &CPU::XXX, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 8 },{ "???", &CPU::NOP, &CPU::IMP, 4 },{ "EOR", &CPU::EOR, &CPU::ZPX, 4 },{ "LSR", &CPU::LSR, &CPU::ZPX, 6 },{ "???", &CPU::XXX, &CPU::IMP, 6 },{ "CLI", &CPU::CLI, &CPU::IMP, 2 },{ "EOR", &CPU::EOR, &CPU::ABY, 4 },{ "???", &CPU::NOP, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 7 },{ "???", &CPU::NOP, &CPU::IMP, 4 },{ "EOR", &CPU::EOR, &CPU::ABX, 4 },{ "LSR", &CPU::LSR, &CPU::ABX, 7 },{ "???", &CPU::XXX, &CPU::IMP, 7 },
			{ "RTS", &CPU::RTS, &CPU::IMP, 6 },{ "ADC", &CPU::ADC, &CPU::IZX, 6 },{ "???", &CPU::XXX, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 8 },{ "???", &CPU::NOP, &CPU::IMP, 3 },{ "ADC", &CPU::ADC, &CPU::ZP0, 3 },{ "ROR", &CPU::ROR, &CPU::ZP0, 5 },{ "???", &CPU::XXX, &CPU::IMP, 5 },{ "PLA", &CPU::PLA, &CPU::IMP, 4 },{ "ADC", &CPU::ADC, &CPU::IMM, 2 },{ "ROR", &CPU::ROR, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 2 },{ "JMP", &CPU::JMP, &CPU::IND, 5 },{ "ADC", &CPU::ADC, &CPU::ABS, 4 },{ "ROR", &CPU::ROR, &CPU::ABS, 6 },{ "???", &CPU::XXX, &CPU::IMP, 6 },
			{ "BVS", &CPU::BVS, &CPU::REL, 2 },{ "ADC", &CPU::ADC, &CPU::IZY, 5 },{ "???", &CPU::XXX, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 8 },{ "???", &CPU::NOP, &CPU::IMP, 4 },{ "ADC", &CPU::ADC, &CPU::ZPX, 4 },{ "ROR", &CPU::ROR, &CPU::ZPX, 6 },{ "???", &CPU::XXX, &CPU::IMP, 6 },{ "SEI", &CPU::SEI, &CPU::IMP, 2 },{ "ADC", &CPU::ADC, &CPU::ABY, 4 },{ "???", &CPU::NOP, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 7 },{ "???", &CPU::NOP, &CPU::IMP, 4 },{ "ADC", &CPU::ADC, &CPU::ABX, 4 },{ "ROR", &CPU::ROR, &CPU::ABX, 7 },{ "???", &CPU::XXX, &CPU::IMP, 7 },
			{ "???", &CPU::NOP, &CPU::IMP, 2 },{ "STA", &CPU::STA, &CPU::IZX, 6 },{ "???", &CPU::NOP, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 6 },{ "STY", &CPU::STY, &CPU::ZP0, 3 },{ "STA", &CPU::STA, &CPU::ZP0, 3 },{ "STX", &CPU::STX, &CPU::ZP0, 3 },{ "???", &CPU::XXX, &CPU::IMP, 3 },{ "DEY", &CPU::DEY, &CPU::IMP, 2 },{ "???", &CPU::NOP, &CPU::IMP, 2 },{ "TXA", &CPU::TXA, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 2 },{ "STY", &CPU::STY, &CPU::ABS, 4 },{ "STA", &CPU::STA, &CPU::ABS, 4 },{ "STX", &CPU::STX, &CPU::ABS, 4 },{ "???", &CPU::XXX, &CPU::IMP, 4 },
			{ "BCC", &CPU::BCC, &CPU::REL, 2 },{ "STA", &CPU::STA, &CPU::IZY, 6 },{ "???", &CPU::XXX, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 6 },{ "STY", &CPU::STY, &CPU::ZPX, 4 },{ "STA", &CPU::STA, &CPU::ZPX, 4 },{ "STX", &CPU::STX, &CPU::ZPY, 4 },{ "???", &CPU::XXX, &CPU::IMP, 4 },{ "TYA", &CPU::TYA, &CPU::IMP, 2 },{ "STA", &CPU::STA, &CPU::ABY, 5 },{ "TXS", &CPU::TXS, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 5 },{ "???", &CPU::NOP, &CPU::IMP, 5 },{ "STA", &CPU::STA, &CPU::ABX, 5 },{ "???", &CPU::XXX, &CPU::IMP, 5 },{ "???", &CPU::XXX, &CPU::IMP, 5 },
			{ "LDY", &CPU::LDY, &CPU::IMM, 2 },{ "LDA", &CPU::LDA, &CPU::IZX, 6 },{ "LDX", &CPU::LDX, &CPU::IMM, 2 },{ "???", &CPU::XXX, &CPU::IMP, 6 },{ "LDY", &CPU::LDY, &CPU::ZP0, 3 },{ "LDA", &CPU::LDA, &CPU::ZP0, 3 },{ "LDX", &CPU::LDX, &CPU::ZP0, 3 },{ "???", &CPU::XXX, &CPU::IMP, 3 },{ "TAY", &CPU::TAY, &CPU::IMP, 2 },{ "LDA", &CPU::LDA, &CPU::IMM, 2 },{ "TAX", &CPU::TAX, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 2 },{ "LDY", &CPU::LDY, &CPU::ABS, 4 },{ "LDA", &CPU::LDA, &CPU::ABS, 4 },{ "LDX", &CPU::LDX, &CPU::ABS, 4 },{ "???", &CPU::XXX, &CPU::IMP, 4 },
			{ "BCS", &CPU::BCS, &CPU::REL, 2 },{ "LDA", &CPU::LDA, &CPU::IZY, 5 },{ "???", &CPU::XXX, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 5 },{ "LDY", &CPU::LDY, &CPU::ZPX, 4 },{ "LDA", &CPU::LDA, &CPU::ZPX, 4 },{ "LDX", &CPU::LDX, &CPU::ZPY, 4 },{ "???", &CPU::XXX, &CPU::IMP, 4 },{ "CLV", &CPU::CLV, &CPU::IMP, 2 },{ "LDA", &CPU::LDA, &CPU::ABY, 4 },{ "TSX", &CPU::TSX, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 4 },{ "LDY", &CPU::LDY, &CPU::ABX, 4 },{ "LDA", &CPU::LDA, &CPU::ABX, 4 },{ "LDX", &CPU::LDX, &CPU::ABY, 4 },{ "???", &CPU::XXX, &CPU::IMP, 4 },
			{ "CPY", &CPU::CPY, &CPU::IMM, 2 },{ "CMP", &CPU::CMP, &CPU::IZX, 6 },{ "???", &CPU::NOP, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 8 },{ "CPY", &CPU::CPY, &CPU::ZP0, 3 },{ "CMP", &CPU::CMP, &CPU::ZP0, 3 },{ "DEC", &CPU::DEC, &CPU::ZP0, 5 },{ "???", &CPU::XXX, &CPU::IMP, 5 },{ "INY", &CPU::INY, &CPU::IMP, 2 },{ "CMP", &CPU::CMP, &CPU::IMM, 2 },{ "DEX", &CPU::DEX, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 2 },{ "CPY", &CPU::CPY, &CPU::ABS, 4 },{ "CMP", &CPU::CMP, &CPU::ABS, 4 },{ "DEC", &CPU::DEC, &CPU::ABS, 6 },{ "???", &CPU::XXX, &CPU::IMP, 6 },
			{ "BNE", &CPU::BNE, &CPU::REL, 2 },{ "CMP", &CPU::CMP, &CPU::IZY, 5 },{ "???", &CPU::XXX, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 8 },{ "???", &CPU::NOP, &CPU::IMP, 4 },{ "CMP", &CPU::CMP, &CPU::ZPX, 4 },{ "DEC", &CPU::DEC, &CPU::ZPX, 6 },{ "???", &CPU::XXX, &CPU::IMP, 6 },{ "CLD", &CPU::CLD, &CPU::IMP, 2 },{ "CMP", &CPU::CMP, &CPU::ABY, 4 },{ "NOP", &CPU::NOP, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 7 },{ "???", &CPU::NOP, &CPU::IMP, 4 },{ "CMP", &CPU::CMP, &CPU::ABX, 4 },{ "DEC", &CPU::DEC, &CPU::ABX, 7 },{ "???", &CPU::XXX, &CPU::IMP, 7 },
			{ "CPX", &CPU::CPX, &CPU::IMM, 2 },{ "SBC", &CPU::SBC, &CPU::IZX, 6 },{ "???", &CPU::NOP, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 8 },{ "CPX", &CPU::CPX, &CPU::ZP0, 3 },{ "SBC", &CPU::SBC, &CPU::ZP0, 3 },{ "INC", &CPU::INC, &CPU::ZP0, 5 },{ "???", &CPU::XXX, &CPU::IMP, 5 },{ "INX", &CPU::INX, &CPU::IMP, 2 },{ "SBC", &CPU::SBC, &CPU::IMM, 2 },{ "NOP", &CPU::NOP, &CPU::IMP, 2 },{ "SBC", &CPU::SBC, &CPU::IMM, 2 },{ "CPX", &CPU::CPX, &CPU::ABS, 4 },{ "SBC", &CPU::SBC, &CPU::ABS, 4 },{ "INC", &CPU::INC, &CPU::ABS, 6 },{ "???", &CPU::XXX, &CPU::IMP, 6 },
			{ "BEQ", &CPU::BEQ, &CPU::REL, 2 },{ "SBC", &CPU::SBC, &CPU::IZY, 5 },{ "???", &CPU::XXX, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 8 },{ "???", &CPU::NOP, &CPU::IMP, 4 },{ "SBC", &CPU::SBC, &CPU::ZPX, 4 },{ "INC", &CPU::INC, &CPU::ZPX, 6 },{ "???", &CPU::XXX, &CPU::IMP, 6 },{ "SED", &CPU::SED, &CPU::IMP, 2 },{ "SBC", &CPU::SBC, &CPU::ABY, 4 },{ "NOP", &CPU::NOP, &CPU::IMP, 2 },{ "???", &CPU::XXX, &CPU::IMP, 7 },{ "???", &CPU::NOP, &CPU::IMP, 4 },{ "SBC", &CPU::SBC, &CPU::ABX, 4 },{ "INC", &CPU::INC, &CPU::ABX, 7 },{ "???", &CPU::XXX, &CPU::IMP, 7 },
		};
	}

	void CPU::reset()
	{
		//Registers
		A = 0;
		X = 0;
		Y = 0;
		status = 0x00 | U; //Initially all zeros except unused bit 5 which is always 1
		SP = 0xFD;

		effectiveAddr = 0xFFFC;
		uint8_t lowByte = readData(effectiveAddr);
		uint8_t highByte = readData(effectiveAddr + 1);
		PC = (highByte << 8) | lowByte;

		fetched = 0x00;
		effectiveAddr = 0x0000;
		relativeAddr = 0x0000;

		setStatusFlag(I, 1);

		// Cycles it takes (some doubts about the cycles required)
		instructionCycles = 8;
	}

	void CPU::irq()
	{
		if (getStatusFlag(I) == 0)
		{
			writeData(base_stack + SP, (PC >> 8) & 0x00FF);
			SP--;

			writeData(base_stack + SP, PC & 0x00FF);
			SP--;

			setStatusFlag(B, 0);
			setStatusFlag(U, 1);
			writeData(base_stack + SP, status);
			SP--;

			setStatusFlag(I, 1); // Side effect

			effectiveAddr = 0xFFFE;
			uint8_t lo_byte = readData(effectiveAddr);
			uint8_t hi_byte = readData(effectiveAddr + 1);
			PC = (hi_byte << 8) | lo_byte;

			instructionCycles = 7;
		}
	}

	void CPU::nmi()
	{
		writeData(base_stack + SP, (PC >> 8) & 0x00FF);
		SP--;

		writeData(base_stack + SP, PC & 0x00FF);
		SP--;

		setStatusFlag(B, 0);
		setStatusFlag(U, 1);
		writeData(base_stack + SP, status);
		SP--;

		setStatusFlag(I, 1); // Side effect

		effectiveAddr = 0xFFFA;
		uint8_t lo_byte = readData(effectiveAddr);
		uint8_t hi_byte = readData(effectiveAddr + 1);
		PC = (hi_byte << 8) | lo_byte;

		instructionCycles = 7;
	}

	void CPU::clock()
	{
		if (instructionCycles == 0) // Last instruction ended, we can execute the next one
		{
#ifdef LOG_MODE
			uint16_t originPC = PC; //*
			uint16_t originA = A; //*
			uint16_t originX = X; //*
			uint16_t originY = Y; //*
			uint16_t originP = status; //*
			uint16_t originSP = SP; //*
#endif
			uint8_t opcode = currentOpcode = readData(PC);
			PC++;

			instructionCycles = instructionsTable[opcode].cyclesRequired;

			uint8_t possible_extra_cycle_1 = (this->*instructionsTable[opcode].addressMode)();

#ifdef LOG_MODE
			uint16_t abs_addr = effectiveAddr; //*
			std::string instName = instructionsTable[opcode].name; //*
			int numFollowingBytes = 0;

			if (instructionsTable[opcode].addressMode == &CPU::IMP)
				numFollowingBytes = 0;
			else if (instructionsTable[opcode].addressMode == &CPU::IMM
				|| instructionsTable[opcode].addressMode == &CPU::REL)
				numFollowingBytes = 1;
			else
				numFollowingBytes = 2;

			fopen_s(&logfile, "CPU6502.txt", "a");

			if (numFollowingBytes == 2)
			{
				uint8_t hi = effectiveAddr >> 8;
				uint8_t lo = effectiveAddr & 0x00FF;
				printf("%04X  %02X %02X %02X  %s\t\tA:%02X X:%02X Y:%02X P:%02X SP:%02X\n", originPC, opcode, lo, hi, instName.c_str(), originA, originX, originY, originP, originSP);
				if (logfile != nullptr)
					fprintf(logfile, "%04X  %02X %02X %02X  %s\t\tA:%02X X:%02X Y:%02X P:%02X SP:%02X\n", originPC, opcode, lo, hi, instName.c_str(), originA, originX, originY, originP, originSP);
			}
			else if (numFollowingBytes == 1)
			{
				printf("%04X  %02X %02X     %s\t\tA:%02X X:%02X Y:%02X P:%02X SP:%02X\n", originPC, opcode, readData((originPC + 1)), instName.c_str(), originA, originX, originY, originP, originSP);
				if (logfile != nullptr)
					fprintf(logfile, "%04X  %02X %02X     %s\t\tA:%02X X:%02X Y:%02X P:%02X SP:%02X\n", originPC, opcode, readData((originPC + 1)), instName.c_str(), originA, originX, originY, originP, originSP);
			}
			else
			{
				printf("%04X  %02X        %s\t\tA:%02X X:%02X Y:%02X P:%02X SP:%02X\n", originPC, opcode, instName.c_str(), originA, originX, originY, originP, originSP);
				if (logfile != nullptr)
					fprintf(logfile, "%04X  %02X        %s\t\tA:%02X X:%02X Y:%02X P:%02X SP:%02X\n", originPC, opcode, instName.c_str(), originA, originX, originY, originP, originSP);
			}

			if (logfile)
				fclose(logfile);
#endif		

			uint8_t possible_extra_cycle_2 = (this->*instructionsTable[opcode].instruction)();

			instructionCycles += (possible_extra_cycle_1 & possible_extra_cycle_2);

			/*if (originPC == 0xC6BC)
				auto test = originPC;*/
		}

		instructionCycles--;

		debugTotalCyclesElapsed++;
	}

	void CPU::loadState(const EmulatorStateInfo& emuStateInfo)
	{
		A = emuStateInfo.CPUStateInfo.regA;
		X = emuStateInfo.CPUStateInfo.regX;
		Y = emuStateInfo.CPUStateInfo.regY;
		PC = emuStateInfo.CPUStateInfo.PC;
		SP = emuStateInfo.CPUStateInfo.SP;
		status = emuStateInfo.CPUStateInfo.status;
		fetched = emuStateInfo.CPUStateInfo.fetched;
		effectiveAddr = emuStateInfo.CPUStateInfo.effectiveAddr;
		relativeAddr = emuStateInfo.CPUStateInfo.relativeAddr;
		currentOpcode = emuStateInfo.CPUStateInfo.currentOpcode;
		instructionCycles = emuStateInfo.CPUStateInfo.instructionCycles;
		debugTotalCyclesElapsed = emuStateInfo.CPUStateInfo.debugTotalCyclesElapsed;
	}

	void CPU::saveState(EmulatorStateInfo& emuStateInfo) const
	{
		emuStateInfo.CPUStateInfo.regA = A;
		emuStateInfo.CPUStateInfo.regX = X;
		emuStateInfo.CPUStateInfo.regY = Y;
		emuStateInfo.CPUStateInfo.PC = PC;
		emuStateInfo.CPUStateInfo.SP = SP;
		emuStateInfo.CPUStateInfo.status = status;
		emuStateInfo.CPUStateInfo.fetched = fetched;
		emuStateInfo.CPUStateInfo.effectiveAddr = effectiveAddr;
		emuStateInfo.CPUStateInfo.relativeAddr = relativeAddr;
		emuStateInfo.CPUStateInfo.currentOpcode = currentOpcode;
		emuStateInfo.CPUStateInfo.instructionCycles = instructionCycles;
		emuStateInfo.CPUStateInfo.debugTotalCyclesElapsed = debugTotalCyclesElapsed;
	}

	bool CPU::isLastCompleted() const
	{
		return instructionCycles == 0;
	}

	void CPU::writeData(uint16_t address, uint8_t data)
	{
		bus->cpuWrite(address, data);
	}

	uint8_t CPU::readData(uint16_t address)
	{
		return bus->cpuRead(address);
	}

	uint8_t CPU::getStatusFlag(StatusFlags flag)
	{
		return (status & flag) > 0;
	}

	void CPU::setStatusFlag(StatusFlags flag, bool isSet)
	{
		if (isSet)
			status |= flag;
		else
			status &= ~flag;
	}

	// ----------------------------
	// ----- ADDRESSING MODES -----
	// ----------------------------

	uint8_t CPU::IMP()
	{
		fetched = A;

		return 0;
	}

	uint8_t CPU::IMM()
	{
		effectiveAddr = PC;
		PC++;

		return 0;
	}

	uint8_t CPU::ZP0()
	{
		effectiveAddr = readData(PC);
		PC++;
		effectiveAddr &= 0x00FF;

		return 0;
	}

	uint8_t CPU::ZPX()
	{
		effectiveAddr = readData(PC) + X;
		PC++;
		effectiveAddr &= 0x00FF;

		return 0;
	}

	uint8_t CPU::ZPY()
	{
		effectiveAddr = readData(PC) + Y;
		PC++;
		effectiveAddr &= 0x00FF;
		return 0;
	}

	uint8_t CPU::REL()
	{
		relativeAddr = readData(PC);
		PC++;

		// When rel addr is 0x80 (1000 0000), it is 128 in unsigned representation, as relative increment goes from -128 to 127,
		// we need to wrap over 128 to be -128 (127 + 1 = -128), so we make previous bots to 0x80 all 1s to make it signed negative.
		if (relativeAddr & 0x80)
			relativeAddr |= 0xFF00;

		return 0;
	}

	uint8_t CPU::ABS()
	{
		uint8_t lowByte = readData(PC);
		PC++;
		uint8_t highByte = readData(PC);
		PC++;
		effectiveAddr = (highByte << 8) | lowByte;

		return 0;
	}

	uint8_t CPU::ABX()
	{
		uint8_t lowByte = readData(PC);
		PC++;
		uint8_t highByte = readData(PC);
		PC++;
		effectiveAddr = ((highByte << 8) | lowByte) + X;

		if ((effectiveAddr & 0xFF00) != highByte)
			return 1;
		else
			return 0;
	}

	uint8_t CPU::ABY()
	{
		uint8_t lowByte = readData(PC);
		PC++;
		uint8_t highByte = readData(PC);
		PC++;
		effectiveAddr = ((highByte << 8) | lowByte) + Y;

		if ((effectiveAddr & 0xFF00) != highByte)
			return 1;
		else
			return 0;
	}

	uint8_t CPU::IND()
	{
		uint8_t intermediate_lo = readData(PC);
		PC++;
		uint8_t intermediate_hi = readData(PC);
		PC++;
		uint16_t intermediate_addr = (intermediate_hi << 8) | intermediate_lo;

		if (intermediate_lo == 0x00FF)
		{
			uint8_t lowByte = readData(intermediate_addr);
			uint8_t highByte = readData(intermediate_addr & 0xFF00); // Wraps back around in the same page, hardware bug
			effectiveAddr = (highByte << 8) | lowByte;
		}
		else
		{
			uint8_t lowByte = readData(intermediate_addr);
			uint8_t highByte = readData(intermediate_addr + 1);
			effectiveAddr = (highByte << 8) | lowByte;
		}

		return 0;
	}

	uint8_t CPU::IZX()
	{
		uint16_t prevZPAddr = readData(PC) & 0x00FF;
		PC++;
		uint16_t postZPAddr = prevZPAddr + X;
		postZPAddr &= 0x00FF; // The result must be always a zero page address

		uint8_t lowByte = readData(postZPAddr & 0x00FF); // We ANDed it to wrap around ZP
		uint8_t highByte = readData((postZPAddr + 1) & 0x00FF);
		effectiveAddr = (highByte << 8) | lowByte;

		return 0;
	}

	uint8_t CPU::IZY()
	{
		uint16_t ptr_zp = readData(PC);
		PC++;
		uint16_t intermediate_lo = readData(ptr_zp & 0x00FF);
		uint16_t intermediate_hi = readData((ptr_zp + 1) & 0x00FF); // Wrap around ZP
		uint16_t intermediate_addr = (intermediate_hi << 8) | intermediate_lo;
		bool crossPage = ((intermediate_addr + Y) & 0xFF00) != (intermediate_addr & 0xFF00);
		effectiveAddr = intermediate_addr + Y;

		if (crossPage)
			return 1;
		else
			return 0;
	}

	// ------------------------
	// ----- INSTRUCTIONS -----
	// ------------------------

	uint8_t CPU::ADC()
	{
		fetch();

		uint16_t result = (uint16_t)A + (uint16_t)fetched + (uint16_t)getStatusFlag(C);
		setStatusFlag(C, result > 255);
		setStatusFlag(Z, (result & 0x00FF) == 0); // Or (result & 0x00FF) == 0
		setStatusFlag(V, (~((uint16_t)A ^ (uint16_t)fetched) & ((uint16_t)A ^ (uint16_t)result)) & 0x0080); // I think cast to uint16_t is not needed. Also AND with 0x0080 is to get msb of low byte
		setStatusFlag(N, result & 0x80); // Or result & 0x0080 (0x80) 
		A = result & 0x00FF;

		return 1;
	}

	uint8_t CPU::AND()
	{
		fetch();

		A &= fetched;
		setStatusFlag(Z, A == 0);
		setStatusFlag(N, A & 0x80);

		return 1;
	}

	uint8_t CPU::ASL()
	{
		fetch();

		setStatusFlag(C, fetched & 0x80);
		uint8_t result = fetched << 1;
		setStatusFlag(Z, result == 0);
		setStatusFlag(N, result & 0x80);

		if (instructionsTable[currentOpcode].addressMode == &CPU::IMP)
			A = result;
		else
			writeData(effectiveAddr, result);

		return 0;
	}

	uint8_t CPU::BCC()
	{
		if (getStatusFlag(C) == 0)
		{
			instructionCycles++;

			effectiveAddr = PC + relativeAddr;

			if ((effectiveAddr & 0xFF00) != (PC & 0xFF00))
				instructionCycles++;

			PC = effectiveAddr;
		}

		return 0;
	}

	uint8_t CPU::BCS()
	{
		if (getStatusFlag(C) == 1)
		{
			instructionCycles++;

			effectiveAddr = PC + relativeAddr;

			if ((effectiveAddr & 0xFF00) != (PC & 0xFF00))
				instructionCycles++;

			PC = effectiveAddr;
		}

		return 0;
	}

	uint8_t CPU::BEQ()
	{
		if (getStatusFlag(Z) == 1)
		{
			instructionCycles++;

			effectiveAddr = PC + relativeAddr;

			if ((effectiveAddr & 0xFF00) != (PC & 0xFF00))
				instructionCycles++;

			PC = effectiveAddr;
		}

		return 0;
	}

	uint8_t CPU::BIT()
	{
		fetch();

		setStatusFlag(Z, (A & fetched) == 0);
		setStatusFlag(V, fetched & 0x40); // bit 6
		setStatusFlag(N, fetched & 0x80); // bit 7

		return 0;
	}

	uint8_t CPU::BMI()
	{
		if (getStatusFlag(N) == 1)
		{
			instructionCycles++;

			effectiveAddr = PC + relativeAddr;

			if ((effectiveAddr & 0xFF00) != (PC & 0xFF00))
				instructionCycles++;

			PC = effectiveAddr;
		}

		return 0;
	}

	uint8_t CPU::BNE()
	{
		if (getStatusFlag(Z) == 0)
		{
			instructionCycles++;

			effectiveAddr = PC + relativeAddr;

			if ((effectiveAddr & 0xFF00) != (PC & 0xFF00))
				instructionCycles++;

			PC = effectiveAddr;
		}

		return 0;
	}

	uint8_t CPU::BPL()
	{
		if (getStatusFlag(N) == 0)
		{
			instructionCycles++;

			effectiveAddr = PC + relativeAddr;

			if ((effectiveAddr & 0xFF00) != (PC & 0xFF00))
				instructionCycles++;

			PC = effectiveAddr;
		}

		return 0;
	}

	uint8_t CPU::BRK()
	{

		PC++; // Last executing instruction it's ensured to be finished before break happens

		writeData(base_stack + SP, (PC >> 8) & 0x00FF); // First we ensure 0s padding and we push hi byte 
		SP--;
		writeData(base_stack + SP, PC & 0x00FF);
		SP--;

		//These are always set by BRK
		setStatusFlag(B, 1);
		setStatusFlag(U, 1);
		writeData(base_stack + SP, status);
		SP--;

		setStatusFlag(I, 1); // Side effect

		PC = (readData(0xFFFF) << 8) | readData(0xFFFE);

		return 0;
	}

	uint8_t CPU::BVC()
	{
		if (getStatusFlag(V) == 0)
		{
			instructionCycles++;

			effectiveAddr = PC + relativeAddr;

			if ((effectiveAddr & 0xFF00) != (PC & 0xFF00))
				instructionCycles++;

			PC = effectiveAddr;
		}

		return 0;
	}

	uint8_t CPU::BVS()
	{
		if (getStatusFlag(V) == 1)
		{
			instructionCycles++;

			effectiveAddr = PC + relativeAddr;

			if ((effectiveAddr & 0xFF00) != (PC & 0xFF00))
				instructionCycles++;

			PC = effectiveAddr;
		}

		return 0;
	}

	uint8_t CPU::CLC()
	{
		setStatusFlag(C, 0);

		return 0;
	}

	uint8_t CPU::CLD()
	{
		setStatusFlag(D, 0);

		return 0;
	}

	uint8_t CPU::CLI()
	{
		setStatusFlag(I, 0);

		return 0;
	}

	uint8_t CPU::CLV()
	{
		setStatusFlag(V, 0);

		return 0;
	}

	uint8_t CPU::CMP()
	{
		fetch();

		setStatusFlag(C, A >= fetched);
		setStatusFlag(Z, A == fetched);
		setStatusFlag(N, (A - fetched) & 0x80);

		return 1;
	}

	uint8_t CPU::CPX()
	{
		fetch();

		setStatusFlag(C, X >= fetched);
		setStatusFlag(Z, X == fetched);
		setStatusFlag(N, (X - fetched) & 0x80);

		return 0;
	}

	uint8_t CPU::CPY()
	{
		fetch();

		setStatusFlag(C, Y >= fetched);
		setStatusFlag(Z, Y == fetched);
		setStatusFlag(N, (Y - fetched) & 0x80);

		return 0;
	}

	uint8_t CPU::DEC()
	{
		fetch();

		uint8_t result = fetched - 1;

		setStatusFlag(Z, result == 0);
		setStatusFlag(N, result & 0x80);

		writeData(effectiveAddr, result);

		return 0;
	}

	uint8_t CPU::DEX()
	{
		fetch();

		X -= 1;

		setStatusFlag(Z, X == 0);
		setStatusFlag(N, X & 0x80);

		return 0;
	}

	uint8_t CPU::DEY()
	{
		fetch();

		Y -= 1;

		setStatusFlag(Z, Y == 0);
		setStatusFlag(N, Y & 0x80);

		return 0;
	}

	uint8_t CPU::EOR()
	{
		fetch();

		A ^= fetched;

		setStatusFlag(Z, A == 0);
		setStatusFlag(N, A & 0x80);

		return 1;
	}

	uint8_t CPU::INC()
	{
		fetch();

		uint8_t result = fetched + 1;

		setStatusFlag(Z, result == 0);
		setStatusFlag(N, result & 0x80);

		writeData(effectiveAddr, result);

		return 0;
	}

	uint8_t CPU::INX()
	{
		fetch();

		X += 1;

		setStatusFlag(Z, X == 0);
		setStatusFlag(N, X & 0x80);

		return 0;
	}

	uint8_t CPU::INY()
	{
		fetch();

		Y += 1;

		setStatusFlag(Z, Y == 0);
		setStatusFlag(N, Y & 0x80);

		return 0;
	}

	uint8_t CPU::JMP()
	{
		PC = effectiveAddr;

		return 0;
	}

	uint8_t CPU::JSR()
	{
		PC--;

		writeData(base_stack + SP, (PC >> 8) & 0x00FF); // First we ensure 0s padding and we push hi byte 
		SP--;
		writeData(base_stack + SP, PC & 0x00FF);
		SP--;

		PC = effectiveAddr;

		return 0;
	}

	uint8_t CPU::LDA()
	{
		fetch();

		A = fetched;

		setStatusFlag(Z, A == 0);
		setStatusFlag(N, A & 0x80);

		return 1;
	}

	uint8_t CPU::LDX()
	{
		fetch();

		X = fetched;

		setStatusFlag(Z, X == 0);
		setStatusFlag(N, X & 0x80);

		return 1;
	}

	uint8_t CPU::LDY()
	{
		fetch();

		Y = fetched;

		setStatusFlag(Z, Y == 0);
		setStatusFlag(N, Y & 0x80);

		return 1;
	}

	uint8_t CPU::LSR()
	{
		fetch();

		setStatusFlag(C, fetched & 0x01);
		uint8_t result = fetched >> 1;
		setStatusFlag(Z, result == 0);
		setStatusFlag(N, result & 0x80);

		if (instructionsTable[currentOpcode].addressMode == &CPU::IMP)
			A = result;
		else
			writeData(effectiveAddr, result);

		return 0;
	}

	uint8_t CPU::NOP()
	{
		switch (currentOpcode)
		{
		case 0x1C:
			PC++;
			PC++;
			return 1;
		case 0x3C:
			PC++;
			PC++;
			return 1;
		case 0x5C:
			PC++;
			PC++;
			return 1;
		case 0x7C:
			PC++;
			PC++;
			return 1;
		case 0xDC:
			PC++;
			PC++;
			return 1;
		case 0xFC:
			PC++;
			PC++;
			return 1;
		case 0x04:
			PC++;
			return 0;
		case 0x44:
			PC++;
			return 0;
		case 0x64:
			PC++;
			return 0;
		case 0x0C:
			PC++;
			PC++;
			return 1;
		case 0x14:
			PC++;
			return 0;
		case 0x34:
			PC++;
			return 0;
		case 0x54:
			PC++;
			return 0;
		case 0x74:
			PC++;
			return 0;
		case 0xD4:
			PC++;
			return 0;
		case 0xF4:
			PC++;
			return 0;
		case 0x80:
			PC++;
			return 0;
		case 0x1A:
			return 0;
		case 0x3A:
			return 0;
		case 0x5A:
			return 0;
		case 0x7A:
			return 0;
		case 0xDA:
			return 0;
		case 0xFA:
			return 0;
		case 0xEA: // Regular NOP
			return 0;
		default:
			//assert(false);
			/*while (true);*/
			return 0;
		}
	}

	uint8_t CPU::ORA()
	{
		fetch();

		A |= fetched;

		setStatusFlag(Z, A == 0);
		setStatusFlag(N, A & 0x80);

		return 1;
	}

	uint8_t CPU::PHA()
	{
		writeData(base_stack + SP, A);
		SP--;

		return 0;
	}

	uint8_t CPU::PHP()
	{
		// These are always set by PHP
		setStatusFlag(B, 1);
		setStatusFlag(U, 1);
		writeData(base_stack + SP, status);
		SP--;

		//Cleared after the push to restore it
		//setStatusFlag(B, 0);
		//setStatusFlag(U, 0);

		return 0;
	}

	uint8_t CPU::PLA()
	{
		SP++;
		A = readData(base_stack + SP);

		setStatusFlag(Z, A == 0);
		setStatusFlag(N, A & 0x80);

		// Ensure U is always 1
		setStatusFlag(U, 1);

		return 0;
	}

	uint8_t CPU::PLP()
	{
		SP++;
		status = readData(base_stack + SP);
		/*status &= ~0x30;*/
		//status &= ~B;
		setStatusFlag(U, 1); // Ensure it's 1 (bit 5 is wired to 1 in the hardware)

		return 0;
	}

	uint8_t CPU::ROL()
	{
		fetch();

		uint8_t cachedFlagC = (fetched & 0x80) >> 7;
		uint8_t result = (fetched << 1) | getStatusFlag(C);
		setStatusFlag(C, cachedFlagC);
		setStatusFlag(N, result & 0x80);

		if (instructionsTable[currentOpcode].addressMode == &CPU::IMP)
			A = result;
		else
			writeData(effectiveAddr, result);

		setStatusFlag(Z, A == 0);

		return 0;
	}

	uint8_t CPU::ROR()
	{
		fetch();

		uint8_t cachedFlagC = fetched & 0x01;
		uint8_t result = (fetched >> 1) | (getStatusFlag(C) << 7);
		setStatusFlag(C, cachedFlagC);
		setStatusFlag(N, result & 0x80);

		if (instructionsTable[currentOpcode].addressMode == &CPU::IMP)
			A = result;
		else
			writeData(effectiveAddr, result);

		setStatusFlag(Z, A == 0);

		return 0;
	}

	uint8_t CPU::RTI()
	{
		SP++;
		status = readData(base_stack + SP);
		/*status &= ~0x30;*/
		//Ignored
		//status &= ~B;
		//setStatusFlag(B, 1);
		setStatusFlag(U, 1); // Ensure it's 1 (bit 5 is wired to 1 in the hardware)
		//status &= ~U;

		SP++;
		uint8_t lowByte = readData(base_stack + SP);
		SP++;
		uint8_t highByte = readData(base_stack + SP);

		PC = (highByte << 8) | lowByte;

		return 0;
	}

	uint8_t CPU::RTS()
	{
		SP++;
		uint8_t lowByte = readData(base_stack + SP);
		SP++;
		uint8_t highByte = readData(base_stack + SP);

		PC = (highByte << 8) | lowByte;

		PC++;

		return 0;
	}

	uint8_t CPU::SBC()
	{
		fetch();

		uint16_t memVal = ((uint16_t)fetched) ^ 0x00FF;

		uint16_t result = (uint16_t)A + memVal + (uint16_t)getStatusFlag(C);
		setStatusFlag(C, result & 0xFF00); // Same as checking if > 255
		setStatusFlag(Z, (result & 0x00FF) == 0); // Or (result & 0x00FF) == 0
		setStatusFlag(V, (result ^ (uint16_t)A) & (result ^ memVal) & 0x0080); // I think cast to uint16_t is not needed. Also AND with 0x0080 is to get msb of low byte
		setStatusFlag(N, result & 0x0080); // Or result & 0x0080 (0x80) 
		A = result & 0x00FF;

		return 1;
	}

	uint8_t CPU::SEC()
	{
		setStatusFlag(C, 1);

		return 0;
	}

	uint8_t CPU::SED()
	{
		setStatusFlag(D, 1);

		return 0;
	}

	uint8_t CPU::SEI()
	{
		setStatusFlag(I, 1);

		return 0;
	}

	uint8_t CPU::STA()
	{
		writeData(effectiveAddr, A);

		return 0;
	}

	uint8_t CPU::STX()
	{
		writeData(effectiveAddr, X);

		return 0;
	}

	uint8_t CPU::STY()
	{
		writeData(effectiveAddr, Y);

		return 0;
	}

	uint8_t CPU::TAX()
	{
		X = A;

		setStatusFlag(Z, X == 0);
		setStatusFlag(N, X & 0x80);

		return 0;
	}

	uint8_t CPU::TAY()
	{
		Y = A;

		setStatusFlag(Z, Y == 0);
		setStatusFlag(N, Y & 0x80);

		return 0;
	}

	uint8_t CPU::TSX()
	{
		X = SP;

		setStatusFlag(Z, X == 0);
		setStatusFlag(N, X & 0x80);

		return 0;
	}

	uint8_t CPU::TXA()
	{
		A = X;

		setStatusFlag(Z, A == 0);
		setStatusFlag(N, A & 0x80);

		return 0;
	}

	uint8_t CPU::TXS()
	{
		SP = X;

		return 0;
	}

	uint8_t CPU::TYA()
	{
		A = Y;

		setStatusFlag(Z, A == 0);
		setStatusFlag(N, A & 0x80);

		return 0;
	}

	uint8_t CPU::XXX()
	{
		return 0;
	}

	uint8_t CPU::fetch()
	{
		if (instructionsTable[currentOpcode].addressMode != &CPU::IMP)
			fetched = readData(effectiveAddr);

		return fetched;
	}
}