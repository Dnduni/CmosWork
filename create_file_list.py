stri = "./2025_05_12_X_Sn_35kV_100uA_1sTI_200Gain_0TB_bis/"
with open("filelistsn.txt", "a") as f:
    for i in range(1000):
        if i < 10:
            f.write(stri + "image0000" + str(i) + ".txt\n")
        if(i > 9 and i < 100):
            f.write(stri + "image000" + str(i) + ".txt\n")
        if(i > 99 and i < 1000):
            f.write( stri + "image00" + str(i) + ".txt\n")
        if i > 1000:
            f.write( stri + "image0" + str(i) + ".txt\n")