import numpy as np
import matplotlib.pyplot as plt
means = []
ev_num = []
treshold = 160
for i in range(2000):
    if i < 10:
        data = np.loadtxt("image0000" + str(i) + ".txt")
    elif i >= 10 and i < 100:
        data = np.loadtxt("image000" + str(i) + ".txt")
    elif i >= 100 and i < 1000:
        data = np.loadtxt("image00" + str(i) + ".txt")
    elif i >= 1000:
        data = np.loadtxt("image0" + str(i) + ".txt")
    else:
        print("Sono morto")
    centers = np.argwhere(data > treshold)
    if len(centers) > 0:
    
        events = []
        for event in centers:
            if(event[0] - 2 > 0 and event[0]+3 < 3008 and event[1] - 2 > 0 and event[1] + 2 < 3008):
                v1 = data[(event[0]-2):(event[0]+3),(event[1] - 2)]
                v2 = data[(event[0]-2):(event[0]+3),(event[1] - 1)]
                v3 = data[(event[0]-2):(event[0]+3),(event[1])]
                v4 = data[(event[0]-2):(event[0]+3),(event[1] + 1)]
                v5 = data[(event[0]-2):(event[0]+3),(event[1] + 2)]
                val = np.vstack([v1,v2,v3,v4,v5])
                events.append(val)

        j = 0
        for element in events:
            plt.matshow(element)
            plt.savefig("./eventi/event" + str(j) + "image" + str(i) + ".png")
            
            with open('./eventi/event' + str(j) + "image" + str(i) + ".txt", 'a') as eve:
                eve.write(str(element))
                eve.close
            j += 1
        with open("./eventi/image" + str(i) + ".txt", 'a') as list:
            list.write("Number of events:" + str(j)+ "\n")
            list.write("Positions: \n")
            list.write(str(centers))
            list.close
        ev_num.append(j)
    means.append(np.mean(data.flatten()))
    print("\n Done:" + str(i*100/2000)+ "%")
with open('Acquisition_report.txt', 'a') as f:
    f.write("\n Number of detected events:" + str(np.sum(ev_num)))
    f.write("\n Using threshold:" + str(treshold))
    f.write("\n Average Pixel Value:" + str(np.mean(means)))

