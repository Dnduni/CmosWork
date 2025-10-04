import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import os
import argparse
from tqdm import tqdm


#Parser perchè mettere ogni volta la directory a mano non mi va
parser = argparse.ArgumentParser("Histogram Active Pixel in cluster")
parser.add_argument("cluster_dir", help="Specifica la directory che contiene i cluster da graficare", type=str)
parser.add_argument("out_img", help="Nome del file immagine in output, senza estensione", type = str)
args = parser.parse_args()
print("Selected directory: " + args.cluster_dir + "\n")
print("Selected filename: " + args.out_img + "\n")
address = args.cluster_dir
os.chdir(address) #Change working directory

non_zero = [] #inizializzo array non zero

for filename in tqdm(os.listdir(os.getcwd())): #Leggo cluster e conto per ciascuno quanti pixel sono diversi da 0
    with open(os.path.join(os.getcwd(),filename), 'r') as f:
        cluster = np.loadtxt(f)
        val = np.count_nonzero(cluster)
        non_zero.append(val) #Li metto nel mio vettore da istogrammare
plt.xlabel("# pixel attivi nel cluster")
plt.ylabel("Numero di cluster")
plt.title("Numero di pixel attivi nei cluster")
plt.hist(non_zero, bins=list(range(49)))
plt.xlim(0,25)
plt.savefig("../" + args.out_img)
plt.show()

