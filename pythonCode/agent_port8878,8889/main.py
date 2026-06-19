import argparse
from envs import HPCEnvironment
import os
import torch
import socket
from Agent import parse_args,get_device,training_loop,testing_loop,MyTopoAgent,MyRouteAgent,MyGeneralAgent
from replay_buffer import ReplyBuffer
from RND import RNDModel

def parse_args():
    parser = argparse.ArgumentParser(description="Hyperparameters Setting for GE-PPO in HPC environment")
    parser.add_argument("--max_interact_steps", type=int, default=int(3e6), help="Maximum number of interact steps with OMNeT++")
    parser.add_argument("--max_train_steps", type=int, default=int(3e6), help="Maximum number of training steps")
    parser.add_argument("--episode_limit", type=int, default=1, help="Maximum number of steps per episode")
    parser.add_argument("--max_recv_data", type=int, default=8192, help="Maximum number of bytes received from the client")
    parser.add_argument("--num_pods", type=int, default=32, help="The number of pods in network")
    parser.add_argument("--max_path_length", type=int, default=2, help="The maximum length of global paths")
    parser.add_argument("--pod_bandwidth", type=int, default=100, help="The bandwidth of links between pods (Gbps)")
    parser.add_argument("--allocate_interval", type=int, default=1, help="The allocate time interval of traffic")

    parser.add_argument("--batch_size", type=int, default=16, help="Batch size (the number of episodes)")
    parser.add_argument("--mini_batch_size", type=int, default=8, help="Minibatch size (the number of episodes)")
    parser.add_argument("--lr", type=float, default=1e-5, help="Learning rate")
    parser.add_argument("--gamma", type=float, default=0.99, help="Discount factor")
    parser.add_argument("--lamda", type=float, default=0.5, help="RND reward weight")
    parser.add_argument("--seed", type=int, default=3407, help="Random seed")

    parser.add_argument("--max_iterations", type=int, help="Number of training iterations", default=50000)
    parser.add_argument("--n_epochs", type=int, help="Training epochs per iteration", default=1)
    parser.add_argument("--n_actors", type=int, help="Number of actors", default=1)
    parser.add_argument("--horizon", type=int, help="Number of timestamps per actor", default=1)
    parser.add_argument("--epsilon", type=float, help="Epsilon", default=0.1)
    
    parser.add_argument("--c1", type=float, help="Smoothing coefficient", default=0.9)
    parser.add_argument("--c2", type=float, help="Initial baseline", default=0)
    parser.add_argument("--c3", type=float, help="RND loss weight", default=0.5) 

    parser.add_argument("--n_test_episodes", type=int, help="Number of episodes to render", default=5300)

    parser.add_argument("--N", type=int, help="Number of nodes", default=32)
    parser.add_argument("--obs_dim", type=int, help="Observation dimension", default=32)
    parser.add_argument("--gat_hidden_dim", type=int, help="GAT hidden layer dimension", default=16)
    parser.add_argument("--gat_output_dim", type=int, help="GAT output layer dimension", default=32)
    parser.add_argument("--state_dim", type=int, help="State dimension", default=256)
    parser.add_argument("--rnd_hidden_dim", type=int, default=128)

    return parser.parse_args()

os.chdir("D:/meteor/PPO_flash")
MODEL_PATH="D:/meteor/PPO_flash/save_model.pth"


if __name__ == '__main__':
    args = parse_args()
    env = HPCEnvironment(args)

    device = get_device()

    model_topo = MyTopoAgent(in_shape=(args.num_pods, args.num_pods), 
                        n_actions=env.topology_action_space.shape[0], 
                        ).to(device)

    model_route = MyRouteAgent(in_shape=(args.num_pods, args.num_pods),           
                            ).to(device)
    model_general = MyGeneralAgent(in_shape=(args.num_pods, args.num_pods),           
                        ).to(device)

    ReplyBuffer = ReplyBuffer(args)

    for cur_step in range(args.max_interact_steps):
        print("cur step: %d / %d " % (cur_step + 1, args.max_interact_steps))
        done = False
        while not done:
            training_loop(env, model_topo, model_route, model_general,args, ReplyBuffer, 
                        max_iterations=args.max_iterations, 
                        n_actors=args.n_actors, 
                        horizon=args.horizon, 
                        lamda=args.lamda, 
                        gamma=args.gamma, 
                        epsilon=args.epsilon, 
                        n_epochs=args.n_epochs, 
                        batch_size=args.batch_size, 
                        lr=args.lr, 
                        c1=args.c1, 
                        c2=args.c2, 
                        c3=args.c3,
                        device=device, 
                        seed=args.seed)

            torch.save(model_topo.state_dict(), "30%_flash_best_topo_agent_model.pth")
            torch.save(model_route.state_dict(), "30%_flash_best_route_agent_model.pth")
            torch.save(model_general.state_dict(), "30%_flash_best_general_agent_model.pth")

            done = env.get_cur_comFlag()


