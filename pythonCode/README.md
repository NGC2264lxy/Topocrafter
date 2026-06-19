# topoCrafter Python DRL Agent

A Python deep reinforcement learning (DRL) agent for **Dragonfly+ data-center networks**. It communicates with the [OMNeT++ simulator](../omnetCode/README.md) over TCP sockets to perform **MEMS optical topology reconfiguration** and **inter-pod routing decisions**.

## Overview

This module acts as an external decision engine while the OMNeT++ simulation runs. When `heuristicAlgorithm = 0` is set in `omnetpp.ini`, the MEMS Controller sends traffic demand, topology state, and fault information to Python. Three **GAT (Graph Attention Network)** agents handle different subtasks:

| Agent | Role |
|---|---|
| `MyTopoAgent` | Optimizes MEMS port mapping (topology reconfiguration) from link state and fault information |
| `MyRouteAgent` | Assigns routing probabilities for each `(src, dst)` flow over candidate paths |
| `MyGeneralAgent` | Estimates global state value to support overall reward optimization |

Training uses **GE-PPO** — a PPO variant with RND (Random Network Distillation) intrinsic rewards. Exploration signals come from `RNDModel`; experience is stored in `ReplyBuffer`.

```
  OMNeT++ (MEMS Controller)
       │  TCP 8889: topology request / topomsg response
       │  TCP 8878: routing request / routemsg response
       ▼
  HPCEnvironment (envs.py)
       │
       ├── MyTopoAgent    → topology actions
       ├── MyRouteAgent   → routing actions
       └── MyGeneralAgent → global value estimation
```

## Directory Structure

```
pythonCode/
├── README.md
└── agent_port8878,8889/          # Main program (ports 8878/8889 for routing/topology)
    ├── main.py                   # Entry point: init env/models, start training loop
    ├── envs.py                   # HPCEnvironment: sockets, rewards, routing table
    ├── Agent.py                  # GAT agents, sampling, and PPO training logic
    ├── RND.py                    # Random Network Distillation exploration module
    ├── replay_buffer.py          # Experience replay buffer
    └── model.py                  # Early GAT prototype (training uses Agent.py)
```

## Prerequisites

- **Python** 3.8+
- **PyTorch** (CUDA build recommended for faster training)
- **PyTorch Geometric** (`GATv2Conv`)
- **NumPy**
- **SciPy**
- **Gym** (`gym.spaces` for action/observation space definitions)

Example installation:

```bash
pip install torch torchvision torchaudio
pip install torch-geometric
pip install numpy scipy gym
```

