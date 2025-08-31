import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from dataclasses import dataclass
from tqdm import tqdm
################################INPUT##########################
#Da aggiungere argparse
print("\n Type cluster file address:\n")
cluster_add = input()
try:
    cluster_file = open(cluster_add)
except:
    print("\n Error, cluster file specified does not exist \n")

clusters = pd.read_csv(cluster_file,sep = r'\s+', names = [ "x" , "y" , "val" , "frame"])
clusters = clusters.to_numpy()
print(clusters)
print("\n Type data directory:\n")
data_dir = input()

@dataclass
class cluster_seed: #definizione oggetto cluster (seed del cluster)
    
    x : int = 0
    y : int = 0
    val : int = 0
    frame : int = 0

###########################DATA READING########################

seeds = [] #array of cluster seeds


for element in clusters:
    seed = cluster_seed()
    seed.x = element[0]
    seed.y = element[1]
    seed.val = element[2]
    seed.frame = element[3]
    seeds.append(seed)

max_frame_seed = max(seeds , key = lambda y: y.frame) #determina frame massimo


cluster_buffer = np.zeros((7,7), dtype = int) #inizializza un buffer per il cluster


for curr_frame in tqdm(range(max_frame_seed.frame)):

    if curr_frame < 10 : #naming scheme: image + #frame + .txt
        dir = str(data_dir) + '/image0000' + str(curr_frame) + '.txt'
    elif curr_frame >= 10 and curr_frame < 100 :
        dir = str(data_dir) + '/image000' + str(curr_frame) + '.txt'
    elif curr_frame >= 100 and curr_frame < 1000 :
        dir = str(data_dir) + '/image00' + str(curr_frame) + '.txt'
    elif curr_frame >= 1000:
        dir = str(data_dir) + '/image0' + str(curr_frame) + '.txt'

    try:
        frame = np.loadtxt(dir) #provo a leggere il frame
    except:
        print("Invalid data directory, use only folder name")
        quit()

    a = 0
    b = 0
    for element in seeds: 
        if element.frame == curr_frame:
            a = 0
            for i in range(element.x -3,element.x +3):
                b = 0
                for j in range(element.y - 3, element.y +3):
                    cluster_buffer[a][b] += frame[i][j]
                    b += 1
                a += 1
cluster_buffer = np.divide(cluster_buffer, len(seeds)) #Valore medio di un cluster
plt.imshow(cluster_buffer)
plt.show()       