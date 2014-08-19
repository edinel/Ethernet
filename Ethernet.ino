/*--------------------------------------------------------------
  Program:      eth_websrv_page

  Description:  Arduino web server that serves up a basic web
                page. Does not use the SD card.
  
  Hardware:       Arduino Uno and official Arduino Ethernet
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
#define REQ_BUF_SZ   100 




const boolean FALSE = 0;
const boolean TRUE = 1;

byte mac[] = { 0x90, 0xA2, 0xDA, 0x0F, 0xDC, 0xD4 }; // MAC address from Ethernet shield sticker under board
IPAddress ip(172, 16, 30, 186); // Just grab 172.16.30.185.  I ought to figure out how to #define this.  Someday.  
const int chipSelect = 4;
EthernetServer server(80);  // create a server at port 80
File webFile; 


/* 
 *Add some LED stuff to respond to the web page activity.
 */

//RGB LED pins
int ledDigitalOne[] = {
  2, 5, 6}; //the three digital pins of the digital LED 
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
// Open serial communications and wait for port to open:
  Serial.begin(9600);
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for Leonardo only
  //}
  Serial.println("Starting Ethernet...");
  Ethernet.begin(mac, ip);  // initialize Ethernet device
  server.begin();           // start to listen for client  
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
  initLED();
}


void loop()
{
  /*
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  // print out the value you read:
  Serial.println(sensorValue);
  delay(100);        // delay in between reads for stability
*/
//setColorByName(ledDigitalOne, RED);
//delay (500); 

  EthernetClient client = server.available();  // try to get client
  char HTTP_req[REQ_BUF_SZ] = {0}; // buffered HTTP request stored as null terminated string
  char req_index = 0;              // index into HTTP_req buffer
  StrClear(HTTP_req, REQ_BUF_SZ);
 

  if (client) {  // got client?
    //setColorByName(ledDigitalOne, BLACK);
    Serial.println("Got a client"); 
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {   // client data available to read
        char c = client.read(); // read 1 byte (character) from client
        if (req_index < (REQ_BUF_SZ - 1)) {
          HTTP_req[req_index] = c;          // put the http request into the buffer
          req_index++;
        }
        Serial.print(c); // print it out also.  
        if (c == '\n' && currentLineIsBlank) {       // If we get a newline which FOLLOWED another newline, we're done with the http request.  
          client.println("HTTP/1.1 200 OK");         // So start printing out 
          client.println("Content-Type: text/html"); // the HTTP response.  
          client.println("Connection: close");       // 
          client.println(); 
          if (StrContains(HTTP_req, "favicon.ico")){ // ignore favicon.ico requests.  Le Sigh.  
          }else if (StrContains(HTTP_req, "green")){
            setColorByName(ledDigitalOne, WHITE); 
          }else if (StrContains(HTTP_req, "red")){
            setColorByName(ledDigitalOne, RED);
          }else if (StrContains(HTTP_req, "blue")){
            setColorByName(ledDigitalOne, BLUE);
          }else{
            setColorByName(ledDigitalOne, BLACK);
          }
          webFile = SD.open("index.htm");        // open web page file
          if (webFile) {
            while(webFile.available()) {
              client.write(webFile.read()); // send web page to client
            }
            webFile.close();
          }
          //req_index = 0;
          //StrClear(HTTP_req, REQ_BUF_SZ);
          break;
        }
        if (c == '\n') {           //we got a blank line.
             currentLineIsBlank = true;
        } else if (c != '\r') {     // we got a text character SOMEWHERE on the line.
          currentLineIsBlank = false;
        }
      } // end if (client.available())
    } // end while (client.connected())
    delay(1);      // give the web browser time to receive the data
    client.stop(); // close the connection
    Serial.println("Client disconnected");
    } // end if (client)
}

/*
void ProcessCheckbox(EthernetClient cl){
  if (HTTP_req.indexOf("LED2=2") > -1) {  // see if checkbox was clicked
  // the checkbox was clicked, toggle the LED
    if (LED_status) {
      LED_status = 0;
    } else {
      LED_status = 1;
    }
  }
}
*/

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

void initLED(){
  for(int i = 0; i < 3; i++){
    pinMode(ledDigitalOne[i], OUTPUT);
    setColorByName(ledDigitalOne, BLACK);   //Set the three LED pins as outputs
  } 
}

/*
 * char StrClear (char* string, int length) 
 * Pulled from the Arduino example; looks to be awfully useful for dealing with any kind of text passed around
 * Success appears to return 1, failure returns 0.  (need to validate this one, obvs).  
 */
void StrClear(char *str, char length)
{
    for (int i = 0; i < length; i++) {
        str[i] = 0;
    }
}

/*
 * char StrContains (string, string-to-find) 
 * Pulled from the Arduino example; looks to be awfully useful for dealing with any kind of text passed around
 * Success appears to return 1, failure returns 0.  (need to validate this one, obvs).  
 */
 
char StrContains(char *str, char *sfind){
  char found = 0;
  char index = 0;
  char len;
  len = strlen(str);
  if (strlen(sfind) > len) {
    return 0;
  }
  while (index < len) {
    if (str[index] == sfind[found]) {
      found++;
      if (strlen(sfind) == found) {
        return 1;
      }
    } else {
      found = 0;
    }
    index++;
  }
  return 0;
}

