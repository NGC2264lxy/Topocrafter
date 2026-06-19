from argparse import ArgumentParser
import numpy as np
import time
import torch
import torch.nn as nn
import torch.nn.functional as F
import os

from torch.optim import Adam
from torch.optim.lr_scheduler import LinearLR
from torch.distributions.categorical import Categorical

from torch_geometric.nn import GATv2Conv
from scipy.special import kl_div
from RND import RNDModel
from replay_buffer import ReplyBuffer

MODEL_PATH="../save_model.pth"

def orthogonal_init(layer, gain=1.0):
    for name, param in layer.named_parameters():
        if 'bias' in name:
            nn.init.constant_(param, 0)
        elif 'weight' in name:
            nn.init.orthogonal_(param, gain=gain)

def write_to_file(filename, content):
    fullpath="..\\fault"
    with open(os.path.join(fullpath, filename), "a") as f:
        f.write(content + "\n")

def parse_args():
    """Parse command-line arguments."""
    parser = ArgumentParser()
    parser.add_argument("--batch_size", type=int, help="Batch size", default=16 )
    parser.add_argument("--max_iterations", type=int, help="Number of training iterations", default=50000)
    parser.add_argument("--n_epochs", type=int, help="Training epochs per iteration", default=1)

    parser.add_argument("--n_actors", type=int, help="Number of actors", default=1)
    parser.add_argument("--horizon", type=int, help="Number of timestamps per actor", default=1) 

    parser.add_argument("--epsilon", type=float, help="Epsilon", default=0.1)
    
    parser.add_argument("--lr", type=float, help="Learning rate", default=1e-5)
    parser.add_argument("--gamma", type=float, help="Discount factor gamma", default=0.99)
    parser.add_argument("--c1", type=float, help="Smoothing coefficient", default=0.9)
    parser.add_argument("--c2", type=float, help="Initial baseline", default=0)
    parser.add_argument("--c3", type=float, help="RND loss weight", default=0.5)
    
    parser.add_argument("--n_test_episodes", type=int, help="Number of episodes to render", default=5300)

    parser.add_argument("--num_pods", type=int, help="Number of pod in system", default=32)


    parser.add_argument("--gat_hidden_dim", type=int, help="GAT hidden layer dimension", default=16)
    parser.add_argument("--gat_output_dim", type=int, help="GAT output layer dimension", default=32)
    parser.add_argument("--state_dim", type=int, help="State dimension", default=256)

    parser.add_argument("--rnd_hidden_dim", type=int, default=128)

    return vars(parser.parse_args())


def get_device():
    if torch.cuda.is_available():
        device = torch.device("cuda")
        print(f"Found GPU device: {torch.cuda.get_device_name(device)}")
    else:
        device = torch.device("cpu")
        print("No GPU found: Running on CPU")
    return device


