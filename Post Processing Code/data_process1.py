'''

1st version of post processing script
Does not create GUI so could only be used in terminal 

Three main txt files for inputs and three txt files for outputs. One text file has calculations on every raw data point.
Another text file has max bend angles and average angular velocities for every event
Final text file provides overview giving total time and other parameters.

Things user needs to do: define the inputs and define the outputs
Inputs: text files
Outputs: text files

'''
import pandas as pd
import numpy as np

# STRAIGHT_RESISTANCE = input('What is the resistance at 0 degrees?\n'); # resistance when straight
# type(STRAIGHT_RESISTANCE)
STRAIGHT_RESISTANCE = 10
# MAX_RESISTANCE = input('What is the resistance at 90 degrees?\n'); # resistance at 90 deg
# type(MAX_RESISTANCE)
MAX_RESISTANCE = 40

#Resistance values in kohms
r1 = 10
r2 = 10
r3 = 10
v_in = 3.7 #input voltage in V

#Read in resting voltage value:
resting_voltage_file = open('resting.txt','r')
resting_voltage = float(resting_voltage_file.readline())
resting_bend_resistance = abs(r1*(v_in/(resting_voltage-1.0)))
print(resting_bend_resistance)
# resting_bend_resistance = abs(-r3*(r2*v_in + resting_voltage*(r1 + r2))/(resting_voltage*(r1+r2) - r1*v_in)) #Equation will probably be changed
resting_bend_angle = np.interp(resting_bend_resistance,[STRAIGHT_RESISTANCE, MAX_RESISTANCE],[0,90.0])

# filepath_raw_data = input("What is the filepath of your text file \'raw_data.txt\'?\n")
# type(filepath)

# filepath_finger_events = input("What is the filepath of your text file \'finger_events.txt\'?\n")
# type(filepath)

# df = pd.read_csv(filepath_raw_data,sep='\t',skip_blank_lines = False) #filepath should point to 'raw_data.txt'

# df = pd.read_csv('C:\\Users\\David\\Documents\\raw_data.txt',sep='\t',skip_blank_lines = False) #filepath should point to 'raw_data.txt'
df = pd.read_csv('raw_data.txt',sep='\t',skip_blank_lines = False) #filepath should point to 'raw_data.txt'
df = df.append(pd.Series([np.nan,np.nan], index=df.columns ), ignore_index=True)
#read in times and voltages
times = df['Time (ms)'] #Gets series of time values
times = times.tolist() #Convert series to list
voltages = df['Voltage (V)'] #Gets series of voltage values
voltages = voltages.tolist() #Convert series to list

#Calculate the bend resistance using known resistances and voltage values
#Calculate bend angles using resistance. Bend angle is relative to resting hand position.
#Calculate angular velocities using bend angles
bend_angles = []
bend_angles_event = []
max_bend_angles = []

angular_velocities = []
angular_velocities_event = []
average_angular_velocities = []

counter = 0
for voltage in voltages:
	#If there is an empty line indicated by NaN, then this means the event has ended and store the max bend angle and average angular velocity
	if pd.isna(voltage):
		#Store max bend angle for event in array
		bend_angles.append(np.nan)
		angular_velocities.append(np.nan)
		if len(bend_angles_event) != 0:
			max_bend_angles.append(max(bend_angles_event))
			#Clear array for bend angles in that event
			bend_angles_event = []
		#Store average angular velocity for event in array
		if len(angular_velocities_event) != 0:
			average_angular_velocities.append(np.mean(angular_velocities_event))
			angular_velocities_event = []
	else:
		#Calculate resistance, bend angle and angular velocity for every voltage value
		bend_resistance = abs(r1*(v_in/(voltage-1.0)))
		print(bend_resistance)
		# bend_resistance = abs(-r3*(r2*v_in + voltage*(r1 + r2))/(voltage*(r1+r2) - r1*v_in)) #Equation will probably be changed
		bend_angle = np.interp(bend_resistance,[STRAIGHT_RESISTANCE, MAX_RESISTANCE],[0,90.0]) - resting_bend_angle
		bend_angles_event.append(bend_angle)
		bend_angles.append(bend_angle)
		#If there is only 1 bend angle, then angular velocity is 0
		if len(bend_angles_event) == 1:
			angular_velocity = 0
			angular_velocities_event.append(angular_velocity)
			angular_velocities.append(angular_velocity)
		else:
			angular_velocity = (bend_angle-bend_angles[counter-1])/(times[counter]-times[counter-1])*1000
			angular_velocities_event.append(angular_velocity)
			angular_velocities.append(angular_velocity)
	counter += 1

df['Bend Angle (degrees)'] = bend_angles
df['Angular Velocity (degrees/sec)'] = angular_velocities

#Restructure dataframe in correct column order
df = df[['Time (ms)','Voltage (V)','Bend Angle (degrees)','Angular Velocity (degrees/sec)']]
df.to_csv('post_processing.txt',sep='\t',index=False)

# df2 = pd.read_csv('C:\\Users\\David\\Documents\\finger_events.txt',sep='\t')
df2 = pd.read_csv('events.txt',sep='\t')
time_durations = df2['Time (sec)']

df2['Max Bend Angle (degrees)'] = max_bend_angles
df2['Average Angular Velocity (degrees/sec)'] = average_angular_velocities

df2 = df2[['Time (sec)','Max Bend Angle (degrees)', 'Average Angular Velocity (degrees/sec)']]
# df2.to_csv('C:\\Users\\David\\Documents\\finger_events_processed.txt',sep='\t',index=False)
df2.to_csv('finger_events_processed.txt',sep='\t',index=False)

#Final outputs
total_time = sum(time_durations)/60
average_time = np.mean(time_durations)
average_max_angle = np.mean(max_bend_angles)
average_angular_velocity = np.mean(average_angular_velocities)  #Or np.mean(angular_velocities), not sure which one to use

# output = open('C:\\Users\\David\\Documents\\final_outputs.txt','w')
output = open('final_outputs.txt','w')
output.write('Total Time (Hours) = %s\n' % total_time)
output.write('Average Duration (sec) = %s\n' % average_time)
output.write('Average_max_angle (degrees) = %s\n' % average_max_angle)
output.write('Average_angular_velocity (degrees/sec) = %s\n' % average_angular_velocity)