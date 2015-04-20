/*

Copyright (c) 2012, 2013 RedBearLab

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

//"RBL_nRF8001.h/spi.h/boards.h" is needed in every new project
#include <SPI.h>
#include <EEPROM.h>
#include <boards.h>
#include <RBL_nRF8001.h>

//BPM setup
//  Variables
int pulsePin = 0;                 // Pulse Sensor purple wire connected to analog pin 0
int blinkPin = 13;                // pin to blink led at each beat
// Volatile Variables, used in the interrupt service routine!
volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded! 
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat". 
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

// Regards Serial OutPut  -- Set This Up to your needs
static boolean serialVisual = false;   // Set to 'false' by Default.  Re-set to 'true' to see Arduino Serial Monitor ASCII Visual Pulse 


void setup()
{
  // Default pins set to 9 and 8 for REQN and RDYN
  // Set your REQN and RDYN here before ble_begin() if you need
  //ble_set_pins(3, 2);
  
  // Set your BLE Shield name here, max. length 10
  //ble_set_name("BIOScan");
  
  // Init. and start BLE library.
  ble_begin();
  
  // Enable serial debug
  Serial.begin(57600);

  //BPM setup
  pinMode(blinkPin,OUTPUT);         // pin that will blink to your heartbeat!
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS 
}

void loop()
{
  static boolean analog_enabled = false;
  // If data is ready
  bool ble_was_read = false;
  while(ble_available())
  {
	ble_was_read = true;
    // read out command and data
    //byte data0 = ble_read();
	Serial.print((char)ble_read());
  }
  if(ble_was_read) {
	  Serial.println();
  }
  while(Serial.available()) {
	  ble_write(Serial.read());
  }
  
  // Allow BLE Shield to send/receive data
  ble_do_events();  

  //BPM Loop
  if (QS == true){     //  A Heartbeat Was Found
	  // BPM and IBI have been Determined
	  // Quantified Self "QS" true when arduino finds a heartbeat
	  digitalWrite(blinkPin,HIGH);     // Blink LED, we got a beat. 
	  serialOutputWhenBeatHappens();   // A Beat Happened, Output that to serial.     
	  QS = false;                      // reset the Quantified Self flag for next time    
  } 
  else { 

	  digitalWrite(blinkPin,LOW);            // There is not beat, turn off pin 13 LED
  }
}

