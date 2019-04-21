# README
*Lists out all the code and files used in this project.*
*There are two folders of code, one containing all Arduino sketches and other containing post processing script written in Python.*

**Upload `flex_final.ino` to TinyZero processor board. Use `data_process_current.exe`, which was built from `data_process_current.py`, for post processing.**

To create application (`.exe` file) from python script (i.e. `myscript.py`), use [pyinstaller](https://pyinstaller.readthedocs.io/en/stable/) 

Install pyinstaller and run following command in terminal:
`pyinstaller -w -i <py.ico> -F myscript.py`

## Arduino code

1. **`accuracy_test.ino`**<br/>Used to conduct accuracy testing of flex sensor. Bends flex sensor using a servo from 0 to 90 degrees and gets voltage outputs from flex sensor.

2. **`event_test.ino`**<br/>First version used for event testing. Controls a servo which is attached to the flex sensor. Servo moves to 10 degrees then back to 0 degrees. It does this again for 8 degrees and 6 degrees. The servo moves fast and slow for all angles to compare flex sensor's ability to respond to different movement speeds.

3. **`event_test2.ino`**<br/>This sketch is the second version used for event testing. This sketch controls a servo which is attached to the flex sensor. The servo moves to 10 degrees then back to 0 degrees. It does this again for 8 degrees and 6 degrees. The servo moves at the same speed for all angles.

4. **`flex_calibrate.ino`**<br/>Stores voltage at 0 and 90 degrees. Should be done prior to using device.

5. **`flex_test_uno.ino`**<br/>Sketch used for basic flex sensor testing while flex sensor is connected to Arduino Uno via breadboard. Moving average is used to give more accurate voltage readings.

6. **`flex_sensor1.ino`**<br/>Sketch stores voltages and logs time as flex sensor is bent. Sketch considers event to be anytime voltage is different from resting voltage. Sketch logs running timer and voltage values on raw data file only when event is occurring (so it doesn't log all data), and times of each event on separate event file.\
\
Outputs:
    - `resting.txt` --> resting voltage value
    - `raw_data.txt` --> elapsed time and voltages for each event
    - `events.txt` --> Time of each finger event
         
7. **`flex_sensor_live.ino`**<br/>This is an old version of the TinyZero code. This code is useful for seeing bend angles, angular velocity, and event data in real time. But this version is more memory intensive and not good for long term data logging. Much of code here, such as calculating bend angles and identifying events, was moved to post processing script to save memory. Reduces memory usage reduces amount of raw data lost during data logging and storing process.\
\
Outputs:
    - `resting.txt` --> resting voltage value
    - `raw_data.txt` --> running time, voltages, angles, angular velocities
    - `events.txt` --> running time, voltages, angles, angular velocities when voltage is greater than 5 degree threshold
         
8. **`flex_v3.ino`**<br/>This sketch records output voltage changes as flex sensor bends and device is worn during daily activity. Output voltage increases as flex sensor resistance increases (as it bends more) because flex sensor is R2 in voltage divider. Sketch also records timestamps and time since sketch was first uploaded to board for every data point.\
\
Outputs: 
    - `resting.txt` --> resting voltage value
    - raw data file(s) --> timestamps, running time, and voltage for all data
         
9. **`flex_final.ino`**<br/>Final version of code for FlexiCapture, a continuous data logging wearable device. This sketch records output voltage changes as device is worn during daily activity. Output voltage increases as flex sensor resistance increases (as it bends more) because flex sensor is R2 in voltage divider. Sketch also records timestamps and time since sketch was first uploaded to board for every data point. Uses a capacitive sensor to determine when device is being worn and when to log data.\
\
Outputs:
    - `resting.txt` --> resting voltage values for each time calibration is done
    - raw data file(s) --> timestamps, running time, and voltage for all data

## Post Processing Code

1. **`bend_angles.py`**<br/>Script to quickly get bend angles. User inputs resistance at 0 and 90 degrees. Used during accuracy and event testing.

2. **`data_process1.py`**<br/>1st version of post processing script. Does not create GUI so could only be used in terminal. User must input appropriate input text file names in terminal.\
\
User inputs:
    - Flex sensor resistance at 0 and 90 degrees
    - `resting.txt` --> resting voltage value
    - `raw_data.txt` --> time and voltage values
    - `events.txt` --> time of each finger event

3. **`data_process2.py`**<br/>2nd version of post processing script. Creates GUI for easy use. Requires user to manually upload following 3 files:
    - `resting.txt`
    - `raw_data.txt`
    - `events.txt`

4. **`data_process3.py`**<br/>3rd version of data processing app. User only has to upload SD card directory to run.

5. **`data_process_current.py`**<br/>Final version of post processing script that makes GUI for easy use. Takes in data and calculates durations, bend angles, angular velocities, and more. Person uses GUI to upload SD card directory. Script can process more than one raw data file.

    - User inputs:
        - Patient first name and last name
        - SD card directory
    - Program outputs:
        - Folder titled `firstname_lastname_data` with following files:
            - `events.txt`
            - `events_processed.txt`
            - `post_processing.txt`
            - `final_outputs.txt`
\
First three files have date and file count in name that corresponds to raw data file i.e. `49R_0_events.txt` for the first raw data file created on April 9th.