@torch.no_grad()
def run_timestamps(env,model_topo, model_route,model_general,args ,ReplyBuffer,horizen, device):

    """Run the given policy in the environment for a fixed number of timestamps."""
    timestamps=horizen
    state = env.reset()
    
    state_topo = state[0]
    state_route = state[1]
    for ts in range(timestamps):
        state_tensor_topo = torch.tensor(state_topo, dtype=torch.float32).unsqueeze(0).to(device)
        edge_index = env.get_edge_index()
        edge_index = torch.tensor(edge_index, dtype=torch.long).to(device)  
        
        topo_start_time = time.perf_counter()
        action_probs_topo, value_topo = model_topo(state_tensor_topo, edge_index)
        topo_inference_time = (time.perf_counter() - topo_start_time) 
        write_to_file("topo_inference_times.txt",f"topo,{topo_inference_time:.4f}\n")

        action_topo_single = action_probs_topo[0].cpu().numpy()
        topo_reward,env.selected_matrix,routing_table = env.step(action_topo_single,1,threshold=0.7)

        state_tensor_route =torch.tensor(state_route,dtype=torch.float32).unsqueeze(0).to(device)
        min_val = state_tensor_route.min()
        max_val = state_tensor_route.max()
        if max_val > min_val:
            state_tensor_route= (state_tensor_route - min_val) / (max_val - min_val)
        min_val = state_route.min()
        max_val = state_route.max()
        if max_val > min_val:
            state_route= (state_route - min_val) / (max_val - min_val)

        route_start_time = time.perf_counter()
        action_route, action_probs_route, value_route = model_route(state_tensor_route, edge_index,routing_table)
        route_inference_time = (time.perf_counter() - route_start_time)   
        write_to_file("route_inference_times.txt",f"route,{route_inference_time:.4f}\n")
    
        edge_index2=env.get_edge_index2(state_topo)
        state_tensor_route = state_tensor_route.requires_grad_()
        edge_index2 = torch.tensor(edge_index2, dtype=torch.long).to(device)

        general_start_time = time.perf_counter()
        value_general=model_general(state_tensor_route,edge_index2)
        general_inference_time = (time.perf_counter() - general_start_time) 
        write_to_file("general_inference_times.txt",f"{ts},general,{general_inference_time:.4f}\n")

        route_reward ,done = env.step(action_route,2,threshold=0.7)
        general_reward,done =env.step(action_route,3,threshold=0.7)
        rnd_reward_topo = model_topo.rnd.calculate_rnd_loss(state_tensor_topo).item()
        rnd_reward_route = model_route.rnd.calculate_rnd_loss(state_tensor_route).item()
        ReplyBuffer.store_transition(
            episode_step=ts,
            obs_topo=state_topo,
            obs_route=state_route,
            v_topo=value_topo,
            v_route=value_route,
            v_general=value_general,
            a_route=action_route,
            a_prob_topo=action_probs_topo, 
            a_prob_route=action_probs_route,
            r_topo=topo_reward,
            r_route=route_reward, 
            r_general=general_reward,
            rnd_r_topo=rnd_reward_topo,
            rnd_r_route=rnd_reward_route,
            done_n=done
        )

        if done:
            state = env.reset()
            state_topo = state[0]
            state_route = state[1]
    ReplyBuffer.episode_num += 1

    return 

class GATNet(nn.Module):
    def __init__(self, input_dim, hidden_dim, output_dim, heads):
        super(GATNet, self).__init__()
        self.gat1 = GATv2Conv(input_dim, output_dim, heads=heads, concat=True)
       
    def forward(self, x, edge_index):
        batch_size, num_nodes, input_dim = x.size()
        x = x.view(-1, input_dim)
        x = self.gat1(x, edge_index)
        x = F.leaky_relu(x)
        x = x.view(batch_size, num_nodes, -1)
        return x
    
class Actor(nn.Module):
    def __init__(self, state_dim, gat_hidden_dim, gat_output_dim, num_heads):
        super(Actor, self).__init__()
        self.gat_net = GATNet(
            input_dim=state_dim,
            hidden_dim=gat_hidden_dim,
            output_dim=gat_output_dim,
            heads=num_heads
        )

    def forward(self, x, edge_index):
        x = self.gat_net(x, edge_index)
        prob=x
        prob = torch.softmax(prob, dim=-1)
        return prob
      
class Critic(nn.Module):
    def __init__(self,state_dim, gat_hidden_dim, gat_output_dim, num_heads):
        super(Critic, self).__init__()
        self.gat_net = GATNet(
            input_dim=state_dim,
            hidden_dim=gat_hidden_dim,
            output_dim=gat_output_dim,
            heads=num_heads
        )
        self.fc = nn.Linear(num_heads*gat_output_dim, 1)
    def forward(self, x, edge_index):
        x = self.gat_net(x, edge_index) 
        value = self.fc(x)
        return value

