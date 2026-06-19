import torch
import torch.nn as nn
import torch.optim as optim
import torch.nn.functional as F
from torch_geometric.nn import GATv2Conv
from scipy.special import kl_div

def orthogonal_init(layer, gain=1.0):
    for name, param in layer.named_parameters():
        if 'bias' in name:
            nn.init.constant_(param, 0)
        elif 'weight' in name:
            nn.init.orthogonal_(param, gain=gain)


num_heads = 6
gat_hidden_dim = 128
gat_output_dim = 4
state_dim = 256

class GATNet(nn.Module):
    def __init__(self, input_dim, hidden_dim, output_dim, heads):
        super(GATNet, self).__init__()
        self.gat1 = GATv2Conv(input_dim, hidden_dim, heads=heads, concat=True)
        self.gat2 = GATv2Conv(hidden_dim * heads, output_dim, heads=1, concat=True)
        
    def forward(self, x, edge_index):
        x = self.gat1(x, edge_index)
        x = F.relu(x)
        x = self.gat2(x, edge_index)
        return x
    
class Actor(nn.Module):
    def __init__(self, actor_input_dim):
        super(Actor, self).__init__()
        self.gat_net = GATNet(input_dim=state_dim, hidden_dim=gat_hidden_dim, output_dim=gat_output_dim, heads=num_heads)

    def forward(self, x):
        x = self.gat_net(x)
        prob = torch.softmax(x, dim=-1)
        return prob
    
class Critic(nn.Module):
    def __init__(self, actor_input_dim):
        super(Critic, self).__init__()
        self.gat_net = GATNet(input_dim=state_dim, hidden_dim=gat_hidden_dim, output_dim=gat_output_dim, heads=num_heads)
        self.fc = nn.Linear(gat_hidden_dim, 1)

    def forward(self, x):
        value = self.fc(x)
        return value

class CentCritic(nn.Module):
    def __init__(self, GATNet, CC_dim):
        super(CentCritic, self).__init__()
        self.GATNet = GATNet
        self.critic = nn.Linear(CC_dim, 1)

    def forward(self, x):
        x = self.GATNet(x)
        value = self.critic(x)
        return value

class TopoAgent(nn.Module):
    def __init__(self):
        super(TopoAgent, self).__init__()
