import os
import sys
import pandas as pd
import numpy as np
from tkinter import *
from tkinter import filedialog
import io
#Making GUI for easy use
print('hello')
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

		self.sd_button = Button(master,text='Upload',command = self.sd_dir)
		self.sd_label = Label(master, text=Main.sd_dirname)

		self.RunButton = Button(master,text = "RUN", fg="green", command = self.run_button, font="Helvetica 16 bold",state='disabled',anchor = CENTER)
		self.run_label = Label(master, text = "")
		
		self.instruction1.grid(row=1, columnspan=2,sticky = W)
		self.instruction2.grid(row=2,columnspan=2,sticky=W)
		self.instruction3.grid(row=3,columnspan=2,sticky=W)

		self.firstname_entry.grid(row = 4, column=1,sticky=W)
		self.lastname_entry.grid(row=5,column=1,sticky=W)

		
		self.sd_button.grid(row=6,sticky=W)
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
			# STRAIGHT_RESISTANCE = 34

			# MAX_RESISTANCE = 45

			#Resistance values in kohms
			r1 = 9.75
			v_in = 3.7 #input voltage in V

			#Read in resting voltage value:

			resting_voltage_file = open(Main.sd_dirname + "/RESTING.TXT",'r')
			resting_voltage = float(resting_voltage_file.readline())
			resting_bend_resistance = abs((r1*resting_voltage)/(v_in-resting_voltage))

			straight_voltage = 2.81
			bend_voltage = 3.37

			STRAIGHT_RESISTANCE = abs((r1*straight_voltage)/(v_in-straight_voltage))
			MAX_RESISTANCE = abs((r1*bend_voltage)/(v_in-bend_voltage))

			resting_bend_angle = np.interp(resting_bend_resistance,[STRAIGHT_RESISTANCE, MAX_RESISTANCE],[0,90.0])

			df = pd.read_csv(Main.sd_dirname + "/RAW_DATA.TXT",sep='\t',skip_blank_lines = False) #filepath should point to 'raw_data.txt'
			#read in times and voltages
			times = df['t'] #Gets series of time values
			times = times.tolist() #Convert series to list
			voltages = df['V'] #Gets series of voltage values
			voltages = voltages.tolist() #Convert series to list

			#Calculate the bend resistance using known resistances and voltage values
			#Calculate bend angles using resistance. Bend angle is relative to resting hand position.
			#Calculate angular velocities using bend angles
			event_times = []  #For getting duration of an event
			durations = []

			bend_angles = []
			bend_angles_event = []
			max_bend_angles = []


			angular_velocities = []
			angular_velocities_event = []
			average_angular_velocities = []

			os.makedirs("Output")

			firstname = self.firstname_entry.get()
			lastname = self.lastname_entry.get()

			events_file = open("Output/%s_%s_events.txt" % (firstname,lastname),'w',encoding='utf-8')

			events_header = u"t\tV\tθ\tω\n"
			events_file.write(events_header)

			x = 0
			counter = 0
			event_counter = 0
			for voltage in voltages:
				#If there is an empty line indicated by NaN, then this means the event has ended and store the max bend angle and average angular velocity
				bend_resistance = abs((r1*voltage)/(v_in-voltage))
				# print(bend_resistance)
				bend_angle = np.interp(bend_resistance,[STRAIGHT_RESISTANCE, MAX_RESISTANCE],[0,90.0]) - resting_bend_angle
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
				if voltage != resting_voltage:
					#Store max bend angle for event in array
					x = 1
					event_times.append(times[counter])
					bend_angles_event.append(bend_angle)
					angular_velocities_event.append(angular_velocity)
					event_string = str(times[counter])+"\t"+str(voltage)+"\t"+str(bend_angles_event[event_counter])+"\t"+str(angular_velocities_event[event_counter])+"\n"
					events_file.write(event_string)
					event_counter += 1
				else:
					if x == 1:
						events_file.write("\n")
						duration = (event_times[len(event_times)-1]-event_times[0])/1000
						# duration = round(duration,2)
						durations.append(duration)
						max_bend_angles.append(max(bend_angles_event))
						average_angular_velocities.append(np.mean(angular_velocities_event))
						event_times = []
						bend_angles_event = []
						angular_velocities_event = []
						x = 0
						event_counter = 0
				counter += 1

			df['θ'] = bend_angles
			df['ω'] = angular_velocities


			#Restructure dataframe in correct column order
			df = df[['t','V','θ','ω']]

			df.to_csv('Output/%s_%s_post_processing.txt' % (firstname,lastname), sep='\t', index=False)

			#Create file for processed events
			d = {'Time (s)': durations,'Max θ': max_bend_angles,'Avg ω': average_angular_velocities}
			df2 = pd.DataFrame(data=d)

			df2 = df2[['Time (s)','Max θ', 'Avg ω']]
			df2.to_csv('Output/%s_%s_events_processed.txt' % (firstname,lastname),sep='\t',index=False)

			#Final outputs
			total_time = (times[len(times)-1]/3600000)
			total_event_time = (sum(durations)/3600000)
			average_event_time = np.mean(durations)
			average_max_angle = np.mean(max_bend_angles)
			average_angular_velocity = np.mean(average_angular_velocities)  #Or np.mean(angular_velocities), not sure which one to use

			output = open('Output/%s_%s_final_outputs.txt' % (firstname,lastname),'w')
			output.write('Total Time (Hours) = %s\n' % total_time)
			output.write('Total Event Time (Hours) = %s\n' % total_event_time)
			output.write('Average Duration (sec) = %s\n' % average_event_time)
			output.write('Average_max_angle (degrees) = %s\n' % average_max_angle)
			output.write('Average_angular_velocity (degrees/sec) = %s\n' % average_angular_velocity)
			output.close()
			events_file.close()
			self.run_label.config(text="Run complete!")
		except Exception as e:
			print(e)
			print(sys.exc_info()[-1].tb_lineno)
			self.run_label.config(text="Error!")
#Execute code
if __name__=="__main__":
	Main.menu = Menu(Main.root)
	Main.root.mainloop()