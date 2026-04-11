#########################################
#		Inputs to the simulation		#
#########################################
import argparse
parser = argparse.ArgumentParser(description='Configuration options for this simulation.')

parser.add_argument('--program',
                    dest="program",
                    required=True,
                    action='store',
                    help='The program to run in the simulator')

parser.add_argument('--configuration',
                    dest="configuration",
                    required=True,
                    action='store',
                    help='The json file with the instruction latencies')


parser.add_argument('--output',
                    dest="output",
                    required=True,
                    action='store',
                    help='The json file to store the output statistics')
# Parse arguments
args = parser.parse_args()

## Print info ##
print("Running simulator using program "+args.program)
print("Configuration in  "+args.configuration)

#####################################
#	Load JSON file with configs
# SO
##################################### 
import json
with open(args.configuration, 'r') as inp_file:
  config=json.load(inp_file)
print("configuration:")
print(json.dumps(config, indent=2))

# Example config
# {
#   "integer.number": 2,
#   "integer.resnumber": 4,
#   "integer.latency": 1
#   "divider.number": 1,
#   "divider.resnumber": 2,
#   "divider.latency": 20
#   "multiplier.number": 2,
#   "multiplier.resnumber": 4,
#   "multiplier.latency": 10

#   "ls.number": 1,
#   "ls.resnumber": 8,
#   "ls.latency": 3

#   "cache": {
#     "associativity": 1,
#     "size": "4096B"
#   },
#   "clock":"1GHz"
# }

# Now the simulation
import sst

# Add our core to the simulation!
core = sst.Component("XSim","XSim.Core")
core.addParams({
  "clock_frequency": config["clock"],
  "program": args.program,
  "verbose": 0
})

core.addParams(config)
# Configure the memory interface in our CPU to use the standard interface
iface = core.setSubComponent("memory", "memHierarchy.standardInterface")
#iface.addParams({"debug" : 1, "debug_level" : 10})


# Now we add the memory to the simulation
# In this case we're using a simple memory controller (the memory frontend)
# This will map RAM from memory add_range_start (0) to add_range_end (64KiB)
memory = sst.Component("data_memory", "memHierarchy.MemController")
memory.addParams(
{
	'clock':		"1GHz",
	"verbose" : 		2,
	"addr_range_end":	64*1024-1,
})

# Now add cache to simulation
cache = sst.Component("L1_cache", "memHierarchy.Cache")
cache.addParams(
{
  "cache_line_size": 16,
  "associativity": config["cache"]["associativity"],
  "cache_size": config["cache"]["size"],
  "cache_frequency": config["clock"],
  "access_latency_cycles": 1,
  "L1": "true",
})

# Memory access timing model we attach it to the memory backend
#	- this does the specifics of the memory simulation
## SimpleMem has a constant access latency for all accesses
## 64KiB memory with 1000ns access latency
memory_timing = memory.setSubComponent("backend", "memHierarchy.simpleMem")
memory_timing.addParams({
	"access_time" : "1000ns",
	"mem_size" : "64KiB"
})

# Connect CPU & cache
cpu_cache_link = sst.Link("cpu_cache_link")
cpu_cache_link.connect(
	(iface,		"lowlink",	"500ps"),
	(cache,	"highlink",	"500ps")
)

# Connect cache & memory
cache_memory_link = sst.Link("cache_memory_link")
cache_memory_link.connect(
	(cache,		"lowlink",	"500ps"),
	(memory,	"highlink",	"500ps")
)

# Enable statistics for all components
sst.setStatisticLoadLevel(7)
sst.setStatisticOutput("sst.statOutputConsole")
sst.enableAllStatisticsForAllComponents()


