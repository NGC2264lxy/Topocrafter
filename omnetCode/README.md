# DragonflyPlus OMNeT++ Simulator

An OMNeT++ discrete-event simulator for a **Dragonfly+ hybrid data-center network** with electric switches, MEMS optical circuit switches, and optional **DRL-based topology reconfiguration** via an external Python agent.

## Overview

This project models a multi-group Dragonfly-style network where:

- **Hosts** generate application flows and inject data flits into the network.
- **Electric switches** (leaf and spine) perform store-and-forward routing and buffer flits.
- **MEMS optical switches** provide reconfigurable inter-group connectivity.
- **MEMS Controller** collects traffic demand, allocates optical topology, and pushes reconfiguration to switches.

Three topology allocation strategies are supported:

| `heuristicAlgorithm` | Method |
|---|---|
| `0` | DRL (Deep Reinforcement Learning) via Python socket agent |
| `1` | KM (Kuhn-Munkres weighted matching) |
| `2` | Edmonds (maximum bipartite matching) |

## Network Topology

Default configuration (see `simulations/omnetpp.ini`):

| Parameter | Default | Description |
|---|---|---|
| `numOfNode` | 64 | Number of compute hosts |
| `numOfGroups` | 8 | Number of pods/groups |
| `switchInPod` | 4 | Leaf/spine switches per group |
| `numOfSwitch` | 32 | Total electric switches (leaf + spine) |
| `numOfMEMS` | 8 | Number of MEMS optical switches |
| `ODataRate` | 100 Gbps | Optical link data rate |
| `EDataRate` | 40 Gbps | Electrical link data rate |

```
  Host ──EC── Leaf Switch ──EC── Spine Switch ──OC── MEMS Switch
                                    │
                                    └──EC── MEMS Controller
```

- **EC**: electrical channel (`EDataRate`)
- **OC**: optical channel (`ODataRate`)

## Directory Structure

```
omnetCode/
├── src/                        # C++ simulation modules
│   ├── Host.cc / Host.h        # Host: flow generation, FCT measurement
│   ├── Switch.cc / Switch.h    # Electric switch: routing, buffering
│   ├── socket_client.cc/.h     # TCP client for Python DRL agent
│   ├── MEMS/
│   │   ├── MEMS_Switch.*       # MEMS optical switch
│   │   ├── MEMS_Controller.*   # Central MEMS controller
│   │   ├── KM.*                # Kuhn-Munkres matching algorithm
│   │   └── Edmonds.*           # Edmonds matching algorithm
│   └── packet/
│       └── Packet.msg          # Message type definitions
├── simulations/
│   ├── dragonflyplus.ned       # Network topology definition
│   ├── omnetpp.ini             # Simulation configuration
│   └── run                     # Launch script
├── out/                        # Build output (object files)
├── Makefile                    # Top-level build entry
└── README.md
```

The companion Python DRL agent lives in the sibling directory:

```
topoCrafter/pythonCode/agent_port8878,8889/
```

## Prerequisites

- **OMNeT++** 5.x or 6.x (with `opp_makemake`, `opp_run`)
- **C++ compiler** supported by your OMNeT++ installation (MinGW/GCC on Windows, GCC on Linux)
- **Windows**: Winsock2 (`ws2_32`) — already linked in `src/Makefile`
- **Python agent** (optional, for DRL mode): Python 3.x, PyTorch, NumPy, Gym

## Build

From the project root:

```bash
# Generate Makefiles (first time or after adding new modules)
make makefiles

# Build release and debug binaries
make

# Clean
make clean
```

The executable is produced under `src/` (name depends on the Makefile target, e.g. `test_GNN_NET_V6_omnetCode.exe` or `DragonflyPlus.exe`).

On Windows with OMNeT++ IDE, you can also import the project and build from the IDE.

## Run

### Standalone (heuristic modes: KM / Edmonds)

```bash
cd simulations
../src/<executable_name> -n .:../src -u Cmdenv -c General omnetpp.ini
```

Or use the provided script (update the executable name inside if needed):

