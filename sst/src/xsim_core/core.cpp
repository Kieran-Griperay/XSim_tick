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

FUType Core::get_fu_type(uint16_t opcode) {
    switch (opcode) {
        case ADD: case SUB: case AND: case NOR:
        case LIZ: case LIS: case LUI: case PUT: case HALT:
            return FU_INTEGER;
        case DIV: case EXP: case MOD:
            return FU_DIVIDER;
        case MUL:
            return FU_MULTIPLIER;
        case LW: case SW:
            return FU_LS;
        default:
            return FU_INTEGER; // Default to integer for unknown opcodes
    }
}
void Core::init_tomasulo(Params &params) {
    uint32_t counts[] = {
        params.find<uint32_t>("integer.resnumber", 4),
        params.find<uint32_t>("divider.resnumber", 2),
        params.find<uint32_t>("multiplier.resnumber", 4),
        params.find<uint32_t>("ls.resnumber", 8)
    };

    // Build RS array with type ranges
    int offset = 0;
    for (int type = 0; type < FU_TYPE_COUNT; type++) {
        rs_type_start[type] = offset;
        rs_type_count[type] = counts[type];
        for (uint32_t i = 0; i < counts[type]; i++) {
            ReservationStation rs;
            rs.fu_type = (FUType)type;
            rs_all.push_back(rs);
        }
        offset += counts[type];
    }

    // FU pools
    fu_pools[FU_INTEGER].resize(params.find<uint32_t>("integer.number", 2));
    fu_pools[FU_DIVIDER].resize(params.find<uint32_t>("divider.number", 1));
    fu_pools[FU_MULTIPLIER].resize(params.find<uint32_t>("multiplier.number", 2));
    fu_pools[FU_LS].resize(params.find<uint32_t>("ls.number", 1));

    // Latencies
    fu_latency[FU_INTEGER]    = params.find<uint32_t>("integer.latency", 1);
    fu_latency[FU_DIVIDER]    = params.find<uint32_t>("divider.latency", 20);
    fu_latency[FU_MULTIPLIER] = params.find<uint32_t>("multiplier.latency", 10);
    fu_latency[FU_LS]         = params.find<uint32_t>("ls.latency", 3);

    // Start all registers with ready status -1 (ready)
    rename_table.fill(-1);
}

// bool Core::tick(Cycle_t cycle)
// {
// 	cycle_count++;
// 	//std::cout<<"tick"<<std::endl;
// 	if(!busy)
// 	{
// 		fetch_instruction();
// 		busy = true;
// 	}
// 	// Block waiting for memory!
// 	if(!waiting_memory)
// 	{
// 		if(latency_countdown==0)
// 		{
// 			execute_instruction();
// 			if(instruction_count)
// 			{
// 				instruction_count->addData(1);
// 			}
// 		}
// 		else
// 		{
// 			latency_countdown--;
// 		}
// 	}
// 	return terminate;
// }
void Core::broadcast(int rs_id) {
    ReservationStation &rs = rs_all[rs_id];

    // Wake up any instruction waiting on this RS's result
    for (auto &other : rs_all) {
        if (!other.busy) continue;
        if (other.src1_tag == rs_id) other.src1_tag = -1;
        if (other.src2_tag == rs_id) other.src2_tag = -1;
    }

    // Update rename table: only if this RS is still the latest writer
    if (rs.dest_reg != 0xFF && rename_table[rs.dest_reg] == rs_id) {
        rename_table[rs.dest_reg] = -1;  // value is now in register file, so it can be marked ready
    }
}
void Core::handle_ls_completion(FunctionalUnit &fu, int rs_id) {
    ReservationStation &rs = rs_all[rs_id];

    if (!fu.memory_sent) {
        if (memory_pending) return;  // only one pending at a time

        memory_pending = true;
        fu.memory_sent = true;


        uint16_t addr = rs.mem_address;
        bool is_write = (rs.opcode == SW);

		//DO WE NEED TO CHECK CACHE
    }

}

void Core::do_write_register() {
	for (int type = 0; type < FU_TYPE_COUNT; type++) {
		for (auto &fu : fu_pools[type]) {
			if (!fu.busy) continue;
			if (fu.cycles_remaining > 0) continue;

			int rs_id = fu.rs_id;
			ReservationStation &rs = rs_all[rs_id];

			// Write result to destination register
            if (type == FU_LS) {
                // L/S: after address computation, send to cache
                //handle_ls_completion(fu, rs_id);
                continue;
            }
            broadcast(rs_id);

            // Free the FU and RS
            fu.busy = false;
            fu.rs_id = -1;
            rs.busy = false;
            if (rs.opcode == HALT) {
                terminate = true;
            }
		}
	}
}
void Core::do_execute() {
    for (int type = 0; type < FU_TYPE_COUNT; type++) {
        for (auto &fu : fu_pools[type]) {
            if (!fu.busy) continue;
            fu.cycles_remaining--;
        }
    }
}
bool Core::is_executing(int rs_id) {
    FUType type = rs_all[rs_id].fu_type;
    for (auto &fu : fu_pools[type]) {
        if (fu.busy && fu.rs_id == rs_id) return true;
    }
    return false;
}

