#Script to quickly get bend angles

import numpy as np

#User input
r_0 = 10.51685393  #resistance value at 0 degrees
r_90 = 17.78816794 #resistance value at 90 degrees

input_file = open("acc_data.txt", "r")
output = open("acc_results.txt", "w")

data = input_file.read().splitlines()	#Reads in values from input file into list
data = list(map(float,data))			#Converts all values in list to floats

for item in data: 
	#Uses one-dimensional linear interpolation given resistance at 0 and 90 degrees to estimate bend angle
	angle = np.interp(item,[r_0,r_90],[0,90])	
	print(angle)
	output.write(str(angle))	#Saves bend angles to file
	output.write('\n')