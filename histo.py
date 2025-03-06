import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
data = []

for i in range(20):
    data.append(pd.read_csv("image" + str(i) + ".txt", sep = ' ', header = None))
    print("\n Done \t" + str(i) + '\n')
data = np.array(data)
sum = 0
for i in range(len(data)):
    sum += data[i]
sum = sum[:,:3008]
mean = sum / (i+1)
mean = mean.flatten()
print(mean.shape)
plt.hist(mean)
plt.show()