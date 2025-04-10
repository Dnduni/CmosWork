import numpy as np
import matplotlib.pyplot as plt
centers = np.loadtxt("event_list.txt")

x = []
y = []
for element in centers:
    x.append(element[0])
    y.append(element[1])
h = plt.hist2d(x,y,bins = len(centers))
centers_tuple = [tuple(x) for x in centers]
unique_cen = sorted(set(centers_tuple), key=lambda x: centers_tuple.index(x))
unique_count = [centers_tuple.count(x) for x in unique_cen]
unique_index = [centers_tuple.index(x) for x in unique_cen]

for i in range(len(unique_cen)):
    with open("1histo.txt", 'a') as f:
        f.write(str(unique_cen[i][0]) + "\t" + str(unique_cen[i][1]) + "\t" + str(unique_count[i]) + "\n")

plt.show()