void Core::do_read_operands() {
    for (int type = 0; type < FU_TYPE_COUNT; type++) {
        int start = rs_type_start[type];
        int count = rs_type_count[type];

        if (type == FU_LS) {
            //ONLY the head can dispatch
            if (ls_queue.empty()) continue;
            int head_id = ls_queue.front();
            ReservationStation &rs = rs_all[head_id];
            if (!rs.busy) continue;
            if (rs.src1_tag != -1 || rs.src2_tag != -1) continue;
            if (is_executing(head_id)) continue;

            int free_fu = -1;
            for (int f = 0; f < (int)fu_pools[FU_LS].size(); f++) {
                if (!fu_pools[FU_LS][f].busy) { free_fu = f; break; }
            }
            if (free_fu == -1) continue;

            fu_pools[FU_LS][free_fu].busy = true;
            fu_pools[FU_LS][free_fu].cycles_remaining = fu_latency[FU_LS];
            fu_pools[FU_LS][free_fu].rs_id = head_id;
            fu_pools[FU_LS][free_fu].instructions_executed++;
            count_reg_reads(rs);
            continue;
        }

        // Non-LS types: collect ready, sort by age and then dispatch
        std::vector<int> ready;
        for (int i = start; i < start + count; i++) {
            if (!rs_all[i].busy) continue;
            if (rs_all[i].src1_tag != -1 || rs_all[i].src2_tag != -1) continue;
            if (is_executing(i)) continue;
            ready.push_back(i);
        }
        if (ready.empty()) continue;

        std::sort(ready.begin(), ready.end(), [this](int a, int b) {
            return rs_all[a].age < rs_all[b].age;
        });

        for (int rs_id : ready) {
            int free_fu = -1;
            for (int f = 0; f < (int)fu_pools[type].size(); f++) {
                if (!fu_pools[type][f].busy) { free_fu = f; break; }
            }
            if (free_fu == -1) break;

            fu_pools[type][free_fu].busy = true;
            fu_pools[type][free_fu].cycles_remaining = fu_latency[type];
            fu_pools[type][free_fu].rs_id = rs_id;
            fu_pools[type][free_fu].instructions_executed++;
            count_reg_reads(rs_all[rs_id]);
        }
    }
}
void Core::decode_instruction(ReservationStation &rs, uint16_t instruction, uint16_t opcode, int global_rs_id) {
    switch (opcode) {
        case ADD: case SUB: case AND: case NOR:
        case DIV: case MUL: case MOD: case EXP:
        {
            uint16_t rd   = (instruction >> 8) & 0x07;
            uint16_t src1 = (instruction >> 5) & 0x07;
            uint16_t src2 = (instruction >> 2) & 0x07;
            rs.src1_ready= rename_table[src1];  // -1 if ready
            rs.src2_ready = rename_table[src2];
            rs.dest_reg = rd;
            rename_table[rd] = global_rs_id;
            break;
        }
        case LW:
        {
            uint16_t rd   = (instruction >> 8) & 0x07;
            uint16_t src1 = (instruction >> 5) & 0x07;
            rs.src1_ready = rename_table[src1];
            rs.src2_ready = -1;
            rs.dest_reg = rd;
            rename_table[rd] = global_rs_id;
            break;
        }
        case SW:
        {
            uint16_t src1 = (instruction >> 5) & 0x07;
            uint16_t src2 = (instruction >> 2) & 0x07;
            rs.src1_ready = rename_table[src1];
            rs.src2_ready = rename_table[src2];
            rs.dest_reg = 0xFF;  // no writeback
            break;
        }
        case LIZ: case LIS:
        {
            uint16_t rd = (instruction >> 8) & 0x07;
            rs.src1_ready = -1;
            rs.src2_ready = -1;
            rs.dest_reg = rd;
            rename_table[rd] = global_rs_id;
            break;
        }
        case LUI:
        {
            uint16_t rd = (instruction >> 8) & 0x07;
            rs.src1_ready = rename_table[rd];  // reads rd for low byte
            rs.src2_ready = -1;
            rs.dest_reg = rd;
            rename_table[rd] = global_rs_id;
            break;
        }
        case PUT:
        {
            uint16_t src1 = (instruction >> 5) & 0x07;
            rs.src1_ready = rename_table[src1];
            rs.src2_ready = -1;
            rs.dest_reg = 0xFF;
            break;
        }
        case HALT:
        {
            rs.src1_ready = -1;
            rs.src2_ready = -1;
            rs.dest_reg = 0xFF;
            break;
        }
    }
}
void Core::do_issue(){
	if (next_issue_index >= (int)program.size()) return; //New PC 
	uint16_t instruction = program[next_issue_index]; //Instruction line
	uint16_t opcode = instruction >> 11;
	std::cout<<"Fetched "<<names[opcode]<<std::endl;
	FUType type = get_fu_type(opcode);
	// Find a free reservation station of the right type
	int free_rs = -1;
    int start = rs_type_start[type];
    int count = rs_type_count[type];0

	for(int i = start; i < start + count; i++) {
		if (!rs_all[i].busy) {
			free_rs = i;
			break;
		}
	}

	//Check if no free rs, then stall because of hazard
	if (free_rs == -1) {
        stall_count++; //STALL 
        return;
    }
	 // Allocate the reservation station
	ReservationStation &rs = rs_all[free_rs];
	rs.busy = true;
	rs.instruction_index = next_issue_index;
	rs.opcode = opcode;
	rs.fu_type = type;
	rs.age = issue_age_counter++;
	if (type == FU_LS) {
		uint16_t src_reg = (instruction >> 5) & 0x07;
		rs.mem_address = registers[src_reg]; // FOR LW/SW This wont work unless register file is maintained
    	ls_queue.push_back(free_rs);  // add to LS queue for in-order issue
	}
	 // Determine source and destination registers from the instruction
	decode_instruction(rs, instruction, opcode, free_rs);
	next_issue_index++;

}
bool Core::tick(Cycle_t cycle)
{
	cycle_count++;
	do_write_register();  // finishing instructions free RS + FU
    do_execute();         // decrement counters on executing instructions (dont need to actually execute)
    do_read_operands();   // check if waiting instructions can dispatch
    do_issue();           // try to issue next instruction from trace
    
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