> PyTorch Geometric must match your local PyTorch / CUDA version. See the [official PyG installation guide](https://pytorch-geometric.readthedocs.io/en/latest/install/installation.html).

## Run

### 1. Configure the OMNeT++ simulation

In `omnetCode/simulations/omnetpp.ini`:

```ini
heuristicAlgorithm = 0
```

### 2. Start the Python agent (before the simulation)

```bash
cd pythonCode/agent_port8878,8889
python main.py
```

The agent listens on localhost and waits for OMNeT++ to connect:

| Port | Purpose |
|---|---|
| `8889` | Topology allocation (Topo Agent) |
| `8878` | Routing decisions (Route Agent) |

### 3. Launch the OMNeT++ simulation

```bash
cd omnetCode/simulations
../src/<executable_name> -n .:../src -u Cmdenv -c General omnetpp.ini
```

For the full co-simulation workflow, see [omnetCode/README.md](../omnetCode/README.md#with-python-drl-agent).

## Hyperparameters

Configure via `main.py` CLI arguments (`python main.py --help` for all options):

| Parameter | Default | Description |
|---|---|---|
| `--num_pods` | 32 | Number of pods |
| `--max_interact_steps` | 3e6 | Max interaction steps with OMNeT++ |
| `--max_iterations` | 50000 | PPO training iterations |
| `--batch_size` | 16 | Batch size (number of episodes) |
| `--lr` | 1e-5 | Learning rate |
| `--gamma` | 0.99 | Discount factor |
| `--lamda` | 0.5 | RND reward weight |
| `--epsilon` | 0.1 | PPO clip coefficient |
| `--c3` | 0.5 | RND loss weight |
| `--seed` | 3407 | Random seed |

Example:

```bash
python main.py --num_pods 32 --max_iterations 10000 --lr 1e-4 --seed 42
```

## Socket Protocol

Communication with OMNeT++ uses plain TCP strings. Parsing logic is in `envs.py` → `handle_recvData()`.

**OMNeT++ → Python (state report, `elsemsg`):**

```
<finishFlag>;<msgType>;<trafficDemand>;<trafficMatrix>;<topology>;<faultMatrix>;<avgPathLength>
```

Each matrix segment is semicolon-separated rows with comma-separated values.

**Python → OMNeT++ (topology response):**

```
topomsg;<MEMS0_config>;<MEMS1_config>;...
```

**Python → OMNeT++ (routing response):**

```
routemsg;<src>,<dst>,<path>,<prob>;...
```

Here `path` is a `-`-joined sequence of pod IDs, e.g. `0-5-12`.

## Training Loop

1. `HPCEnvironment.reset()` waits on port `8889` for an OMNeT++ connection and receives the initial state.
2. `MyTopoAgent` outputs topology actions from the link-state matrix; actions above a threshold (default 0.7) are sent as `topomsg`.
3. On port `8878`, updated state is received; `MyRouteAgent` produces routing probabilities and sends `routemsg`.
4. `MyGeneralAgent` computes global value; rewards from all three agents are stored in `ReplyBuffer`.
5. `training_loop()` runs PPO updates after collecting enough episodes and periodically saves model weights.

## Output Files

### Model Checkpoints

Saved to the current working directory during training:

| File | Description |
|---|---|
| `topo_agent_model.pth` | Topo agent (updated on best reward) |
| `route_agent_model.pth` | Route agent |
| `general_agent_model.pth` | Global value network |
| `30%_flash_best_*_agent_model.pth` | Snapshot after each interaction round |

### Training Logs

`Agent.py` appends metrics to `../fault/` (relative to `agent_port8878,8889/`):

| File | Content |
|---|---|
| `topo_inference_times.txt` | Topology inference latency |
| `route_inference_times.txt` | Routing inference latency |
| `topo_loss.txt` / `route_loss.txt` | Policy loss |
| `avg_reward_topo.txt` / `avg_reward_route.txt` | Average rewards |
| `iteration_time.txt` | Per-iteration runtime |

Ensure the `fault/` directory exists before running, or update the path in `write_to_file()` inside `Agent.py`.

## Reward Design

`envs.py` → `cal_cur_reward()` combines:

- **Link utilization balance** (`lu_reward`): lower variance across links yields higher reward
- **Average path length** (`path_reward`): shorter paths yield higher reward
- **Fault link penalty** (General Agent): negative reward proportional to fault links in `link_state`

The topology agent additionally computes a detection reward from the gap between action probabilities and link state.

## Notes

1. **Startup order**: Start the Python agent before OMNeT++; otherwise socket connections will fail.
2. **Working directory**: `main.py` contains a hard-coded `os.chdir(...)` path — update it to your project path or remove it before deployment.
3. **GPU requirement**: `replay_buffer.py` → `get_training_data()` moves batches to `'cuda'` by default; on CPU-only machines, switch to `'cpu'` or route through `get_device()`.
4. **Pod scale**: GAT layers and action spaces are sized for 32×32 matrices. Changing `--num_pods` requires matching updates in network definitions and `envs.py`.

## Mapping to OMNeT++

| Python Module | OMNeT++ Module |
|---|---|
| `envs.py` | `socket_client.cc` |
| Topo Agent (8889) | `MEMS_Controller` topology allocation |
| Route Agent (8878) | Routing / next-hop computation |
| `heuristicAlgorithm=0` | DRL mode switch |

## License

Same as the topoCrafter project. See individual source file headers for details.
