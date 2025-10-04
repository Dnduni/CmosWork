import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
from tqdm import tqdm

#Define relevant arrays and constants
means = []
ev_num = []
treshold = 200
file_num = 100
#Set directories up for analysis to keep everything tidy

Path("./eventi").mkdir(parents = True, exist_ok= True)
Path("./eventi/images").mkdir(parents=True, exist_ok=True)
Path("./eventi/event_logs").mkdir(parents = True, exist_ok = True)


#load files
for i in tqdm(range(file_num)):
    if i < 10:
        data = np.loadtxt("image0000" + str(i) + ".txt")
    elif i >= 10 and i < 100:
        data = np.loadtxt("image000" + str(i) + ".txt")
    elif i >= 100 and i < 1000:
        data = np.loadtxt("image00" + str(i) + ".txt")
    elif i >= 1000:
        data = np.loadtxt("image0" + str(i) + ".txt")
    else:
        print("Sono morto") #funny

    #Get seeds     
    centers = np.argwhere(data > treshold)
    #if events are detected paint a 5x5 matrix around them
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
        #heatmap each element and add it to the detected events file for that image
        for element in events:
            with open("./eventi/event_logs/events_image" + str(i) + ".txt", 'a') as eve:
                eve.write("Event number:" + str(j) + "\n")
                eve.write(str(element) + "\n")
                eve.close
            j += 1
        #write out to file the center of the event
        with open("./eventi/event_list.txt", 'a') as list:
            for centro in centers:
                list.write(str(centro[0]) + "\t" + str(centro[1]) + "\n")
        ev_num.append(j)
    means.append(np.mean(data.flatten()))
    #print out to file information on the detected events, threshold used and average pixel value
with open('Acquisition_report.txt', 'a') as f:
    f.write("\n Number of detected events:" + str(np.sum(ev_num)))
    f.write("\n Using threshold:" + str(treshold))
    f.write("\n Average Pixel Value:" + str(np.mean(means)))