```bash
cd simulations
sh run
```

### With Python DRL Agent

1. Set `heuristicAlgorithm = 0` in `simulations/omnetpp.ini`.
2. Start the Python agent **before** launching the simulation:

```bash
cd ../pythonCode/agent_port8878,8889
python main.py
```

3. Run the OMNeT++ simulation from `simulations/`.

The simulator connects to localhost on two ports:

| Port | Purpose |
|---|---|
| `8889` (`socketPort`) | Topology allocation (DRL training agent) |
| `8878` (`socketPort_logic`) | Routing / next-hop computation |

## Configuration

Key parameters in `simulations/omnetpp.ini`:

### Traffic Generation

| Parameter | Description |
|---|---|
| `flowPattern` | `1`=uniform random, `2`=intra/inter-pod, `3`=trace-driven, `4`=sequential ring |
| `messageinterval_us` | Inter-message interval (microseconds) |
| `msgLength` | Application message size (bytes) |
| `flitByteLength` | Flit size (bytes, default 64) |
| `bigOrSmall` | Enable bimodal message size distribution |
| `**.FilePath` | Path to CSV traffic trace (required for `flowPattern=3`) |

> **Note:** The Host module reads the parameter `FilePath`. Make sure to set it in `omnetpp.ini`, for example:
> ```
> **.FilePath = "../omnetCode/simulations/dataset/lulesh.csv"
> ```

Trace file format: each time slice contains `numOfNode` rows; each row is comma-separated demand values to every destination host.

### MEMS & Allocation

| Parameter | Description |
|---|---|
| `heuristicAlgorithm` | `0`=DRL, `1`=KM, `2`=Edmonds |
| `allocateInterval` | Period for traffic demand collection and path-length reporting |
| `numOfSamples` | Traffic samples collected before each MEMS reconfiguration |
| `stepLength` | Max DRL training steps before simulation ends |
| `T_night` / `T_nightInMEMS` | Reconfiguration delay before re-enabling ports |

### MEMS Fault Simulation

| Parameter | Description |
|---|---|
| `blinkInterval` | Mean interval between port fault events |
| `blinkDuration` | Port recovery duration |
| `blinkDistribution` | `1`=exponential, `2`=uniform, `3`=normal |

## Output Files

During simulation, the following files are written to the working directory (typically `simulations/`):

| File | Content |
|---|---|
| `FCT<N>.txt` | Flow Completion Time per message |
| `P99FCT<N>.txt` | Approximate P99 FCT (last few flits) |
| `Traffic Demand<N>.txt` | Recorded traffic demand matrix |

`<N>` is the experiment repetition index (`repetition` parameter).

OMNeT++ scalar results (e.g. `ALL_FLIT_SEND`, `ALL_FLIT_RECEIVE`, per-port counters) are recorded at simulation finish.

## Socket Protocol

Communication between OMNeT++ and Python uses plain TCP strings:

**OMNeT++ → Python (DRL request):**
```
<finishFlag>;<msgType>;<trafficDemand>;<trafficMatrix>;<topology>;<faultMatrix>;<avgPathLength>
```

**Python → OMNeT++ (topology response):**
```
topomsg;<MEMS0_config>;<MEMS1_config>;...
```

**Python → OMNeT++ (routing response):**
```
routemsg;<src>,<dst>,<path>,<prob>;...
```

See `src/socket_client.cc` and `pythonCode/agent_port8878,8889/envs.py` for full parsing details.

## Module Summary

| Module | Role |
|---|---|
| `Host` | Generate flows, split messages into flits, measure end-to-end delay and FCT |
| `Switch` | Route flits (intra-group local / inter-group via MEMS topology), report traffic demand |
| `MEMS_Switch` | Forward flits through configured optical port mapping; simulate port faults |
| `MEMS_Controller` | Aggregate demand, run allocation algorithm, push setup messages |
| `serverClient` | Serialize/deserialize state for the Python DRL agent |

## License

This project is licensed under the **GNU Lesser General Public License v3** (LGPL-3). See individual source file headers for details.
