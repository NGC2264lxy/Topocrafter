# Topocrafter
A co-simulation framework for **Dragonfly+ hybrid data-center networks** that combines an **OMNeT++ packet-level simulator** with an optional **Python deep reinforcement learning (DRL) agent** for MEMS optical topology reconfiguration and inter-pod routing.

## Overview

Modern Dragonfly-style networks use electrical switches for intra-group traffic and **MEMS optical circuit switches** for reconfigurable inter-group connectivity. Reconfiguring MEMS port mappings in response to shifting traffic is a hard combinatorial problem — especially under link faults and varying flow patterns.

**topoCrafter** provides:

- A discrete-event **OMNeT++ simulator** (`omnetCode/`) modeling hosts, electric switches, MEMS switches, and a central MEMS controller
- An external **Python DRL agent** (`pythonCode/`) that learns topology and routing policies via GAT-based GE-PPO with RND exploration
- **Classical heuristics** (Kuhn-Munkres and Edmonds matching) as baselines, selectable without Python

```
┌─────────────────────────────────────────────────────────────────┐
│                         topoCrafter                              │
├──────────────────────────────┬──────────────────────────────────┤
│         omnetCode/           │           pythonCode/             │
│  OMNeT++ Dragonfly+ Sim      │  GAT + GE-PPO DRL Agent          │
│                              │                                   │
│  Host ── Switch ── MEMS      │  MyTopoAgent   (port 8889)       │
│         MEMS Controller ◄────┼──► MyRouteAgent  (port 8878)      │
│                              │    MyGeneralAgent                  │
└──────────────────────────────┴──────────────────────────────────┘
         TCP sockets (localhost 8878 / 8889)
```

## Features

| Feature | Description |
|---|---|
| Hybrid network model | Electrical leaf/spine switches + MEMS optical cross-connects |
| Multiple traffic patterns | Uniform random, intra/inter-pod, trace-driven (CSV), sequential ring |
| Topology allocation | DRL, KM weighted matching, or Edmonds maximum matching |
| Fault simulation | Configurable MEMS port blink/recovery events |
| Online DRL training | Python agent trains while co-simulating with OMNeT++ |
| Performance metrics | Flow Completion Time (FCT), P99 FCT, traffic demand logs |

## Project Structure

```
topoCrafter/
├── README.md                          # This file
├── omnetCode/                         # OMNeT++ simulation
│   ├── src/                           # C++ modules (Host, Switch, MEMS, socket client)
│   ├── simulations/                   # NED topology, omnetpp.ini, run script
│   └── README.md                      # Simulator build & configuration guide
└── pythonCode/                        # Python DRL agent
    ├── agent_port8878,8889/           # Main agent source
    └── README.md                      # Agent setup & hyperparameter guide
```

## Prerequisites

| Component | Requirements |
|---|---|
| Simulator | OMNeT++ 5.x or 6.x, C++ compiler (GCC / MinGW) |
| DRL agent (optional) | Python 3.8+, PyTorch, PyTorch Geometric, NumPy, SciPy, Gym |
| OS | Linux or Windows (Winsock2 linked in Makefile) |

See [omnetCode/README.md](omnetCode/README.md) and [pythonCode/README.md](pythonCode/README.md) for detailed dependency lists.

## Quick Start

### Build the simulator

```bash
cd omnetCode
make makefiles
make
```

### Mode A — Heuristic baselines (no Python)

Set `heuristicAlgorithm` in `omnetCode/simulations/omnetpp.ini`:

| Value | Algorithm |
|---|---|
| `1` | Kuhn-Munkres (KM) weighted matching |
| `2` | Edmonds maximum bipartite matching |

Run:

```bash
cd omnetCode/simulations
../src/<executable_name> -n .:../src -u Cmdenv -c General omnetpp.ini
```

### Mode B — DRL co-simulation

1. Set `heuristicAlgorithm = 0` in `omnetpp.ini`.
2. **Start the Python agent first:**

```bash
cd pythonCode/agent_port8878,8889
python main.py
```

3. **Then launch the simulation:**

```bash
cd omnetCode/simulations
../src/<executable_name> -n .:../src -u Cmdenv -c General omnetpp.ini
```

| Port | Direction | Purpose |
|---|---|---|
| `8889` | OMNeT++ ↔ Python | Topology allocation (`topomsg`) |
| `8878` | OMNeT++ ↔ Python | Routing decisions (`routemsg`) |

> **Important:** The Python agent must be running before OMNeT++ connects, or socket initialization will fail.

## Default Network Configuration

From `omnetCode/simulations/omnetpp.ini`:

| Parameter | Default | Description |
|---|---|---|
| `numOfNode` | 64 | Compute hosts |
| `numOfGroups` | 8 | Pods / groups |
| `numOfSwitch` | 32 | Electric switches (leaf + spine) |
| `numOfMEMS` | 8 | MEMS optical switches |
| `ODataRate` | 100 Gbps | Optical link rate |
| `EDataRate` | 40 Gbps | Electrical link rate |
| `flowPattern` | 3 | Trace-driven traffic (CSV) |
| `allocateInterval` | 0.0001 s | MEMS reconfiguration period |

Traffic trace path (update to your local file):

```ini
**.trafficFile = "../omnetCode/simulations/dataset/lulesh.csv"
```

## How It Works

1. **Hosts** generate flows and inject flits; **switches** route and buffer traffic within and across groups.
2. The **MEMS Controller** periodically collects traffic demand matrices and current topology state.
3. Depending on `heuristicAlgorithm`:
   - **DRL (`0`)**: State is serialized and sent to Python; the agent returns a new MEMS port mapping and routing probabilities.
   - **KM / Edmonds (`1` / `2`)**: Allocation runs entirely inside the C++ controller.
4. **MEMS switches** apply the new configuration (with reconfiguration delay `T_night` / `T_nightInMEMS`).
5. Simulation records **FCT**, link utilization, and scalar statistics.

### DRL Agent Architecture

Three GAT-based agents trained with GE-PPO + RND:

| Agent | Task |
|---|---|
| `MyTopoAgent` | MEMS port mapping / topology reconfiguration |
| `MyRouteAgent` | Path selection probabilities per `(src, dst)` flow |
| `MyGeneralAgent` | Global value estimation and fault-aware reward |

Rewards balance link utilization variance, average path length, and fault penalties. Full details: [pythonCode/README.md](pythonCode/README.md).

## Output

Simulation output (typically under `omnetCode/simulations/`):

| File | Content |
|---|---|
| `FCT<N>.txt` | Per-flow completion time |
| `P99FCT<N>.txt` | Approximate P99 FCT |
| `Traffic Demand<N>.txt` | Recorded demand matrix |

DRL training logs and model checkpoints are written by the Python agent (see [pythonCode/README.md](pythonCode/README.md#output-files)).

## Documentation

| Document | Contents |
|---|---|
| [omnetCode/README.md](omnetCode/README.md) | Build, run, INI parameters, socket protocol, module reference |
| [pythonCode/README.md](pythonCode/README.md) | Agent setup, hyperparameters, training loop, reward design |

## License

This project is licensed under the **GNU Lesser General Public License v3 (LGPL-3)**. See individual source file headers for details.
