import matplotlib.pyplot as plt
import pandas as pd

df = pd.read_csv("DATA.TXT",sep='\t',skip_blank_lines = False)

times = df['t'] #Gets series of time values
times = times.tolist() #Convert series to list


voltages = df['v']
voltages = voltages.tolist()

resistances = df['r']
resistances = resistances.tolist()

angles = df['a']
angles = angles.tolist()

plt.plot(times,voltages)
plt.ylabel('voltages')
plt.xlabel('time')
plt.show()

plt.plot(times,resistances)
plt.ylabel('resistances')
plt.xlabel('time')
plt.show()

plt.plot(times,resistances)
plt.ylabel('angles')
plt.xlabel('time')
plt.show()