/*--------------------------------------------------------------
  Program:      eth_websrv_page

  Description:  Arduino web server that serves up a basic web
                page. Does not use the SD card.
  
  Hardware:     Arduino Uno and official Arduino Ethernet
                shield. Should work with other Arduinos and
                compatible Ethernet shields.
                
  Software:     Developed using Arduino 1.0.3 software
                Should be compatible with Arduino 1.0 +
  
  References:   - WebServer example by David A. Mellis and 
                  modified by Tom Igoe
                - Ethernet library documentation:
                  http://arduino.cc/en/Reference/Ethernet

  Date:         7 January 2013
 
  Author:       W.A. Smith, http://startingelectronics.com
--------------------------------------------------------------*/

#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>


const boolean FALSE = 0;
const boolean TRUE = 1;

byte mac[] = { 0x90, 0xA2, 0xDA, 0x0F, 0xDC, 0xD4 }; // MAC address from Ethernet shield sticker under board
IPAddress ip(172, 16, 30, 185); // Just grab 172.16.30.185.  I ought to figure out how to #define this.  Someday.  
EthernetServer server(80);  // create a server at port 80
File webFile; 
const int chipSelect = 4;


/* 
 *Add some LED stuff so we can be sure things are running
 */

//RGB LED pins
int ledDigitalOne[] = {
  5, 6, 7}; //the three digital pins of the digital LED 
  //5 = redPin, 6 = greenPin, 7 = bluePin

const boolean ON = LOW;     //Define on as LOW (this is because we use a common Anode RGB LED (common pin is connected to +5 volts)
const boolean OFF = HIGH;   //Define off as HIGH

//Predefined Colors
const boolean RED[] = {
  ON, OFF, OFF};    
const boolean GREEN[] = {
  OFF, ON, OFF}; 
const boolean BLUE[] = {
  OFF, OFF, ON}; 
const boolean YELLOW[] = {
  ON, ON, OFF}; 
const boolean CYAN[] = {
  OFF, ON, ON}; 
const boolean MAGENTA[] = {
  ON, OFF, ON}; 
const boolean WHITE[] = {
  ON, ON, ON}; 
const boolean BLACK[] = {
  OFF, OFF, OFF}; 

//An Array that stores the predefined colors (allows us to later randomly display a color)
const boolean* COLORS[] = {
  RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE, BLACK
};  //OK so this is an array of arrays (of pointers?)


/*
 *LED Stuff
 */
 
boolean isLit = FALSE;
boolean err = FALSE;
long previousMillis = 0;
long interval = 250; 
int ColorIndex = 0;



void setup()
{
    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for client 
                
// Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
    Serial.println("OK, card initialized.");
    // check for index.htm file
    if (!SD.exists("index.htm")) {
      Serial.println("ERROR - Can't find index.htm file!");
      err = TRUE; 
      return;  // can't find index file
    }
    Serial.println("SUCCESS - Found index.htm file.");
}



void loop()
{
/* LED Stuff
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > interval) { // save the last time you blinked the LED 
  previousMillis = currentMillis;
  if (!err) { ChangeLEDState(); }
  }
    
    */
  EthernetClient client = server.available();  // try to get client

    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close");
                    client.println();
                    // send web page
                    webFile = SD.open("index.htm");        // open web page file
                    if (webFile) {
                        while(webFile.available()) {
                            client.write(webFile.read()); // send web page to client
                        }
                        webFile.close();
                    }
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)
}


void ChangeLEDState(){
  if (isLit == FALSE){
    //SetRandomColor();
    SetNextColor();
    isLit = TRUE; 
  }
  else
  {
    setColorByName (ledDigitalOne, BLACK);
    isLit = FALSE; 
  }
}


/* Sets an led to any color
 led - a three element array defining the three color pins (led[0] = redPin, led[1] = greenPin, led[2] = bluePin)
 color - a three element boolean array (color[0] = red value (LOW = on, HIGH = off), color[1] = green value, color[2] =blue value)
 */
void setColor(int* led, boolean* color){
  for(int i = 0; i < 3; i++){
    digitalWrite(led[i], color[i]);
  }

}

/* A version of setColor that allows for using const boolean colors
 *
 */
void setColorByName(int* led, const boolean* color){
  boolean tempColor[] = {
    color[0], color[1], color[2]  };
  setColor(led, tempColor);
}


void SetRandomColor(){
  int rand = random() % 7; // There are 7 non-black colors in the array, after all.  
  setColorByName(ledDigitalOne, COLORS[rand]);  //Set the color of led one to a random color
}


void SetNextColor(){
  ColorIndex++; 
  ColorIndex = ColorIndex % 3; // Red, Green, Blue.  
  setColorByName(ledDigitalOne, COLORS[ColorIndex]);  //Set the color of led one to the color chosen.
}