class MyTopoAgent(nn.Module):
    """PPO agent for topology selection."""

    def __init__(self,in_shape, n_actions, hidden_d=16, share_backbone=False, rnd_hidden_dim=128):
        super(MyTopoAgent, self).__init__()

        self.in_shape = in_shape
        self.n_actions = n_actions
        self.hidden_d = hidden_d
        self.share_backbone = share_backbone

        in_dim = np.prod(in_shape)
    
        def to_features():
            return nn.Sequential(
                nn.Flatten(),
                nn.Linear(in_dim, hidden_d),
                nn.ReLU(),
                nn.Linear(hidden_d, hidden_d),
                nn.ReLU()
            )
        
        self.backbone = to_features() if self.share_backbone else nn.Identity()

        self.actor = Actor(state_dim=in_shape[1], gat_hidden_dim=hidden_d, gat_output_dim=in_shape[1], num_heads=32)
        self.critic = Critic(state_dim=in_shape[1], gat_hidden_dim=hidden_d, gat_output_dim=hidden_d, num_heads=32)

        device = get_device()
        self.rnd = RNDModel(device,rnd_input_size=in_dim, rnd_output_size=rnd_hidden_dim)

    def forward(self, x, edge_index):
        features = self.backbone(x)
        action_probs = self.actor(features, edge_index)
       
        action_probs_reshape = torch.zeros(1,32,32)
        for i in range (32):
            for j in range(32):
                action_probs_reshape[0,i,j]=action_probs[0,i,j * 32:(j + 1) * 32 - 1].mean()
       
        for i in range (32):
            for j in range(32):
                if x[0,i,j]:
                    action_probs_reshape[0,i,j]=action_probs_reshape[0,i,j]*(x[0,i,j])+0.7
        value = self.critic(features, edge_index)
        return action_probs_reshape, value
    
class MyRouteAgent(nn.Module):
    """PPO agent for routing."""

    def __init__(self,in_shape,hidden_d=100, share_backbone=False,rnd_hidden_dim=128,num_choices=32,num_heads=32):
        super(MyRouteAgent, self).__init__()

        self.in_shape = in_shape
        self.hidden_d = hidden_d
        self.share_backbone = share_backbone

        self.num_choices = num_choices  
        self.num_heads = num_heads

        in_dim = np.prod(in_shape)
    
        def to_features():
            return nn.Sequential(
                nn.Flatten(),
                nn.Linear(in_dim, hidden_d),
                nn.ReLU(),
                nn.Linear(hidden_d, hidden_d),
                nn.ReLU()
            )
        
        self.backbone = to_features() if self.share_backbone else nn.Identity()

        self.actor = Actor(state_dim=in_shape, gat_hidden_dim=hidden_d, gat_output_dim=num_choices, num_heads=num_heads)
        self.critic = Critic(state_dim=in_shape, gat_hidden_dim=hidden_d, gat_output_dim=hidden_d, num_heads=num_heads)

        device = get_device()
        self.rnd = RNDModel(device,rnd_input_size=in_dim, rnd_output_size=rnd_hidden_dim)

    def forward(self, x, edge_index,routing_table):
        features = self.backbone(x)
        action_probs = self.actor(features, edge_index)

        routing_dict = {}
        for src, dst, paths, flow in routing_table:
            if src not in routing_dict:
                routing_dict[src] = {}
            routing_dict[src][dst] = paths

        actions = {}
        prob_mean=np.zeros((32,32))
        for src in range(32):
            actions[src] = {}
            for dst in range(32):
                if src == dst or dst not in routing_dict.get(src, {}):
                    actions[src][dst] = None
                    continue

                routes = routing_dict[src][dst]
                if not routes:
                    actions[src][dst] = None
                    continue

                path_probs = action_probs[0, src, dst * 32:(dst + 1) * 32 - 1].detach().cpu().numpy()
                
                path_bias = np.zeros_like(path_probs)
                for i, route in enumerate(routes):
                    path, is_direct = route
                    if is_direct:
                        if i<len(path_bias):
                            path_bias[i] += 2

                adjusted_probs = path_probs + path_bias
                path_probs = torch.tensor(adjusted_probs, dtype=torch.float32)
                path_probs = torch.softmax(path_probs, dim=-1)

                prob_mean = path_probs.mean().item()
                actions[src][dst] = path_probs.tolist()
        value = self.critic(features, edge_index)
        return actions, prob_mean, value

class MyGeneralAgent(nn.Module):
    def __init__(self,in_shape,hidden_d=100, share_backbone=False,rnd_hidden_dim=128,num_heads=32):
        super(MyGeneralAgent, self).__init__()
        self.in_shape = in_shape
        self.hidden_d = hidden_d
        self.share_backbone = share_backbone 
        self.num_heads = num_heads

        in_dim = np.prod(in_shape)
    
        def to_features():
            return nn.Sequential(
                nn.Flatten(),
                nn.Linear(in_dim, hidden_d),
                nn.ReLU(),
                nn.Linear(hidden_d, hidden_d),
                nn.ReLU()
            )
        
        self.backbone = to_features() if self.share_backbone else nn.Identity()
        self.critic = Critic(state_dim=in_shape, gat_hidden_dim=hidden_d, gat_output_dim=hidden_d, num_heads=num_heads)

    def forward(self, x, edge_index):
        features = self.backbone(x)
        value = self.critic(features, edge_index)
        return  value

