import numpy as np

file = open("acc_data2.txt", "r")
output = open("acc_results.txt", "w")

data = file.read().splitlines()
data = list(map(float,data))

for item in data: 
	x = np.interp(item,[12.93867925,18.21511628],[0,90])
	print(x)
	# angle = np.interp(5,[0,90],[10.51685393,16.97222222])
	# voltage_5 = 3.7*angle/(9.75+angle)
	# print(voltage_5)
	output.write(str(x))
	output.write('\n')
	
output.write('\n')