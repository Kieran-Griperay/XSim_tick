#include <sst/core/sst_config.h>
#include <sst/core/interfaces/stdMem.h>

// #include <sst/core/simulation.h>

#include <sstream>
#include <cstdlib>
#include <fstream>
#include <cmath>
#include <iostream>
#include <vector>
#include <array>
#include <xsim_core/core.hpp>

#include <json/json.h>

namespace XSim
{
namespace Core
{
	



Core::Core(ComponentId_t id, Params& params):
	Component(id)
{

	// verbose is one of the configuration options for this component
	verbose = params.find<uint32_t>("verbose", verbose);

	// clock_frequency is one of the configuration options for this component
	clock_frequency=params.find<std::string>("clock",clock_frequency);
	this->registerTimeBase(clock_frequency, true );

	// set instruction latencies
	load_latencies(params);

	// load the program that is to be executed
	load_program(params);

	// Create the SST output with the required verbosity level
	output = new Output("mips_core[@t:@l]: ", verbose, 0, Output::STDOUT);

	// Create a tick function with the frequency specified
	tc = Super::registerClock( clock_frequency, new Clock::Handler2<Core, &Core::tick>(this) );

	output->verbose(CALL_INFO, 1, 0, "Configuring connection to memory...\n");
	// memory_wrapper is used to make write/read requests to the SST simulated memory
	memory_wrapper = loadComponentExtension<MemoryWrapper>(params, output, tc);
	// new MemoryWrapper(*this, params, output, tc);
	output->verbose(CALL_INFO, 1, 0, "Configuration of memory interface completed.\n");

	// SST statistics
	instruction_count = registerStatistic<uint64_t>( "instructions" );

	// tell the simulator not to end without us
	registerAsPrimaryComponent();
	primaryComponentDoNotEndSim();
}

void Core::init(unsigned int phase)
{
	memory_wrapper->init(phase);
}

void Core::setup()
{
	output->output("Setting up.\n");
	std::cout<<"========== STARTED PROGRAM =========="<<std::endl;

}

#include <fstream>
#include <json/json.h>
void Core::finish()
{
	Json::Value root;

	Json::Value integer;
	Json::Value multiplier;
	Json::Value divider;
	Json::Value ls;

    root["reg reads"] = reg_reads;
    
    for (auto const& [op, name] : names) {
        stats[name] = (Json::UInt64)instruction_counts[op]; 
    }
    
    stats["instructions"] = (Json::UInt64)total_instructions;
    stats["cycles"] = (Json::UInt64)total_cycles;
    
    root["stats"] = stats;
	std::string output_file_path = "statistics.json";
    std::ofstream out_file(output_file_path);
    if (out_file.is_open()) {
        out_file << root; 
        out_file.close();
        std::cout << "Successfully saved stats to " << output_file_path << std::endl;
    } else {
        std::cerr << "Failed to open output file: " << output_file_path << std::endl;
    }
	// Nothing to cleanup
	std::cout<<"========== FINISHED PROGRAM =========="<<std::endl;
}


/**
 * @brief This function loads the program from the file in parameter: "program"
 */
void Core::load_program(Params &params)
{
	std::string program_path = params.find<std::string>("program","");
	std::ifstream f(program_path);
	if(!f.is_open())
	{
		std::cerr<<"Invalid program file: "<<program_path<<std::endl;
		exit(EXIT_FAILURE);
	}
	for( std::string line; getline( f, line ); )
	{
		std::size_t first_comment=line.find("#");
		std::size_t first_number = line.find_first_of("0123456789ABCDEFabcdef");
		// Ignore comments
		if( first_number<first_comment)
		{
			std::stringstream ss;
			ss << std::hex << line.substr(first_number,4);
			uint16_t instruction;
			ss >> instruction;
			program.push_back(instruction);
		}
	}
	// Print the program for visual inspection
	std::cout<<"Program:"<<std::endl;
	for(uint16_t instruction: program)
	{
		std::cout<<std::hex<<instruction<<std::dec<<std::endl;
	}
}

/**
 * @brief This function loads the latencies from the parameters
 */
void Core::load_latencies(Params &params)
{
	// Map opcodes to instruction names and to respective functional unit latencies
    latencies.insert({ADD, params.find<uint32_t>("integer.latency", 1)});
    names.insert({ADD, "add"});
    
    latencies.insert({SUB, params.find<uint32_t>("integer.latency", 1)});
    names.insert({SUB, "sub"});
    
    latencies.insert({AND, params.find<uint32_t>("integer.latency", 1)});
    names.insert({AND, "and"});
    
    latencies.insert({NOR, params.find<uint32_t>("integer.latency", 1)});
    names.insert({NOR, "nor"});
    
    latencies.insert({DIV, params.find<uint32_t>("divider.latency", 1)});
    names.insert({DIV, "div"});
    
    latencies.insert({MUL, params.find<uint32_t>("multiplier.latency", 1)});
    names.insert({MUL, "mul"});
    
    latencies.insert({MOD, params.find<uint32_t>("divider.latency", 1)});
    names.insert({MOD, "mod"});
    
    latencies.insert({EXP, params.find<uint32_t>("divider.latency", 1)});
    names.insert({EXP, "exp"});

    latencies.insert({LW, params.find<uint32_t>("ls.latency", 1)});
    names.insert({LW, "lw"});
    
    latencies.insert({SW, params.find<uint32_t>("ls.latency", 1)});
    names.insert({SW, "sw"});

    latencies.insert({LIZ, params.find<uint32_t>("integer.latency", 1)});
    names.insert({LIZ, "liz"});
    
    latencies.insert({LIS, params.find<uint32_t>("integer.latency", 1)});
    names.insert({LIS, "lis"});
    
    latencies.insert({LUI, params.find<uint32_t>("integer.latency", 1)});
    names.insert({LUI, "lui"});

    latencies.insert({HALT, params.find<uint32_t>("integer.latency", 1)});
    names.insert({HALT, "halt"});
    
    latencies.insert({PUT, params.find<uint32_t>("integer.latency", 1)});
    names.insert({PUT, "put"});

    // latencies.insert({BP, params.find<uint32_t>("bp", 1)});
    // names.insert({BP, "bp"});
    
    // latencies.insert({BN, params.find<uint32_t>("bn", 1)});
    // names.insert({BN, "bn"});
    
    // latencies.insert({BX, params.find<uint32_t>("bx", 1)});
    // names.insert({BX, "bx"});
    
    // latencies.insert({BZ, params.find<uint32_t>("bz", 1)});
    // names.insert({BZ, "bz"});

    // latencies.insert({JR, params.find<uint32_t>("jr", 1)});
    // names.insert({JR, "jr"});
    
    // latencies.insert({JALR, params.find<uint32_t>("jalr", 1)});
    // names.insert({JALR, "jalr"});
    
    // latencies.insert({J, params.find<uint32_t>("j", 1)});
    // names.insert({J, "j"});
}

bool Core::tick(Cycle_t cycle)
{
	//std::cout<<"tick"<<std::endl;
	if(!busy)
	{
		fetch_instruction();
		busy = true;
	}
	// Block waiting for memory!
	if(!waiting_memory)
	{
		if(latency_countdown==0)
		{
			execute_instruction();
			if(instruction_count)
			{
				instruction_count->addData(1);
			}
		}
		else
		{
			latency_countdown--;
		}
	}
	return terminate;
}

void Core::fetch_instruction()
{
	// Better be aligned!!
	uint16_t instruction = program[pc/2];
	uint16_t opcode = instruction >> 11;
	std::cout<<"Fetched "<<names[opcode]<<std::endl;
	try
	{
		latency_countdown = latencies.at(opcode)-1;  //This cycle counts as 1
	}
	catch(std::out_of_range &e)
	{
		std::cerr<<"Unknown instruction: opcode "<<opcode<<std::endl;
		exit(EXIT_FAILURE);
	}
}
// Return in order:  rd, rs, rt format
std::array<uint16_t, 3> populateRInstructionRegisters(uint16_t instruction){
    //std::cout << instruction << std::endl;
    std::array<uint16_t, 3> r_registers;
    
    r_registers[0] = (uint16_t)((instruction >> 8) & 0x07); // rd 
    r_registers[1] = (uint16_t)((instruction >> 5) & 0x07); // rs 
    r_registers[2] = (uint16_t)((instruction >> 2) & 0x07); // rt
    
    
    return r_registers;
}
void incrementRegisterReads(int count){
	reg_reads += count;
}
void Core::execute_instruction()
{
	// Better be aligned!!
	uint16_t instruction = program[pc/2];
	uint16_t opcode = instruction >> 11;
	instruction_counts[opcode]++;
    total_instructions++;
    total_cycles += latencies[opcode];
	uint16_t rd,rs,rt,imm8;

	std::cout<<"Running "<<names[opcode]<<std::endl;
	switch (opcode)
	{
		case ADD:
		{
			std::array<uint16_t, 3> r_registers = populateRInstructionRegisters(instruction);
			uint16_t rd = r_registers[0]; 
			uint16_t rs = r_registers[1]; 
			uint16_t rt = r_registers[2]; 

			registers[rd] = registers[rs] + registers[rt];
			incrementRegisterReads(2);

			pc += 2;
			busy = false; 		
			break;
		}
		case SUB:
		{
			std::array<uint16_t, 3> r_registers = populateRInstructionRegisters(instruction);
			uint16_t rd = r_registers[0]; 
			uint16_t rs = r_registers[1]; 
			uint16_t rt = r_registers[2]; 

			registers[rd] = registers[rs] - registers[rt];
			incrementRegisterReads(2);

			pc += 2;
			busy = false; 
			break;
		}
		case AND:
		{
			std::array<uint16_t, 3> r_registers = populateRInstructionRegisters(instruction);
			uint16_t rd = r_registers[0]; 
			uint16_t rs = r_registers[1]; 
			uint16_t rt = r_registers[2]; 

			registers[rd] = registers[rs] & registers[rt];
			incrementRegisterReads(2);
			pc += 2;
			busy = false; 
			
			break;
		}
		case NOR:
		{
			std::array<uint16_t, 3> r_registers = populateRInstructionRegisters(instruction);
			uint16_t rd = r_registers[0]; 
			uint16_t rs = r_registers[1]; 
			uint16_t rt = r_registers[2]; 

			registers[rd] = !(registers[rs] | registers[rt]);
			incrementRegisterReads(2);
			
			pc += 2;
			busy = false; 
			
			break;
		}
		case DIV:
		{
			std::array<uint16_t, 3> r_registers = populateRInstructionRegisters(instruction);
			uint16_t rd = r_registers[0]; 
			uint16_t rs = r_registers[1]; 
			uint16_t rt = r_registers[2]; 

			registers[rd] = registers[rs] / registers[rt];
			incrementRegisterReads(2);


			pc += 2;
			busy = false; 
			
			break;
		}
		case MUL:
		{
			std::array<uint16_t, 3> r_registers = populateRInstructionRegisters(instruction);
			uint16_t rd = r_registers[0]; 
			uint16_t rs = r_registers[1]; 
			uint16_t rt = r_registers[2]; 

			registers[rd] = (uint16_t) ((registers[rs] * registers[rt]) & 0xFF);
			incrementRegisterReads(2);


			pc += 2;
			busy = false; 
			
			break;
		}
		case MOD:
		{
			std::array<uint16_t, 3> r_registers = populateRInstructionRegisters(instruction);
			uint16_t rd = r_registers[0]; 
			uint16_t rs = r_registers[1]; 
			uint16_t rt = r_registers[2]; 

			registers[rd] = registers[rs] % registers[rt];
			incrementRegisterReads(2);

			pc += 2;
			busy = false; 
			
			break;
		}
		case EXP:
		{
			std::array<uint16_t, 3> r_registers = populateRInstructionRegisters(instruction);
			uint16_t rd = r_registers[0]; 
			uint16_t rs = r_registers[1]; 
			uint16_t rt = r_registers[2]; 

			registers[rd] = (uint16_t)std::pow(registers[rs], registers[rt]);
			incrementRegisterReads(2);

			pc += 2;
			busy = false; 
			
			break;
		}
		
		case LW:
			rs = (instruction >> 5) & 0x07;
			rd = (instruction >> 8) & 0x07;
			waiting_memory = true;
			memory_wrapper->read(registers[rs], [this, rd](uint16_t addr, uint16_t data)
			{
				registers[rd]=data;
				pc+=2;
				busy = false;
				waiting_memory = false;
			});
			incrementRegisterReads(1);

			break;
		case SW:
			rs = (instruction >> 5) & 0x07;
			rt = (instruction >> 2) & 0x07;
			waiting_memory = true;
			memory_wrapper->write(registers[rs], registers[rt], [this](uint16_t addr)
			{
				pc+=2;
				busy = false;
				waiting_memory = false;
			});
			incrementRegisterReads(1);
			break;
		case LIZ:
			rd = (instruction >> 8) & 0x07;
			imm8 = instruction & 0xFF;
			registers[rd]=imm8;
			pc+=2;
			busy = false;
			break;
		case LIS: 
            rd = (instruction >> 8) & 0x07;
            imm8 = instruction & 0xFF;
            registers[rd] = (uint16_t)((int16_t)((int8_t)imm8)); 
            pc += 2;
            busy = false;
            break;
        case LUI: 
            rd = (instruction >> 8) & 0x07;
            imm8 = instruction & 0xFF;
            registers[rd] = (imm8 << 8) | (registers[rd] & 0xFF);
			incrementRegisterReads(1);
            pc += 2;
            busy = false;
            break;
		case BP: 
            rd = (instruction >> 8) & 0x07;
            rs = (instruction >> 5) & 0x07;
            imm8 = instruction & 0xFF;
            if ((int16_t)registers[rs] > 0) pc = (imm8 << 1); else pc += 2;
			incrementRegisterReads(1);

            busy = false;
            break;
        case BN: 
            rd = (instruction >> 8) & 0x07;
            rs = (instruction >> 5) & 0x07;
            imm8 = instruction & 0xFF;
            if ((int16_t)registers[rs] < 0) pc = (imm8 << 1); else pc += 2;
			incrementRegisterReads(1);

            busy = false;
            break;
        case BX: 
            rd = (instruction >> 8) & 0x07;
            rs = (instruction >> 5) & 0x07;
            imm8 = instruction & 0xFF;
            if (registers[rs] != 0) pc = (imm8 << 1); else pc += 2;
			incrementRegisterReads(1);

            busy = false;
            break;
        case BZ: 
            rd = (instruction >> 8) & 0x07;
            rs = (instruction >> 5) & 0x07;
            imm8 = instruction & 0xFF;
            if (registers[rs] == 0) pc = (imm8 << 1); else pc += 2;
			incrementRegisterReads(1);

            busy = false;
            break;
		case JR: 
            rs = (instruction >> 5) & 0x07;
            pc = registers[rs] & 0xFFFE;
			incrementRegisterReads(1);

            busy = false;
            break;
        case JALR: 
            rd = (instruction >> 8) & 0x07;
            rs = (instruction >> 5) & 0x07;
            registers[rd] = pc + 2;
            pc = registers[rs] & 0xFFFE;
			incrementRegisterReads(1);

            busy = false;
            break;
        case J: 
        {
            uint16_t imm11 = instruction & 0x07FF; 
            pc = (pc & 0xF000) | (imm11 << 1); 
            busy = false;
            break;
        }
		case PUT:
			rs = (instruction >> 5) & 0x07;
			std::cout<<"Register "<<(int)rs<<" = "<<registers[rs]<<"(unsigned) = "<<(int16_t)registers[rs]<<"(signed)"<<std::endl;
			incrementRegisterReads(1);
			
			pc+=2;
			busy = false;
			break;
		case HALT:
			pc+=2;
			primaryComponentOKToEndSim();
			// unregisterExit();
			busy = false;
			break;
	}
	if(opcode == HALT)
	{
		terminate = true;
	}
}


}
}
