'''
GUI for easy use
2nd version of post processing script. Requires user to manually select 3 files. 
3 files:
1. resting.txt
2. raw_data.txt
3. events.txt
'''

import os
import pandas as pd
import numpy as np
from tkinter import *
from tkinter import filedialog

#Making GUI for easy use

class Main:
	root = Tk()
	root.title("Flex Sensor Data Processing App")
	rv_filename = "No file selected"
	rd_filename = "No file selected"
	fe_filename = "No file selected"

class Menu:
	def __init__(self, master):
		self.master = master

		self.button_label = Label(master, text = "Click on buttons directly below (located on left side) to upload files and run program")

		self.rv_button = Button(master,text='Upload \'RESTING.TXT\'',command = self.rv_file)
		self.rv_label = Label(master, text=Main.rv_filename)

		self.rd_button = Button(master,text='Upload \'RAW_DATA.TXT\'',command = self.rd_file)
		self.rd_label = Label(master, text=Main.rd_filename)

		self.fe_button = Button(master,text='Upload \'EVENTS.TXT\'',command = self.fe_file)
		self.fe_label = Label(master, text=Main.fe_filename)

		self.RunButton = Button(master,text = "RUN", fg="green", command = self.run_button, font="Helvetica 16 bold",state='disabled')
		self.run_label = Label(master, text = "Select appropriate text files")
		
		self.button_label.grid(row=0, columnspan=2)
		self.rv_button.grid(row=1,sticky=E)
		self.rv_label.grid(row=1,column=1)
		self.rd_button.grid(row=2,sticky=E)
		self.rd_label.grid(row=2,column=1)
		self.fe_button.grid(row=3,sticky=E)
		self.fe_label.grid(row=3,column=1)
		self.RunButton.grid(row=4,columnspan=2,sticky=W+E)
		self.run_label.grid(row=5,columnspan=2)

	def check_status(self):
		if Main.rv_filename == "" or Main.rv_filename == "No file selected" or \
		   Main.rd_filename == "" or Main.rd_filename == "No file selected" or \
		   Main.fe_filename == "" or Main.fe_filename == "No file selected":
			self.RunButton['state'] = 'disabled'
			self.run_label['text'] = "Select appropriate text files"
		else:
			self.RunButton['state'] = 'normal'

	def rv_file(self):
		Main.rv_filename = filedialog.askopenfilename(initialdir = "/",title = "Select file", filetypes = (("Text Files","*.txt"),("All Files","*.*")))
		if Main.rv_filename == "":
			self.rv_label['text'] = "No file selected"
		elif Main.rv_filename[len(Main.rv_filename)-4:len(Main.rv_filename)] != ".TXT":
			self.rv_label['text'] = "Error! Wrong file selected. Select a .txt file"
		else:
			self.rv_label['text'] = Main.rv_filename
		self.check_status()

	def rd_file(self):
		Main.rd_filename = filedialog.askopenfilename(initialdir = "/",title = "Select file", filetypes = (("Text Files","*.txt"),("All Files","*.*")))
		if Main.rd_filename == "":
			self.rd_label['text'] = "No file selected"
		elif Main.rd_filename[len(Main.rd_filename)-4:len(Main.rd_filename)] != ".TXT":
			self.rd_label['text'] = "Error! Wrong file selected. Select a .txt file"
		else:
			self.rd_label['text'] = Main.rd_filename
		self.check_status()

	def fe_file(self):
		Main.fe_filename = filedialog.askopenfilename(initialdir = "/",title = "Select file", filetypes = (("Text Files","*.txt"),("All Files","*.*")))
		if Main.fe_filename == "":
			self.fe_label['text'] = "No file selected"
		elif Main.fe_filename[len(Main.fe_filename)-4:len(Main.fe_filename)] != ".TXT":
			self.rv_label['text'] = "Error! Wrong file selected. Select a .txt file"
		else:
			self.fe_label['text'] = Main.fe_filename
		self.check_status()

	def run_button(self):

		try:
			STRAIGHT_RESISTANCE = 22
			MAX_RESISTANCE = 31

			#Resistance values in kohms
			r1 = 10
			v_in = 3.7 #input voltage in V

			#Read in resting voltage value:
			resting_voltage_file = open(Main.rv_filename,'r')
			resting_voltage = float(resting_voltage_file.readline())
			# resting_bend_resistance = abs(-r3*(r2*v_in + resting_voltage*(r1 + r2))/(resting_voltage*(r1+r2) - r1*v_in)) #Equation will probably be changed
			resting_bend_resistance = abs(r1*(v_in/(resting_voltage-1.0)))
			resting_bend_angle = np.interp(resting_bend_resistance,[STRAIGHT_RESISTANCE, MAX_RESISTANCE],[0,90.0])


			df = pd.read_csv(Main.rd_filename,sep='\t',skip_blank_lines = False) #filepath should point to 'raw_data.txt'
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

			#Read in finger_events.txt
			df2 = pd.read_csv(Main.fe_filename,sep='\t')
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
			self.run_label.config(text="Error!")
#Execute code
if __name__=="__main__":
	Main.menu = Menu(Main.root)
	Main.root.mainloop()