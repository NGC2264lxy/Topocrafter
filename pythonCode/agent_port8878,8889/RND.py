from torch.nn import init
import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F

class Flatten(nn.Module):
    def forward(self, input):
        return input.view(input.size(0), -1)

class RNDModel(nn.Module):
    def __init__(self,device, rnd_input_size, rnd_output_size):
        super(RNDModel, self).__init__()

        self.input_size = rnd_input_size
        self.output_size = rnd_output_size
        self.device = device

        self.predictor = nn.Sequential(
            nn.Linear(self.input_size, 512),
            nn.ReLU(),
            nn.Linear(512, 512),
            nn.ReLU(),
            nn.Linear(512, 512)
        ).to(device)

        self.target = nn.Sequential(
            nn.Linear(self.input_size, 512),
            nn.ReLU(),
            nn.Linear(512, 512)
        ).to(device)

        for m in self.modules():
            if isinstance(m, nn.Conv2d) or isinstance(m, nn.Linear):
                init.orthogonal_(m.weight, np.sqrt(2))
                m.bias.data.zero_()

        for param in self.target.parameters():
            param.requires_grad = False

    def forward(self, next_obs):
        next_obs=next_obs.to(self.device)
        if next_obs.dim() == 4:
            next_obs = next_obs.view(next_obs.size(0), next_obs.size(1), -1)
        elif next_obs.dim() == 3:
            next_obs = next_obs.view(next_obs.size(0), -1)
        
        target_feature = self.target(next_obs)
        predict_feature = self.predictor(next_obs)

        return predict_feature, target_feature
    
    def calculate_rnd_loss(self, next_obs):
        next_obs = next_obs.to(self.device)
        predict_feature, target_feature = self.forward(next_obs)
        rnd_loss = F.mse_loss(predict_feature, target_feature, reduction='mean')
        return rnd_loss
    
    def update_target(self, tau=0.001):
        for target_param, predictor_param in zip(self.target.parameters(), self.predictor.parameters()):
            target_param.data.copy_(tau * predictor_param.data + (1.0 - tau) * target_param.data)
