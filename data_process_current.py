import os
import sys
import pandas as pd
import numpy as np
from tkinter import *
from tkinter import filedialog
import io
#Making GUI for easy use
class Main:
	root = Tk()
	root.title("Flex Sensor Data Processing App")
	sd_dirname = "No directory selected"

class Menu:
	def __init__(self, master):
		self.master = master
		self.instruction = Label(master, text = "follow instructions below:").grid(row=0,sticky=W,columnspan=2)
		self.instruction1 = Label(master, text = "1. Enter patient info")
		self.instruction2 = Label(master, text = "2. Click button to upload SD card directory")
		self.instruction3 = Label(master, text = "3. Click RUN")

		self.firstname_entry = Entry(master)
		self.firstname_label = Label(master, text = "First Name").grid(row=4,sticky=W)

		self.lastname_entry = Entry(master)
		self.lastname_label = Label(master, text = "Last Name").grid(row=5, sticky=W)

		self.sd_button = Button(master,text='Upload',fg = "green", font = "Helvetica 16 bold",command = self.sd_dir)
		self.sd_label = Label(master, text=Main.sd_dirname)

		self.RunButton = Button(master,text = "RUN", fg="green", command = self.run_button, font="Helvetica 16 bold",state='disabled',anchor = CENTER)
		self.run_label = Label(master, text = "")
		
		self.instruction1.grid(row=1, columnspan=2,sticky = W)
		self.instruction2.grid(row=2,columnspan=2,sticky=W)
		self.instruction3.grid(row=3,columnspan=2,sticky=W)

		self.firstname_entry.grid(row = 4, column=1,sticky=W)
		self.lastname_entry.grid(row=5,column=1,sticky=W)

		
		self.sd_button.grid(row=6,column =0)
		self.sd_label.grid(row=7,column=0)
		
		self.RunButton.grid(row=6,column = 1)
		# self.run_label.grid(row=8,columnspan=2)

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
			self.sd_button.config(state="disabled")
			firstname = self.firstname_entry.get()
			lastname = self.lastname_entry.get()
			#Resistance values in kohms
			r1 = 9.75
			v_in = 3.7 #input voltage in V

			#Read in resting voltage value:

			calibrate_file = open(Main.sd_dirname + "/INIT.TXT",'r')
			values = calibrate_file.readlines()

			straight_voltage = float(values[0])
			bend_voltage = float(values[1])
			

			STRAIGHT_RESISTANCE = abs((r1*straight_voltage)/(v_in-straight_voltage))
			MAX_RESISTANCE = abs((r1*bend_voltage)/(v_in-bend_voltage))

			resistance_5 = np.interp(5,[0,90.0],[STRAIGHT_RESISTANCE, MAX_RESISTANCE])
			voltage_5 = v_in*resistance_5/(r1+resistance_5)

			voltage_diff = voltage_5 - straight_voltage

			resting_voltage_file = open(Main.sd_dirname + "/RESTING.TXT",'r')
			resting_voltages = resting_voltage_file.readlines()

			#sort resting voltages

			resting_voltages = list(map(float, resting_voltages))

			resting_bend_resistances = list(map(lambda x: abs((r1*x)/(v_in-x)), resting_voltages))

			#one of two main lists
			resting_bend_angles = list(map(lambda x: np.interp(x,[STRAIGHT_RESISTANCE, MAX_RESISTANCE],[0,90.0]),resting_bend_resistances))
			
			raw_files = os.listdir(Main.sd_dirname + "/RAW_DATA")

			#sort raw files
			file_tuples = []
			raw_files_sorted = []
			for file in raw_files:
				file_tuple = tuple(file.split('_'))
				file_tuples.append(file_tuple)
				
			file_tuples.sort(key=lambda x : x[1])

			#second main list
			raw_files_sorted = list(map(lambda x : "_".join(x),file_tuples))

			output_dir = "%s_%s_data" % (firstname,lastname)
			os.makedirs(output_dir)

			file_count = 0

			for file in raw_files_sorted:
				filename = file.split('.')[0]
				df = pd.read_csv(Main.sd_dirname + "/RAW_DATA/" + file,sep='\t') #filepath should point to 'raw_data.txt'
				#read in times and voltages
				dates = df['d']
				timestamps = df['ts']
				times = df['t'] #Gets series of time values
				times = times.tolist() #Convert series to list
				voltages = df['V'] #Gets series of voltage values
				voltages = voltages.tolist() #Convert series to list

				#Calculate the bend resistance using known resistances and voltage values
				#Calculate bend angles using resistance. Bend angle is relative to resting hand position.
				#Calculate angular velocities using bend angles
				event_times = []  #For getting duration of an event
				event_dates = []
				event_timestamps = []

				durations = []
				start_dates = []
				end_dates = []
				start_times = []
				end_times = []

				bend_angles = []
				bend_angles_event = []
				max_bend_angles = []

				angular_velocities = []
				angular_velocities_event = []
				average_angular_velocities = []

				events_file = open(output_dir + "/%s_events.txt" % (filename),'w',encoding='utf-8')

				events_header = u"t\tV\tθ\tω\n"
				events_file.write(events_header)

				x = 0
				counter = 0
			
				for voltage in voltages:
					#If there is an empty line indicated by NaN, then this means the event has ended and store the max bend angle and average angular velocity
					bend_resistance = abs((r1*voltage)/(v_in-voltage))
					# print(bend_resistance)
					bend_angle = np.interp(bend_resistance,[STRAIGHT_RESISTANCE, MAX_RESISTANCE],[0,90.0]) - resting_bend_angles[file_count]
					bend_angle = round(bend_angle,2)
					bend_angles.append(bend_angle)

					if counter == 0:
						angular_velocity = 0
						angular_velocities.append(angular_velocity)
					else:
						angular_velocity = ((bend_angle-bend_angles[counter-1])/(times[counter]-times[counter-1]))*1000
						angular_velocity = round(angular_velocity,2)
						angular_velocities.append(angular_velocity)

					#Set threshold for determining an event
					if voltage > resting_voltages[file_count]+voltage_diff:
						#Store max bend angle for event in array
						x = 1
						event_dates.append(dates[counter])
						event_timestamps.append(timestamps[counter])
						event_times.append(times[counter])
						bend_angles_event.append(bend_angle)
						angular_velocities_event.append(angular_velocity)
						event_string = str(dates[counter]) + "\t" + str(timestamps[counter]) + "\t" + str(times[counter])+"\t"+str(voltage)+"\t"+str(bend_angle)+"\t"+str(angular_velocity)+"\n"
						events_file.write(event_string)
					
					else:
						if x == 1:
							events_file.write("\n")
							start_dates.append(event_dates[0])
							end_dates.append(event_dates[len(event_dates)-1])
							start_times.append(event_timestamps[0])
							end_times.append(event_timestamps[len(event_timestamps)-1])

							duration = (event_times[len(event_times)-1]-event_times[0])/1000
							duration = round(duration,2)
							durations.append(duration)
							max_bend_angles.append(max(bend_angles_event))
							average_angular_velocities.append(np.mean(angular_velocities_event))
							event_dates = []
							event_timestamps = []
							event_times = []
							bend_angles_event = []
							angular_velocities_event = []
							x = 0
					counter += 1

				events_file.close()
				file_count += 1

				df['θ'] = bend_angles
				df['ω'] = angular_velocities


				#Restructure dataframe in correct column order
				df = df[['d','ts','t','V','θ','ω']]

				df.to_csv(output_dir + "/%s_post_processing.txt" % (filename), sep='\t', index=False)

				#Create file for processed events
				d = {'Start Date': start_dates, 'End Date': end_dates, 'Start Time': start_times, 'End Time': end_times, 'Time (s)': durations,'Max θ': max_bend_angles,'Avg ω': average_angular_velocities}
				df2 = pd.DataFrame(data=d)

				df2 = df2[['Start Date','End Date','Start Time','End Time','Time (s)','Max θ', 'Avg ω']]
				df2.to_csv(output_dir + '/%s_events_processed.txt' % (filename),sep='\t',index=False)
			
			#Final outputs

			total_event_time = (sum(durations)/3600)
			average_event_time = np.mean(durations)
			average_max_angle = np.mean(max_bend_angles)
			average_angular_velocity = np.mean(average_angular_velocities)  #Or np.mean(angular_velocities), not sure which one to use

			output = open(output_dir + '/final_outputs.txt','w')
			output.write('Total Event Time (Hours) = %s\n' % total_event_time)
			output.write('Average Duration (sec) = %s\n' % average_event_time)
			output.write('Average_max_angle (degrees) = %s\n' % average_max_angle)
			output.write('Average_angular_velocity (degrees/sec) = %s\n' % average_angular_velocity)
			output.close()
			self.sd_label.config(text="Run complete!")
			self.RunButton.config(state="disabled")
			self.sd_button.config(state="normal")
		except Exception as e:
			print(e)
			print(sys.exc_info()[-1].tb_lineno)
			self.run_label.config(text="Error!")
#Execute code
if __name__=="__main__":
	Main.menu = Menu(Main.root)
	Main.root.mainloop()