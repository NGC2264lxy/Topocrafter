import numpy as np
import torch
import socket
import time
from collections import deque
from gym import spaces
class HPCEnvironment:
    def __init__(self, args):
        self.num_pods = args.num_pods
        self.complete_flag = False
        self.cur_traffic = []
        self.cur_configuration = []
        self.cur_PathLength = 0
        self.matrix_size = self.num_pods
        self.selected_matrix = np.zeros((self.matrix_size, self.matrix_size), dtype=int)
        self.ports_per_mems = 31
        self.initopo = []
        self.initopoFlag = True

        self.routing_table = {
            src: {dst: [] for dst in range(self.num_pods) if dst != src}
            for src in range(self.num_pods)
        }

        self.link_utilization = []
        self.link_load = []
        self.link_state = [[0 for _ in range (32)] for _ in range (32)]
        
        self.matrix_size=self.num_pods
        self.topology_action_space = spaces.MultiDiscrete([31] * self.matrix_size)
        self.topology_observation_space = spaces.Box(low=-1, high=self.matrix_size - 1, shape=(self.matrix_size,self.matrix_size), dtype=np.int32)
        
        self.route_action_space = spaces.Dict({
            src: spaces.Dict({
                dst: spaces.Box(low=0.0, high=1.0, shape=(len(paths),), dtype=np.float32)  for dst, paths in self.routing_table[src].items()
                
            })
            for src in range(len(self.selected_matrix))
        })
        self.route_observation_space = spaces.Box(low=0, high=1, shape=(32, 32))

        self.topo_address = ('127.0.0.1', 8889)
        self.topo_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.topo_server.bind(self.topo_address)

        self.route_address = ('127.0.0.1', 8878)
        self.route_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.route_server.bind(self.route_address)


    def get_cur_comFlag(self):
        comFlag = self.complete_flag
        return comFlag

    def get_cur_configuration(self):
        configuration = self.cur_configuration
        return configuration

    def get_cur_traffic(self):
        traffic = self.cur_traffic
        return traffic
  
    def get_cur_state(self):
        traffic=self.get_cur_traffic()
        link_utilization=[[element * 1000 * 256 * 8 / (100 * (10 ** 9)) for element in row] for row in traffic]
        self.link_utilization=link_utilization
        link_load=[[element * 1000 * 256 * 8 / (10 * (10 ** -3)) for element in row] for row in traffic]
        load_min=np.min(link_load)
        load_max=np.max(link_load)
        link_load=(link_load-load_min)/(load_max-load_min)
        self.link_load=link_load

        link_state=np.array( self.link_state)
        return  link_utilization,link_load,link_state

    def handle_recvData(self, recvData):
            parts = recvData.split(';')
            complete_flag = parts[0]
            self.msgkind = parts[1]
            if self.msgkind=='elsemsg':
                traffic_matrix = []
                configuration = []
                link_state= []
                self.traffic_demand = []  
                for i in range(2,self.num_pods+2):
                    self.traffic_demand.append(list(map(int, parts[i].split(','))))
                for i in range(2*self.num_pods+2, 3*self.num_pods+2):
                    traffic_matrix.append(list(map(int, parts[i].split(','))))
                for i in range(3*self.num_pods+2, 4*self.num_pods+2):
                    configuration.append(list(map(int, parts[i].split(','))))
                for i in range(4*self.num_pods+2,5*self.num_pods+2):
                    link_state.append(list(map(int, parts[i].split(','))))
                self.complete_flag = complete_flag
                self.cur_traffic = traffic_matrix
                self.cur_configuration = configuration
                if self.initopoFlag:
                    self.initopo = configuration
                    self.initopoFlag = False
                self.cur_PathLength = float(parts[-1])
                for mems_idx in range(len(link_state)):
                    for src_pod in range(len(link_state[mems_idx])):
                        if link_state[mems_idx][src_pod]>0:
                            dst_pod=self.selected_matrix[mems_idx][src_pod]
                            self.link_state[src_pod][dst_pod]+=1

            elif self.msgkind=='topomsg':
                configuration=[]
                for i in range(2, self.num_pods+2):
                    configuration.append(list(map(int, parts[i].split(','))))
                self.cur_configuration=configuration
            else:
                print(f"error msgkind from omnetpp")
        
    def get_cur_topology(self):
        num_groups = len(self.cur_configuration[0])
        adjacency_matrix = np.zeros((num_groups, num_groups), dtype=int)
        for groups in self.cur_configuration:
            for group_id in groups:
                if group_id != -1:
                    for other_group_id in groups:
                        if other_group_id != -1 and other_group_id != group_id:
                            adjacency_matrix[group_id][other_group_id] = 1
        return adjacency_matrix
    
    def get_edge_index(self):
        topology = self.get_cur_topology()
        edge_index = np.argwhere(topology == 1)
        return edge_index.T

    def get_edge_index2(self,link_state):
        edge_index = np.argwhere(link_state == 0)
        return edge_index.T

    def cal_cur_reward(self):
        lu=self.get_cur_state()[0]
       
        for i in range(len(lu)):
          lu[i][i] = 0.0
        filtered_matrix = [lu[i][j] for i in range(len(lu)) for j in range(len(lu[i])) if i != j and lu[i][j] != 0.0]
        lu_mean = np.mean(filtered_matrix)
        sum_squared_diff = np.sum((filtered_matrix - lu_mean)**2)
        num_elements = len(filtered_matrix)
        lu_diff = np.sqrt(sum_squared_diff / num_elements)

        lu_reward = np.exp(-lu_diff)
        path_reward = np.exp(-(self.cur_PathLength-1))
        total_reward = np.exp(-lu_diff - (self.cur_PathLength-1))
        return lu_reward, path_reward, total_reward
    
    def reset(self):
        self.topo_server.listen(5)
        print("Initialize:waiting for Topo Agent connection...")
        self.topo_client, self.topo_addr = self.topo_server.accept()
        print(f"Topo Agent accepted connection from {self.topo_addr}")
        data = self.topo_client.recv(65536).decode()
        self.handle_recvData(data)
        self.complete_flag = False
        _,link_load,link_state=self.get_cur_state()
        state = (link_state, link_load)
        return state
        
    def step(self, action_prob=None, agent_id=None,threshold=None):
        done = self.get_cur_comFlag()
        if agent_id==1:
          
            self.selected_matrix=np.copy(self.cur_configuration)
            detect_reward=np.zeros((self.num_pods,self.num_pods))
           
            optimized_linky=self.selected_matrix.copy()
            for src_pod in range (self.num_pods):
                for mems_idx in range (self.num_pods):
                    dst_pod =optimized_linky[mems_idx,src_pod]
                    if dst_pod == -1:
                        continue

                    if action_prob [src_pod,dst_pod]>= threshold:
                        traffic_demand = np.array(self.traffic_demand)
                        demands = [(j, traffic_demand[src_pod, optimized_linky[j, src_pod]]) for j in range(self.num_pods) if optimized_linky[j, src_pod] != -1]
                        demands.sort(key=lambda x: x[1], reverse=True)
                        for swap_mems, _ in demands:
                            if swap_mems == mems_idx:
                                continue
                            if traffic_demand[src_pod, dst_pod] > traffic_demand[src_pod, optimized_linky[swap_mems, src_pod]]:
                                dst1=optimized_linky[mems_idx, src_pod]
                                dst2=optimized_linky[swap_mems, src_pod]

                                dst3=optimized_linky[mems_idx, dst2]
                                dst4=optimized_linky[swap_mems, dst1]
                                optimized_linky[mems_idx,src_pod]=dst2
                                optimized_linky[swap_mems, src_pod]=dst1
                                
                                optimized_linky[mems_idx,dst2]=src_pod
                                optimized_linky[swap_mems,dst1] = src_pod

                                optimized_linky[mems_idx,dst3]=dst1
                                optimized_linky[mems_idx,dst1]=dst3
                                optimized_linky[swap_mems,dst4]=dst2
                                optimized_linky[swap_mems,dst2]=dst4
                                self.link_state[src_pod][dst1]=0
                                self.link_state[src_pod][dst2]+=1

                                break
                    else:
                        continue
            self.selected_matrix=optimized_linky 
            for src_pod in range(self.num_pods):
                for dst_pod in range(self.num_pods):
                    action_probability = action_prob[src_pod,dst_pod]
                    link_status = self.link_state[src_pod] [dst_pod]
                    difference = link_status-action_probability 
                    detect_reward[src_pod][dst_pod] =  -difference*difference
            self.selected_matrix=optimized_linky

            if not self.check_invalid_action_topo():
                return self.selected_matrix
            topo_str =  self.matrix_to_string(self.selected_matrix)
            topo_str ="topomsg;"+ topo_str
            self.send_to_omnetpp(topo_str,self.topo_client)
            self.topo_client.close()
            topo_reward,_,_ = self.cal_cur_reward()
            topo_reward=np.mean(topo_reward)

            self.route_server.listen(5)
            self.route_client, self.route_addr = self.route_server.accept()
            msgfrom_omnetpp = self.route_client.recv(65536).decode()
            self.handle_recvData(msgfrom_omnetpp)
            routing_table=self.generate_routing_table()
            return topo_reward,self.selected_matrix,routing_table

        elif agent_id==2:
            self.generate_routing_table2()
            routing_info = []
            routing_dict = {}
            for src, dst, paths,flow in self.routing_table:
                if src not in routing_dict:
                    routing_dict[src] = {}
                routing_dict[src][dst] = paths
            for src, dst_action in action_prob.items():
                for dst, path_probs in dst_action.items():
                    if path_probs is None:
                        continue
                    paths = routing_dict.get(src, {}).get(dst, [])
                    if not paths:
                        continue
                    for path, prob in zip(paths, path_probs):
                        path_str = '-'.join(map(str, path))
                        routing_info.append(f"{src},{dst},{path_str},{prob:.4f}")

          
            routing_str = ";".join(routing_info)
            routing_str="routemsg;"+routing_str
            self.send_to_omnetpp(routing_str,self.route_client)

            _, route_reward,_ = self.cal_cur_reward()
            return route_reward,done
        elif agent_id==3:
            state=self.get_cur_state()
            total_reward=self.cal_cur_reward()
            link_state=state[2]
            num_fault_links = (link_state == 1).sum().item()
            reward = -0.5 * num_fault_links + 0.5 * total_reward[2]
            return reward,done

    def generate_routing_table(self):
        """
        Plan optimal paths for inter-POD traffic based on traffic_demand and selected_matrix.
        Supports direct and single-hop relay paths with identifiers for extra relay hops.

        Returns:
            list: Routing table entries as (src, dst, paths, flow) tuples.
        """
        num_pods = self.num_pods
        num_mems = self.selected_matrix.shape[0]

        path_plan = []
        traffic_demand = np.array(self.traffic_demand)
        flows = [(src, dst, traffic_demand[src][dst])
                for src in range(num_pods)
                for dst in range(num_pods) if traffic_demand[src][dst]>0]
        flows.sort(key=lambda x: -x[2])

        for src, dst, flow in flows:
            direct_paths = []
            for mem in range(num_mems):
                if self.selected_matrix[mem][src] == dst and self.link_state[src][dst] == 0:
                    direct_paths.append(([src, dst], True))

            paths = direct_paths[:]

            possible_paths = []
            for mem1 in range(num_mems):
                mid_pod = self.selected_matrix[mem1][src]
                if mid_pod == -1 or mid_pod == src:
                    continue
                for mem2 in range(num_mems):
                    final_pod = self.selected_matrix[mem2][mid_pod]
                    if final_pod == dst and self.link_state[mid_pod][dst] == 0:
                        requires_extra_hop = not self.is_same_switch(mem1, mem2)
                        path = [src, mid_pod, dst]
                        load = (self.link_load[src][mid_pod] +
                                self.link_load[mid_pod][dst])
                        possible_paths.append((path, load, not requires_extra_hop))

            possible_paths.sort(key=lambda x: x[1])

            if len(paths) < 32:
                for i in range(32 - len(paths)):
                    if i < len(possible_paths):
                        path, load, direct_flag = possible_paths[i]
                        paths.append((path, direct_flag))

            path_plan.append((src, dst, paths, flow))

            for path, direct_flag in paths:
                for i in range(len(path) - 1):
                    self.link_load[path[i]][path[i + 1]] += flow
        
        return path_plan

    def is_same_switch(self, mem1, mem2):
        switch_groups = {
                0: [0, 16],
                1: [1, 17],
                2: [2, 18],
                3: [3, 19],
                4: [4, 20],
                5: [5, 21],
                6: [6, 22],
                7: [7, 23],
                8: [8, 24],
                9: [9, 25],
                10: [10, 26],
                11: [11, 27],
                12: [12, 28],
                13: [13, 29],
                14: [14, 30],
                15: [15, 31]

        }
        for group in switch_groups.values():
            if mem1 in group and mem2 in group:
                return True
        return False

    def generate_routing_table2(self):
        """
        Build routing table for OMNeT++ communication.
        Supports direct and single-hop relay paths.

        Returns:
            list: Routing table entries as (src, dst, paths, flow) tuples.
        """
        num_pods = self.num_pods
        num_mems = self.selected_matrix.shape[0]

        path_plan = []
        traffic_demand = np.array(self.traffic_demand)

        flows = [(src, dst, traffic_demand[src][dst])
                 for src in range(num_pods)
                 for dst in range(num_pods)if traffic_demand[src][dst]>0 ]
        flows.sort(key=lambda x: -x[2])

        for src, dst, flow in flows:
            direct_paths = []
            for mem in range(num_mems):
                if self.selected_matrix[mem][src] == dst and self.link_state[src][dst] == 0:
                    direct_paths.append([src, dst])

            paths = direct_paths[:]
            possible_paths = []
            for mem1 in range(num_mems):
                mid_pod = self.selected_matrix[mem1][src]
                if mid_pod == -1 or mid_pod == src:
                    continue
                for mem2 in range(num_mems):
                    final_pod = self.selected_matrix[mem2][mid_pod]
                    if final_pod == dst and self.link_state[mid_pod][dst] == 0:
                        path = [src, mid_pod, dst]
                        load = (self.link_load[src][mid_pod] +
                                self.link_load[mid_pod][dst])
                        possible_paths.append((path, load))

            possible_paths.sort(key=lambda x: x[1])
            if len(paths) < 32:
                for i in range(32 - len(paths)):
                    if i < len(possible_paths):
                        paths.append(possible_paths[i][0])

            path_plan.append((src, dst, paths, flow))

            for path in paths:
                for i in range(len(path) - 1):
                    self.link_load[path[i]][path[i + 1]] += flow

        self.routing_table = path_plan

    def close(self):
        """Close socket connections."""
        self.server.close()
        print("Socket connection closed.")

    def send_to_omnetpp(self, data,socket_connection):
        """Send serialized data to OMNeT++ over socket."""
        socket_connection.sendall(data.encode())

    def matrix_to_string(self, matrix):
        """Convert selected_matrix to a string for socket transmission."""
        return ';'.join([','.join(map(str, row)) for row in matrix])

    def check_invalid_action_topo(self):
        valid = True
        visited = [False] * self.num_pods
        queue = deque([0])
        visited[0] = True

        while queue:
            src = queue.popleft()
            
            for mid_node in range(self.selected_matrix.shape[0]):
                dst = self.selected_matrix[mid_node, src]
                if dst != -1 and not visited[dst]:
                    visited[dst] = True
                    queue.append(dst)

        if all(visited):
            print("The graph is connected.")
            valide = True
        else:
            print("The graph is not connected.")
            valide = False
                            
        return valid

    def check_invalid_action_route(self):
        valid = True

        if self.has_deadlock():
            valid = False
        return valid

    def has_deadlock(self):
        """
        Detect cycles in the routing graph using Tarjan's algorithm.
        A cycle indicates potential network deadlock.

        Returns:
            bool: True if a cycle is detected, False otherwise.
        """
        graph = self.get_routing_graph()

        num_nodes = len(graph)
        index = 0
        indices = [-1] * num_nodes
        lowlink = [-1] * num_nodes
        stack = []
        on_stack = [False] * num_nodes
        has_cycle = [False]

        def tarjan(v):
            nonlocal index
            indices[v] = index
            lowlink[v] = index
            index += 1
            stack.append(v)
            on_stack[v] = True

            for w in graph[v]:
                if indices[w] == -1:
                    tarjan(w)
                    lowlink[v] = min(lowlink[v], lowlink[w])
                elif on_stack[w]:
                    lowlink[v] = min(lowlink[v], indices[w])

            if lowlink[v] == indices[v]:
                scc = []
                while True:
                    w = stack.pop()
                    on_stack[w] = False
                    scc.append(w)
                    if w == v:
                        break
                if len(scc) > 1:
                    has_cycle[0] = True

        for v in range(num_nodes):
            if indices[v] == -1:
                tarjan(v)

        return has_cycle[0]

    def get_routing_graph(self):
        """
        Build a POD adjacency list for Tarjan deadlock detection.
        Only direct reachability is considered (relay paths are ignored).

        Returns:
            dict: Adjacency list mapping each POD to reachable PODs.
        """
        num_rows, num_cols = self.selected_matrix.shape
        graph = {src: set() for src in range(num_cols)}

        for src in range(num_cols):
            for intermediate in range(num_rows):
                dst = self.selected_matrix[intermediate][src]
                if dst != -1 and src != dst:
                    graph[src].add(dst)
        return graph


