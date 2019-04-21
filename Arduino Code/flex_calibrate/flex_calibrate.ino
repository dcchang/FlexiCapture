//Stores voltage at 0 and 90 degrees. Should be done prior to using device.

#include <SPI.h>
#include <SD.h>


//for printing to serial monitor with arduino zero board
#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
  // Required for Serial on Zero based boards
  #define Serial SERIAL_PORT_USBVIRTUAL
#endif

File threshold_file;  //file to store 0 and 90 degree voltage values

const float VCC = 3.7;  //input voltage from battery
const float r1 = 9.75;  //resistance of r1 in voltage divider in kohms

//variables used to calculate voltage values
int total;
int flexADC;
int average_0;
int average_90;
float voltage_0;
float voltage_90;

const int chipSelect = 10;  //For writing to SD card
const int FLEX_PIN = A1;  //analog pin to read in voltage values from voltage divider circuit
const int LED = 7;  //digital pin for LED light

void setup() {

  //initialize Serial monitor
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  pinMode(10, OUTPUT);
  pinMode(FLEX_PIN, INPUT);
  pinMode(LED, OUTPUT);

  //set up SD card
  if (!SD.begin(SPI_HALF_SPEED,10)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  //create text file to store 0 and 90 degree voltage values on SD card
  threshold_file = SD.open("init.txt",FILE_WRITE);

  //Get voltage value at 0 degrees 

  //LED light blinks twice to alert user that calibration is about to begin
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);

  //tells user to prepare for calibration if they are using Serial monitor. 5 second delay before calibration starts
  Serial.println("Place flex sensor completely flat at 0 degrees. Getting voltage value at 0 degrees in 5 seconds."); 
  delay(5000);


  //Get the average value at 0 degrees by reading in the voltage 5000 times
  Serial.println("Beginning calibration for 0 degrees...");
  total = 0;
  for (int i=0; i<5000; i++) {
    flexADC = analogRead(FLEX_PIN);
    total += flexADC;
    digitalWrite(LED, HIGH); //LED light is on while calibration is occuring. 
    delay(1);
  }
  digitalWrite(LED, LOW); //turn LED off
  
  average_0 = total/5000;
  voltage_0 = average_0 * VCC / 1023.0; //calculate voltage
  Serial.println("Flat Voltage: " + String(voltage_0));
  threshold_file.println(voltage_0);  //save voltage value to SD card
  
  //Get voltage value at 90 degrees

  //LED light blinks twice to alert user that calibration is about to begin
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
  
  //tells user to prepare for calibration if they are using Serial monitor. 5 second delay before calibration starts
  Serial.println("Bend flex sensor at exactly 90 degrees. Getting voltage value at 90 degrees in 5 seconds.");
  delay(5000);

  //Get the average value at 90 degrees by reading in the voltage 5000 times
  Serial.println("Beginning calibration for 90 degrees...");
  total = 0;
  for (int i=0; i<5000; i++) {
    flexADC = analogRead(FLEX_PIN);
    total += flexADC;
    delay(1);
  }
  average_90 = total/5000;
  voltage_90 = average_90 * VCC / 1023.0; //calculate voltage
  Serial.println("Bend Voltage: " + String(voltage_90));
  threshold_file.println(voltage_90);  //save voltage value to SD card
  threshold_file.close();
}

void loop() {

  //start blinking faster once calibration is complete
  digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);                       // wait for 0.1 seconds
  digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
  delay(100);                       // wait for 0.1 seconds
}