def training_loop(env, model_topo, model_route,model_general,args,ReplyBuffer, max_iterations, n_actors, 
                   horizon,lamda, gamma, epsilon, n_epochs, batch_size, lr,c1, c2, c3,device,seed):
 
    """Train the model using multiple actors over a fixed horizon."""

    max_reward = float("-inf")
    optimizer_topo = Adam(model_topo.parameters(), lr=lr )
    optimizer_route = Adam(model_route.parameters(),lr=lr ) 
    optimizer_topo_rnd = Adam(model_topo.rnd.parameters(),lr)
    optimizer_route_rnd=Adam(model_route.rnd.parameters(),lr)

    scheduler_topo = LinearLR(optimizer_topo, 1, 0, max_iterations * n_epochs)
    scheduler_route = LinearLR(optimizer_route, 1, 0, max_iterations * n_epochs)
    scheduler_topo_rnd = LinearLR(optimizer_topo_rnd, 1, 0, max_iterations * n_epochs)
    scheduler_route_rnd = LinearLR(optimizer_topo_rnd, 1, 0, max_iterations * n_epochs)
    ReplyBuffer.reset_buffer()
    while ReplyBuffer.episode_num < 16:
                run_timestamps( env, model_topo, model_route,model_general, args, ReplyBuffer, 
                    horizon, device)
    for iteration in range(max_iterations):
        start_time = time.perf_counter()
        for actor in range(n_actors):
            
            run_timestamps( env, model_topo, model_route,model_general, args, ReplyBuffer,
                                        horizon, device)  
            for epoch in range(n_epochs):
                
                batch_data = ReplyBuffer.get_training_data()
                topo_state_batch = batch_data['obs_topo']
                route_state_batch = batch_data['obs_route']

                topo_probs_batch = batch_data['a_prob_topo']

                topo_values_batch = batch_data['v_topo']
                topo_rewards_batch = batch_data['r_topo']

                route_actions_batch = batch_data['a_route']
                route_probs_batch = batch_data['a_prob_route']

                route_values_batch = batch_data['v_route']
                route_rewards_batch = batch_data['r_route']
                done_batch = batch_data['done_n']

                general_values_batch = batch_data['v_general']
                general_rewards_batch = batch_data['r_general']

                optimizer_topo.zero_grad()
                optimizer_topo_rnd.zero_grad()
                
                rnd_loss_topo = model_topo.rnd.calculate_rnd_loss(topo_state_batch)
               
                topo_prob=topo_probs_batch
                topo_combined_reward = topo_rewards_batch - lamda*rnd_loss_topo
               
                topo_values_batch_mean=topo_values_batch.mean()
                topo_values_batch_std=topo_values_batch.std()
                topo_values_batch=(topo_values_batch-topo_values_batch_mean)/(topo_values_batch_std+1e-6)
                topo_rewards_batch_mean=topo_rewards_batch.mean()
                topo_rewards_batch_std=topo_rewards_batch.std()
                topo_rewards_batch=(topo_rewards_batch-topo_rewards_batch_mean)/topo_rewards_batch_std
                topo_value_loss = F.mse_loss(topo_values_batch, topo_rewards_batch)
                topo_advantage = topo_combined_reward + (1 - done_batch) * gamma *topo_value_loss - topo_values_batch
                topo_ratio = (topo_prob - topo_prob.detach()).exp()
                topo_ratio=torch.mean(topo_ratio,dim=-1)
                topo_surrogate_loss = topo_ratio * topo_advantage.detach()
                topo_policy_loss = -torch.mean(torch.min(topo_surrogate_loss, torch.clamp(topo_ratio, 1 - epsilon, 1 + epsilon) * topo_advantage.detach()))
                topo_loss = topo_policy_loss - c3* rnd_loss_topo

                topo_combined_reward =topo_combined_reward.mean()
                write_to_file("topo_combined_reward.txt", f"topo_combined_reward: {topo_combined_reward}")
                topo_advantage=topo_advantage.mean()
                write_to_file("topo_advantage.txt", f"topo_advantage: {topo_advantage}")
                write_to_file("topo_policy_loss.txt", f"topo_policy_loss: {topo_policy_loss}")
                write_to_file("topo_loss.txt", f"topo_loss: {topo_loss}")
                topo_loss.backward(retain_graph=True)
                rnd_loss_topo.backward()
                optimizer_topo.step()
                optimizer_topo_rnd.step()
                

                optimizer_route.zero_grad()
                optimizer_route_rnd.zero_grad()
                rnd_loss_route = model_route.rnd.calculate_rnd_loss(route_state_batch)

               
                route_prob=route_probs_batch
                route_combined_reward = route_rewards_batch - lamda * rnd_loss_route
                baseline=c2
                beta=c1
                baseline=beta * baseline + (1 - beta) * route_combined_reward
                route_value_loss = F.mse_loss(route_values_batch, route_rewards_batch)
              
                route_advantage = route_combined_reward - baseline  + (1 - done_batch) * gamma * route_value_loss - route_values_batch
                
                route_ratio =(route_prob - route_prob.detach()).exp()
                route_ratio =route_ratio.mean(dim=1,keepdim=True)
                
                route_surrogate_loss = route_ratio * route_advantage.detach()
                route_policy_loss = -torch.mean(torch.min(route_surrogate_loss, torch.clamp(route_ratio, 1 - epsilon, 1 + epsilon) * route_advantage.detach()))
                route_loss = route_policy_loss - c3* rnd_loss_route

                route_combined_reward=route_combined_reward.mean()
                write_to_file("route_combined_reward.txt", f"route_combined_reward: {route_combined_reward}")
                route_advantage=route_advantage.mean()
                write_to_file("route_advantage.txt", f"route_advantage: {route_advantage}")
                write_to_file("route_policy_loss.txt", f"route_policy_loss: {route_policy_loss}")
                write_to_file("route_loss.txt", f"route_loss: {route_loss}")
                route_loss.backward(retain_graph=True)
                rnd_loss_route.backward()
                optimizer_route.step() 
                optimizer_route_rnd.step()

                general_value_loss = ( F.mse_loss(general_values_batch,general_rewards_batch))
                write_to_file("general_value_loss.txt",f"general_value_loss:{general_value_loss}")

            scheduler_topo.step()
            scheduler_route.step()
            scheduler_topo_rnd.step()
            scheduler_route_rnd.step()

        iteration_time = time.perf_counter() - start_time
        write_to_file("iteration_time.txt",f"{iteration},{iteration_time:.4f}\n")
        avg_reward_topo = np.mean(topo_combined_reward.detach().cpu().numpy())
        avg_reward_route = np.mean(route_combined_reward.detach().cpu().numpy())
        avg_reward_general = np.mean(general_rewards_batch.detach().cpu().numpy())

        if avg_reward_topo > max_reward:
            max_reward = avg_reward_topo
            torch.save(model_topo.state_dict(), "topo_agent_model.pth")

        if avg_reward_route > max_reward:
            max_reward = avg_reward_route
            torch.save(model_route.state_dict(), "route_agent_model.pth")
        if avg_reward_general > max_reward:
            max_reward = avg_reward_general
            torch.save(model_general.state_dict(), "general_agent_model.pth")
        
        write_to_file("avg_reward_topo.txt", f"avg_reward_topo: {avg_reward_topo}")
        write_to_file("avg_reward_route.txt", f"avg_reward_route: {avg_reward_route}")
        write_to_file("max_topo_reward.txt", f"max_topo_reward: {max_reward}")
        write_to_file("max_route_reward.txt", f"max_route_reward: {max_reward}")
        write_to_file("avg_reward_general.txt", f"avg_reward_general: {avg_reward_general}")
        write_to_file("max_general_reward.txt", f"max_general_reward: {max_reward}")
    return model_topo, model_route,model_general
                

def testing_loop(env, model_topo, model_route,model_general,args, ReplyBuffer,n_episodes,horizon, device):
    for _ in range(n_episodes):
        run_timestamps( env, model_topo, model_route,model_general, args, ReplyBuffer, 
                           horizon, device=device)


