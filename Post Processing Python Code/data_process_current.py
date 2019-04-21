'''
Final version of post processing script that makes GUI for easy use
Takes in data and calculates durations, bend angles, angular velocities, and more.
Person uses GUI to upload directory and can process more than one raw data file 

User inputs:

1. Patient first name and last name
2. SD card directory

Program outputs:
Folder titled firstname_lastname_data with following files:
1. events.txt
2. events_processed.txt
3. post_processing.txt
4. final_outputs.txt

First three files have date and file count in name that corresponds to raw data file
i.e. 49R_0_events.txt for the first raw data file created on April 9th
'''

#Import packages
import os
import sys
import pandas as pd
import numpy as np
from tkinter import *		#tkinter used to make GUI
from tkinter import filedialog
import io

#Making GUI for easy use
class Main:
	root = Tk()
	root.title("Flex Sensor Data Processing App")  #Title for app
	sd_dirname = "No directory selected"
	root.wm_iconbitmap('py.ico')	#Icon for app

class App:
	def __init__(self, master):
		self.master = master

		#instructions for user
		self.instruction = Label(master, text = "follow instructions below:").grid(row=0,sticky=W,columnspan=2)
		self.instruction1 = Label(master, text = "1. Enter patient info")
		self.instruction2 = Label(master, text = "2. Click button to upload SD card directory")
		self.instruction3 = Label(master, text = "3. Click RUN")

		self.firstname_entry = Entry(master)
		self.firstname_label = Label(master, text = "First Name").grid(row=4,sticky=W)

		self.lastname_entry = Entry(master)
		self.lastname_label = Label(master, text = "Last Name").grid(row=5, sticky=W)

		#Button to click on for uploading SD card directory into app. When button is clicked, it runs function "sd_dir"
		self.sd_button = Button(master,text='Upload',fg = "green", font = "Helvetica 16 bold",command = self.sd_dir)
		self.sd_label = Label(master, text=Main.sd_dirname)

		##Button to click on to run the application. When this button is clicked, it runs the function "run_code"
		self.RunButton = Button(master,text = "RUN", fg="green", command = self.run_code, font="Helvetica 16 bold",state='disabled',anchor = CENTER)
		self.run_label = Label(master, text = "")
		
		#Places instructions, name entrys and buttons in specified orientation
		self.instruction1.grid(row=1, columnspan=2,sticky = W)
		self.instruction2.grid(row=2,columnspan=2,sticky=W)
		self.instruction3.grid(row=3,columnspan=2,sticky=W)

		self.firstname_entry.grid(row = 4, column=1,sticky=W)
		self.lastname_entry.grid(row=5,column=1,sticky=W)
		
		self.sd_button.grid(row=6,column =0)
		self.sd_label.grid(row=7,column=0)
		
		self.RunButton.grid(row=6,column = 1)
		self.run_label.grid(row=7,column=1)

	#Function updates name with directory selected and sd label
	def sd_dir(self):

		#if user does not select a file, they won't be able to run the program
		Main.sd_dirname = filedialog.askdirectory(initialdir = os.getcwd(), title = "Select directory")
		if Main.sd_dirname == "":
			self.sd_label['text'] = "No directory selected"
			self.RunButton['state'] = 'disabled'
		else: 
			self.sd_label['text'] = Main.sd_dirname
			self.RunButton['state'] = 'normal'

	def run_code(self):

		try:
			#read in firstname and lastname of patient from user entry
			firstname = self.firstname_entry.get()
			lastname = self.lastname_entry.get()
			#Resistance values in kohms
			r1 = 9.75
			v_in = 3.7 #input voltage in V

			#Read in voltage values to calculate bend angle

			calibrate_file = open(Main.sd_dirname + "/INIT.TXT",'r')
			values = calibrate_file.readlines()

			#Needed to get bend angle
			straight_voltage = float(values[0])		#voltage at 0 degrees
			bend_voltage = float(values[1])			#voltage at 90 degrees
			
			#Flex sensor resistance = (r1 * v_out)/(v_in - v_out)
			STRAIGHT_RESISTANCE = abs((r1*straight_voltage)/(v_in-straight_voltage))
			MAX_RESISTANCE = abs((r1*bend_voltage)/(v_in-bend_voltage))

			#Get voltage at 5 degrees to set threshold
			resistance_5 = np.interp(5,[0,90.0],[STRAIGHT_RESISTANCE, MAX_RESISTANCE])
			voltage_5 = v_in*resistance_5/(r1+resistance_5)

			#Gets voltage difference between 5 degrees and 0 degrees, add this to resting voltage to set threshold
			voltage_diff = voltage_5 - straight_voltage

			#read in resting voltage values and convert all to floats. These values used to determine event threshold
			resting_voltage_file = open(Main.sd_dirname + "/RESTING.TXT",'r')
			resting_voltages = resting_voltage_file.readlines()

			resting_voltages = list(map(float, resting_voltages))

			resting_bend_resistances = list(map(lambda x: abs((r1*x)/(v_in-x)), resting_voltages))	#converts voltages to resistance

			#converts resistances to bend angles using one dimensional linear interpolation
			resting_bend_angles = list(map(lambda x: np.interp(x,[STRAIGHT_RESISTANCE, MAX_RESISTANCE],[0,90.0]),resting_bend_resistances)) 	
			
			#store all filenames in RAW_DATA folder in a list for sorting
			raw_files = os.listdir(Main.sd_dirname + "/RAW_DATA")

			#sort raw data files so that appropriate file corresponds to appropriate resting voltage value
 			file_tuples = []
			raw_files_sorted = []
			for file in raw_files:
				file_tuple = tuple(file.split('_'))
				file_tuples.append(file_tuple)

			#Sort files in ascending order based on count number in file name 
			file_tuples.sort(key=lambda x : x[1])

			#sorted list of raw data files
			raw_files_sorted = list(map(lambda x : "_".join(x),file_tuples))

			#Create folder to store all output files. Folder name is firstname_lastname_data
			output_dir = "%s_%s_data" % (firstname,lastname)
			os.makedirs(output_dir)

			file_count = 0

			#Start of data processing. Loops through all raw data files in folder. Calculate bend angles, angular velocity and more.

			for file in raw_files_sorted:
				filename = file.split('.')[0]
				df = pd.read_csv(Main.sd_dirname + "/RAW_DATA/" + file,sep='\t') #filepath points to raw data files
				#read in times and voltages from different columns of txt file using column names
				timestamps = df['ts']
				times = df['t'] #Gets series of time values
				times = times.tolist() #Convert series to list
				voltages = df['V'] #Gets series of voltage values
				voltages = voltages.tolist() #Convert series to list							
				
				bend_angles = []
				angular_velocities = []

				#Lists used for processed events file
				event_times = []
				event_timestamps = []
				durations = []
				start_times = []
				end_times = []
				max_bend_angles = []
				average_angular_velocities = []
				angular_velocities_event = []
				bend_angles_event = []		

				events_file = open(output_dir + "/%s_events.txt" % (filename),'w',encoding='utf-8')  #File for storing event data 

				events_header = u"ts\tt\tV\tθ\tω\n"
				events_file.write(events_header)

				x = 0			#Switch to determine whether you've finished going through data of a single event. Used to separate out events
				counter = 0		#acts as index when looping through lists
			
				for voltage in voltages:
					bend_resistance = abs((r1*voltage)/(v_in-voltage))

					#Calculate bend angles using resistance. Bend angle is relative to resting hand position.
					bend_angle = np.interp(bend_resistance,[STRAIGHT_RESISTANCE, MAX_RESISTANCE],[0,90.0]) - resting_bend_angles[file_count]
					bend_angle = round(bend_angle,2)
					bend_angles.append(bend_angle)

					#For first value, set angular velocity equal to zero to initialize
					if counter == 0:
						angular_velocity = 0
						angular_velocities.append(angular_velocity)  
					else:
						#Calculate angular velocities using bend angles and times
						angular_velocity = ((bend_angle-bend_angles[counter-1])/(times[counter]-times[counter-1]))*1000
						angular_velocity = round(angular_velocity,2)
						angular_velocities.append(angular_velocity)

					#If voltage equates to greater than 5 degree angle difference from resting voltage, then classify data as finger event
					if voltage > resting_voltages[file_count]+voltage_diff:
						#Events file contains same parameters as post_processing file, just for data that passes threshold
						x = 1
						event_timestamps.append(timestamps[counter])
						event_times.append(times[counter])
						bend_angles_event.append(bend_angle)
						angular_velocities_event.append(angular_velocity)
						event_string = str(timestamps[counter]) + "\t" + str(times[counter])+"\t"+str(voltage)+"\t"+str(bend_angle)+"\t"+str(angular_velocity)+"\n"
						events_file.write(event_string)
					
					else:
						if x == 1:
							#Get values for events_processed file. For every event, gets duration of event, max bend angle and average angular velocity
							events_file.write("\n")
							start_times.append(event_timestamps[0])
							end_times.append(event_timestamps[len(event_timestamps)-1])
							duration = (event_times[len(event_times)-1]-event_times[0])/1000	#Get event duration in seconds
							duration = round(duration,2)
							durations.append(duration)
							max_bend_angles.append(max(bend_angles_event))
							average_angular_velocities.append(round(np.mean(angular_velocities_event),2))
							event_timestamps = []
							event_times = []
							bend_angles_event = []
							angular_velocities_event = []
							x = 0
					counter += 1

				events_file.close()
				file_count += 1

				#Create new columns in dataframe with bend angles and angular velocities for post processing file
				df['θ'] = bend_angles 	
				df['ω'] = angular_velocities

				#Restructure dataframe in correct column order
				df = df[['ts','t','V','θ','ω']]

				#Create file for post processing. Contains all raw data plus corresponding bend angles and angular velocity.
				df.to_csv(output_dir + "/%s_post_processing.txt" % (filename), sep='\t', index=False)

				#Create file for processed events
				d = {'Start Time': start_times, 'End Time': end_times, 'Time (s)': durations,'Max θ': max_bend_angles,'Avg ω': average_angular_velocities}
				df2 = pd.DataFrame(data=d)

				#Create file for processed events
				df2 = df2[['Start Time','End Time','Time (s)','Max θ', 'Avg ω']]
				df2.to_csv(output_dir + '/%s_events_processed.txt' % (filename),sep='\t',index=False)
			
			#Final outputs

			#Round all final outputs to 2 decimal places
			total_event_time = round((sum(durations)/3600),2)	#Get total event time in hours
			average_event_time = round(np.mean(durations),2)
			average_max_angle = round(np.mean(max_bend_angles),2)
			average_angular_velocity = round(np.mean(average_angular_velocities),2)  #Or np.mean(angular_velocities), not sure which one to use

			output = open(output_dir + '/final_outputs.txt','w')
			output.write('Total Event Time (Hours) = %s\n' % total_event_time)
			output.write('Average Duration (sec) = %s\n' % average_event_time)
			output.write('Average max angle (degrees) = %s\n' % average_max_angle)
			output.write('Average angular velocity (degrees/sec) = %s\n' % average_angular_velocity)
			output.close()
			self.sd_label.config(text="Run complete!")		#Will show up under SD card button once the run successfully completes
			self.RunButton.config(state="disabled")			#Run button will be disabled, so user will have to upload new directory to run again
		
		#Error handling for debugging when running code in terminal. Tells you what the error is.
		except Exception as e:
			print(e)
			print(sys.exc_info()[-1].tb_lineno)
			self.run_label.config(text="Error!")

#Execute code
if __name__=="__main__":
	Main.app = App(Main.root)
	Main.root.mainloop()