'''
3rd version of data processing app
User only has to upload SD card directory
'''

import os
import sys
import pandas as pd
import numpy as np
from tkinter import *
from tkinter import filedialog

#Making GUI for easy use

class Main:
	root = Tk()
	root.title("Flex Sensor Data Processing App")
	sd_dirname = "No directory selected"

class Menu:
	def __init__(self, master):
		self.master = master
		self.instruction = Label(master,text = "Follow instructions below:").grid(row=0,sticky=W,columnspan=2)
		self.instruction1 = Label(master, text = "1. Enter patient info")
		self.instruction2 = Label(master, text = "2. Click button to upload SD card directory")
		self.instruction3 = Label(master, text = "3. Click RUN")

		self.firstname_entry = Entry(master)
		self.firstname_label = Label(master, text = "First Name").grid(row=4,sticky=W)

		self.lastname_entry = Entry(master)
		self.lastname_label = Label(master, text = "Last Name").grid(row=5, sticky=W)

		self.sd_button = Button(master,text='Upload',command = self.sd_dir)
		self.sd_label = Label(master, text=Main.sd_dirname)

		self.RunButton = Button(master,text = "RUN", fg="green", command = self.run_button, font="Helvetica 16 bold",state='disabled',anchor = CENTER)
		self.run_label = Label(master, text = "")
		
		self.instruction1.grid(row=1, columnspan=2,sticky = W)
		self.instruction2.grid(row=2,columnspan=2,sticky=W)
		self.instruction3.grid(row=3,columnspan=2,sticky=W)

		self.firstname_entry.grid(row = 4, column=1,sticky=W)
		self.lastname_entry.grid(row=5,column=1,sticky=W)

		
		self.sd_button.grid(row=6)
		self.sd_label.grid(row=6,column=1,sticky=W)
		
		self.RunButton.grid(row=7,columnspan=2)
		self.run_label.grid(row=8,columnspan=2)

	def check_status(self):
		if Main.sd_dirname == "" or Main.sd_dirname == "No directory selected":
			self.RunButton['state'] = 'disabled'
			self.run_label['text'] = ""
		else:
			self.RunButton['state'] = 'normal'

	def sd_dir(self):
		Main.sd_dirname = filedialog.askdirectory(initialdir = os.getcwd(), title = "Select directory")
		self.sd_label['text'] = Main.sd_dirname
		if Main.sd_dirname == "":
			self.sd_label['text'] = "No directory selected"
		self.check_status()

	def run_button(self):

		try:
			STRAIGHT_RESISTANCE = 22
			MAX_RESISTANCE = 31

			#Resistance values in kohms
			r1 = 10
			v_in = 3.7 #input voltage in V

			#Read in resting voltage value:
			resting_voltage_file = open(Main.sd_dirname + "/RESTING.TXT",'r')
			resting_voltage = float(resting_voltage_file.readline())
			# resting_bend_resistance = abs(-r3*(r2*v_in + resting_voltage*(r1 + r2))/(resting_voltage*(r1+r2) - r1*v_in)) #Equation will probably be changed
			resting_bend_resistance = abs(r1*(v_in/(resting_voltage-1.0)))
			resting_bend_angle = np.interp(resting_bend_resistance,[STRAIGHT_RESISTANCE, MAX_RESISTANCE],[0,90.0])


			df = pd.read_csv(Main.sd_dirname + "/RAW_DATA.TXT",sep='\t',skip_blank_lines = False) #filepath should point to 'raw_data.txt'
			df = df.append(pd.Series([np.nan,np.nan], index=df.columns ), ignore_index=True)
			#read in times and voltages
			times = df['Time (ms)'] #Gets series of time values
			times = times.tolist() #Convert series to list
			voltages = df['Voltage (V)'] #Gets series of voltage values
			voltages = voltages.tolist() #Convert series to list
			bend_angles = df['Bend Angle (degrees']
			angular_velocities = df['Angular Velocity (degrees/sec)']
			#Calculate the bend resistance using known resistances and voltage values
			#Calculate bend angles using resistance. Bend angle is relative to resting hand position.
			#Calculate angular velocities using bend angles
			bend_angles_event = []
			max_bend_angles = []
			
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
					else:
						max_bend_angles.append(np.nan)
						
					#Store average angular velocity for event in array
					if len(angular_velocities_event) != 0:
						average_angular_velocities.append(np.mean(angular_velocities_event))
					else:
						average_angular_velocities.append(np.nan)
					#Clear array for bend angles in that event
					bend_angles_event = []
					angular_velocities_event = []
				else:
					#Calculate resistance, bend angle and angular velocity for every voltage value
					# bend_resistance = abs(-r3*(r2*v_in + voltage*(r1 + r2))/(voltage*(r1+r2) - r1*v_in)) #Equation will probably be changed
					bend_resistance = abs(r1*(v_in/(voltage-1.0)))
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

			os.makedirs("Output")
			#Restructure dataframe in correct column order
			df = df[['Time (ms)','Voltage (V)','Bend Angle (degrees)','Angular Velocity (degrees/sec)']]
			df.to_csv('Output/post_processing.txt',sep='\t',index=False)

			#Read in events.txt
			df2 = pd.read_csv(Main.sd_dirname + "/events.txt" ,sep='\t')
			time_durations = df2['Time (sec)']

			df2['Max Bend Angle (degrees)'] = max_bend_angles
			df2['Average Angular Velocity (degrees/sec)'] = average_angular_velocities

			df2 = df2[['Time (sec)','Max Bend Angle (degrees)', 'Average Angular Velocity (degrees/sec)']]
			df2.to_csv('Output/finger_events_processed.txt',sep='\t',index=False)

			#Final outputs
			total_time = sum(time_durations)/60
			average_time = np.mean(time_durations)
			average_max_angle = np.mean(max_bend_angles)
			average_angular_velocity = np.mean(average_angular_velocities)  #Or np.mean(angular_velocities), not sure which one to use

			output = open('Output/final_outputs.txt','w')
			output.write('Total Time (Hours) = %s\n' % total_time)
			output.write('Average Duration (sec) = %s\n' % average_time)
			output.write('Average_max_angle (degrees) = %s\n' % average_max_angle)
			output.write('Average_angular_velocity (degrees/sec) = %s\n' % average_angular_velocity)
			self.run_label.config(text="Run complete!")
		except Exception as e:
			print(e)
			print(sys.exc_info()[-1].tb_lineno)
			self.run_label.config(text="Run Complete!")
#Execute code
if __name__=="__main__":
	Main.menu = Menu(Main.root)
	Main.root.mainloop()
