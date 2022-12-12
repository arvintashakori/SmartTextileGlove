import pandas as pd
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.pyplot import figure
import matplotlib.font_manager as font_manager

data = pd.read_csv("data.csv")

noise = np.random.normal(0,1,1234)
figure(figsize=(20, 5), dpi=80)
matplotlib.rc('xtick', labelsize=18)
matplotlib.rc('ytick', labelsize=18)
plt.plot(0*noise + 20*(data["P3"]/data["P3"][0]-1),linewidth=5)
plt.title("Sensor", fontsize=18)
#plt.legend(bbox_to_anchor=(1.02,1.15),fontsize=18)
plt.xlabel('time (20 unit = 1 second)', fontsize=18)
plt.ylabel('Actual sensor value', fontsize=18)
plt.savefig('scaled.png')
plt.show()
plt.close()
