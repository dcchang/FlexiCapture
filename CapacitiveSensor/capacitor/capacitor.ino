#include "CapacitiveSensor.h"

/*
 * CapitiveSense Library Demo Sketch
 * Paul Badger 2008
 * Uses a high value resistor e.g. 10M between send pin and receive pin
 * Resistor effects sensitivity, experiment with values, 50K - 50M. Larger resistor values yield larger sensor values.
 * Receive pin is the sensor pin - try different amounts of foil/metal on this pin
 */
#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
  // Required for Serial on Zero based boards
  #define Serial SERIAL_PORT_USBVIRTUAL
#endif


CapacitiveSensor   cs_8_2 = CapacitiveSensor(3,9);        // 10M resistor between pins 4 & 2, pin 2 is sensor pin, add a wire and or foil if desired

void setup()                    
{
   cs_8_2.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example
   Serial.begin(9600);

   pinMode(7, OUTPUT);
}

void loop()                    
{
//  digitalWrite(7, HIGH);
//  delay(500);
//  digitalWrite(7, LOW);
//  delay(500);
//    long start = millis();
    long total1 =  cs_8_2.capacitiveSensor(30);

//    Serial.println(millis() - start);        // check on performance in milliseconds

    Serial.println(total1);                  // print sensor output 1 - this is also the value you can use to use this in other projects

//    delay(100);                             // arbitrary delay to limit data to serial port 
}
