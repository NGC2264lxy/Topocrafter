
import numpy as np
import torch

class ReplyBuffer:
    def __init__(self, args):
        self.N = args.N
        self.obs_dim = args.obs_dim
        self.state_dim = args.state_dim
        self.episode_limit = args.episode_limit
        self.batch_size = args.batch_size
        self.limit =16
        self.episode_num = 0
        self.buffer = None
        self.reset_buffer()

    def reset_buffer(self):
        self.buffer = {'obs_topo': np.empty([self.batch_size, self.episode_limit, self.N, self.obs_dim]),
                       'obs_route': np.empty([self.batch_size, self.episode_limit, self.N, self.obs_dim]),
                       'v_topo': np.empty([self.batch_size, self.episode_limit , self.N]),
                       'v_route': np.empty([self.batch_size, self.episode_limit , self.N]),
                       'v_general':np.empty([self.batch_size, self.episode_limit , self.N]),
                       'a_prob_topo': np.empty([self.batch_size, self.episode_limit, self.N,self.N]),
                       'a_prob_route': np.empty([self.batch_size, self.episode_limit, self.N,self.N]),
                       'r_topo': np.empty([self.batch_size, self.episode_limit, self.N]),
                       'r_route': np.empty([self.batch_size, self.episode_limit, self.N]),
                       'r_general': np.empty([self.batch_size, self.episode_limit, self.N]),
                       'rnd_r_topo': np.empty([self.batch_size, self.episode_limit, self.N]),
                       'rnd_r_route': np.empty([self.batch_size, self.episode_limit, self.N]),
                       'done_n': np.empty([self.batch_size, self.episode_limit, self.N])
                       }
                       
        self.buffer['a_route'] = [[] for _ in range(self.batch_size)]
        self.episode_num = 0

    def store_transition(self, episode_step, obs_topo, obs_route, v_topo,v_route,v_general, a_route, a_prob_topo,
                         a_prob_route, r_topo, r_route,r_general, rnd_r_topo, rnd_r_route, done_n):
        self.buffer['obs_topo'][self.episode_num % self.limit][episode_step] = obs_topo
        self.buffer['obs_route'][self.episode_num% self.limit ][episode_step] = obs_route

        self.buffer['v_topo'][self.episode_num% self.limit ][episode_step] = v_topo.cpu().numpy().squeeze()
        self.buffer['v_route'][self.episode_num % self.limit][episode_step] = v_route.cpu().numpy().squeeze()
        self.buffer['v_general'][self.episode_num % self.limit][episode_step] = v_general.cpu().numpy().squeeze()
        formatted_a_route = torch.full((self.N, self.N), 0, dtype=torch.float32)
        for src in a_route:
            for dst in a_route[src]:
                if a_route[src][dst] is None:
                    formatted_a_route[src, dst] = 0
                else:
                    formatted_a_route[src, dst] =  torch.mean(torch.tensor(a_route[src][dst], dtype=torch.float32)).item()

        if len(self.buffer['a_route']) <= self.episode_num:
            self.buffer['a_route'].append([])

        self.buffer['a_route'][self.episode_num].append(formatted_a_route)

        self.buffer['a_prob_topo'][self.episode_num % self.limit][episode_step] = a_prob_topo.cpu().numpy()
        self.buffer['a_prob_route'][self.episode_num % self.limit][episode_step] = a_prob_route
        self.buffer['r_topo'][self.episode_num% self.limit ][episode_step] = r_topo
        self.buffer['r_route'][self.episode_num % self.limit][episode_step] = r_route
        self.buffer['r_general'][self.episode_num % self.limit][episode_step] = r_general
        self.buffer['rnd_r_topo'][self.episode_num % self.limit][episode_step] = rnd_r_topo
        self.buffer['rnd_r_route'][self.episode_num% self.limit ][episode_step] = rnd_r_route
        self.buffer['done_n'][self.episode_num % self.limit ][episode_step] = done_n

    def get_training_data(self):
        batch = {}
        for key in self.buffer.keys():
            if key == 'a_route':
                non_empty_episodes = []
                for episode in self.buffer['a_route']:
                    if episode:
                        processed_episode = []
                        for x in episode:
                            if x is None:
                                processed_episode.append(torch.full((1, self.N), 0, dtype=torch.float32))
                            else:
                                processed_episode.append(torch.tensor(x, dtype=torch.float32))
                        
                        non_empty_episodes.append(torch.stack(processed_episode))
                if non_empty_episodes:
                    batch[key] = torch.stack(non_empty_episodes).to('cuda')
                else:
                    batch[key] = torch.empty((0, self.N, self.N), dtype=torch.long).to('cuda')
            else:
                batch[key] = torch.tensor(self.buffer[key], dtype=torch.float32).to('cuda')
        return batch
        

