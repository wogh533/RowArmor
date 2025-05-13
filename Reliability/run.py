from multiprocessing import Pool
import sys
import os
import time

oecc = [1]
fault = [0, 1, 2, 3, 4, 5, 6, 7, 8]
recc = [1, 2, 3] 


for oecc_param in oecc:
    for fault_param in fault:        
        for recc_param in recc:
            os.system("./Fault_sim_start {0:d} {1:d} {2:d} &".format(oecc_param, fault_param, recc_param))