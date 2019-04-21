# senior-design
*Lists out all the code used in this project.*
*There are two folders of code, one containing all Arduino sketches and other containing post processing script written in Python.*

## Arduino code
1. **accuracy_test.ino** 
Used to conduct accuracy testing of flex sensor. Bends flex sensor using a servo from 0 to 90 degrees and gets voltage outputs from flex sensor.

2. **event_test.ino**
First version used for event testing. Controls a servo which is attached to the flex sensor. Servo moves to 10 degrees then back to 0 degrees. It does this again for 8 degrees and 6 degrees. The servo moves fast and slow for all angles to compare flex sensor's ability to respond to different movement speeds.

3. **event_test2.ino**
This sketch is the second version used for event testing. This sketch controls a servo which is attached to the flex sensor. The servo moves to 10 degrees then back to 0 degrees. It does this again for 8 degrees and 6 degrees. The servo moves at the same speed for all angles.

4. **flex_calibrate.ino**
Stores voltage at 0 and 90 degrees. Should be done prior to using device.

5. **flex_test_uno.ino**
Sketch used for basic flex sensor testing while flex sensor is connected to Arduino Uno via breadboard. Moving average is used to give more accurate voltage readings.

6. **flex_sensor1.ino**
Sketch stores voltages and logs time as flex sensor is bent. Sketch considers event to be anytime voltage is different from resting voltage. Sketch logs running timer and voltage values on raw data file only when event is occurring (so it doesn't log all data), and times of each event on separate event file.
Outputs: resting.txt --> resting voltage value
         raw_data.txt --> elapsed time and voltages for each event
         events.txt --> Time of each finger event



