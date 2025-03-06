import numpy as np
import pandas as pd
import matplotlib.pyplot as plt   
times = [10,50,100,200,500,1000,10000,20000,60000,100000,200000]
ambient = [0,0,0,0,5,9,97,193,254,254,254]
darkbox = [0,0,0,0,0,0,8,16,50,85,170]

fig = plt.plot(times,ambient, label = 'ambient')
fig  = plt.plot(times,darkbox, label = 'dark')
plt.legend()
plt.xlabel("Exposure time (ms)")
plt.ylabel("Pixel value")
plt.savefig("Linearity.png")
plt.show()