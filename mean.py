import numpy as np
import pandas as pd


data = pd.read_csv('image0.txt', sep=' ', header = None)
print(data)
i = 0 
a = data.sum()
print(int(a.sum()/(len(data)**2)))

