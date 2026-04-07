#pragma once

#include <sst/core/component.h>
// #include <sst/core/elementinfo.h>
// SST datatypes
#include <sst/core/sst_types.h>
// SST output to print information
#include <sst/core/output.h>

// SST interface for memory
#include <sst/core/interfaces/stdMem.h>

// SST statistics
#include <sst/core/statapi/stataccumulator.h>

#include <xsim_core/memory_wrapper.hpp>
#include <xsim_core/opcodes.hpp>

namespace XSim
{
namespace Core
{

class Core: public SST::Component
{
	public:
		// SST registration -- See https://sst-simulator.org/sst-docs/docs/guides/dev/devtutorial
		// SST example CPU with memory -- See https://github.com/sstsimulator/sst-elements/blob/master/src/sst/elements/memHierarchy/testcpu/scratchCPU.h
		SST_ELI_REGISTER_COMPONENT(
			Core,
			"XSim",
			"Core",
			SST_ELI_ELEMENT_VERSION(1,0,0),
			"Simple MIPS-based simulator",
			COMPONENT_CATEGORY_PROCESSOR
		)

		/*
		{
					"integer.number": 2,
					"integer.resnumber": 4,
					"integer.latency": 1
					"divider.number": 1,
					"divider.resnumber": 2,
					"divider.latency": 20
					"multiplier.number": 2,
					"multiplier.resnumber": 4,
					"multiplier.latency": 10

					"ls.number": 1,
					"ls.resnumber": 8,
					"ls.latency": 3

					"cache": {
						"associativity": 1,
						"size": "4096B"
					},
					"clock":"1GHz"
					}
		*/

		SST_ELI_DOCUMENT_PARAMS(
			{ "integer.number", "(uint) Number of integer units", "2" },
			{ "integer.resnumber", "(uint) Number of integer reservation stations", "4" },
			{ "integer.latency", "(uint) Latency of integer operations", "1" },
			{ "divider.number", "(uint) Number of divider units", "1" },
			{ "divider.resnumber", "(uint) Number of divider reservation stations", "2" },
			{ "divider.latency", "(uint) Latency of divider operations", "20" },
			{ "multiplier.number", "(uint) Number of multiplier units", "2" },
			{ "multiplier.resnumber", "(uint) Number of multiplier reservation stations", "4" },
			{ "multiplier.latency", "(uint) Latency of multiplier operations", "10" },
			{ "ls.number", "(uint) Number of load/store units", "1" },
			{ "ls.resnumber", "(uint) Number of load/store reservation stations", "8" },
			{ "ls.latency", "(uint) Latency of load/store operations", "3" },
			{ "cache.associativity", "(uint) Cache associativity", "1" },
			{ "cache.size", "(string) Cache size", "4096B" },
			{ "clock", "(string) Clock frequency", "1GHz" }
		)
			// { "verbose", "(uint) Verbosity for debugging. Increased numbers for increased verbosity.", "0" },
			// { "clock_frequency", "(string) Sets the clock of the core in Hz", "0"} ,
			// { "program", "(infile) Path to program to be executed by the simulator", "REQUIRED"} ,
			// { "liz", "(uint) Latency of the liz instruction", "1"} ,
			// { "put", "(uint) Latency of the put instruction", "1"} ,
			// { "sw", "(uint) Latency of the sw instruction", "1"} ,
			// { "lw", "(uint) Latency of the lw instruction", "1"} ,
			// { "halt", "(uint) Latency of the halt instruction", "1"}
		//)

		// Statistics for our component
		SST_ELI_DOCUMENT_STATISTICS(
			{ "instructions", "Number of instructions executed", "", 0 }
		)

		// This is used to connect the memory interface, thus we don't need to implement one!
		SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS( {"memory", "Interface to memory (e.g., caches)", "SST::Interfaces::StandardMem"} )

	private:
		using Super=SST::Component;
		template<typename Type> using Statistics=SST::Statistics::Statistic<Type>;
		using Cycle_t=SST::Cycle_t;
		using StandardMem=SST::Interfaces::StandardMem;
		using Output=SST::Output;
		using ComponentId_t=SST::ComponentId_t;
		using Params=SST::Params;
		using Clock=SST::Clock;
		using TimeConverter = ::SST::TimeConverter;

	public:
		/** These functions are defined by SST **/
		Core(ComponentId_t id, Params& params);
		virtual void init(unsigned int phase) override final;
		virtual void setup() override final;
		virtual void finish() override final;
		bool tick(Cycle_t cycle);

	private:
		/** For completeness **/
		Core();
		Core(const Core& params);
		Core operator=(const Core& params);
		virtual ~Core() = default;

	protected:
		// Method to load the program from a file
		void load_program(Params &params);
		// Method to load instruction latencies
		void load_latencies(Params &params);

	private:
		/** Parameters **/
		// Prints verbose information
		int verbose{0};
		// The core clock frequency
		std::string clock_frequency{"0Hz"};
		// The vector with the program instructions
		std::vector<uint16_t> program;
		// The map with instruction latencies
		std::map<uint32_t, uint32_t> latencies;
		// The map with instruction names (for printing)
		std::map<uint32_t, std::string> names;
		uint64_t total_instructions = 0;
		uint64_t total_cycles = 0;
		std::map<uint16_t, uint64_t> instruction_counts;
		uint64_t reg_reads = 0;

		/** CPU state **/
		// The program counter
		uint32_t pc{0};
		// Registers
		std::array<uint16_t,8> registers;
		// Busy processing instruction
		bool busy{false};
		// Busy processing instruction
		int latency_countdown{0};
		// Waiting for memory return
		bool waiting_memory{false};
		// Finished simulation
		bool terminate{false};

		// SST managed output
		Output *output{nullptr};

		// Wrapper to simplify memory access with SST's memory interface
		MemoryWrapper *memory_wrapper;

		/** Functions to process events **/
		// Fetch a new instruction
		void fetch_instruction();
		// Execute a new instruction
		void execute_instruction();

		// Statistics definitions
		Statistics<uint64_t> *instruction_count;

		// TimeConverter -> memory needs this
		TimeConverter* tc{nullptr};
};

}
}
