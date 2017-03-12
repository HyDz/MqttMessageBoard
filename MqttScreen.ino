/*
 * Mqtt Message Board
 *
 * Created: 12/3/2017 
 * Author: HyDz
 *
 * MQTT Message Board With ST7735 SCREEN
 */ 

#include <ESP8266WiFi.h>

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define WLAN_SSID       "Your SSID"
#define WLAN_PASS       "SSID Password"

ESP8266WebServer server(80);


/*/ config static IP
IPAddress ip(192, 168, 1, 115); // where xx is the desired IP Address
IPAddress gateway(192, 168, 1, 254); // set gateway to match your network
IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network
*/

#define AIO_SERVER      "Your MQTT Broker adress"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "MQTT Username"
#define AIO_KEY         "MQTT PASSWORD"


// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;


// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;
const char MQTT_CLIENTID[] PROGMEM  = "ESP_Message_Screen";
const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM  = AIO_KEY;

//Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);


const char MESS_FEED[] PROGMEM = "message";

Adafruit_MQTT_Subscribe message = Adafruit_MQTT_Subscribe(&mqtt, "message");
Adafruit_MQTT_Subscribe allfeed = Adafruit_MQTT_Subscribe(&mqtt, "#");



// For the breakout, you can use any 2 or 3 pins
// These pins will also work for the 1.8" TFT shield
#define TFT_CS     15
#define TFT_RST    5  // you can also connect this to the Arduino reset
                      // in which case, set this #define pin to 0!
#define TFT_DC     2

// Option 1 (recommended): must use the hardware SPI pins
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)


Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
int attempt = 0;
int cursorVertPos = 0;
int cursorHoriPos = 0;

void MQTT_connect();


void setup(void) {

  Serial.begin(115200);
  // Use this initializer if you're using a 1.8" TFT
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
tft.fillScreen(ST7735_BLACK);
testdrawtext("Shit Started", ST7735_YELLOW, 40, 70);
delay(500);
tft.fillScreen(ST7735_YELLOW);
/*
   uint16_t time = millis();
  tft.fillScreen(ST7735_BLACK);
  time = millis() - time;
*/
// WiFi.config(ip, gateway, subnet);

 WiFi.begin(WLAN_SSID, WLAN_PASS);
  
 while (WiFi.status() != WL_CONNECTED) {
    testdrawtext("Connecting Wifi", ST7735_GREEN, 20, attempt);
    delay(2000);
    attempt = attempt +9; 
    if (attempt>=120){tft.fillScreen(ST7735_BLACK);attempt=0;return;}
//    testdrawtext(WiFi.status(), ST7735_GREEN, 0, attempt);
  //  testdrawtext("Connecting Wifi", ST7735_GREEN, 20, attempt);
    }
   tft.fillScreen(ST7735_BLACK);
 testdrawtext("Wifi OK", ST7735_GREEN, 5, 5);
MDNS.begin("esp8266");
 server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  mqtt.subscribe(&message);
   mqtt.subscribe(&allfeed);
testdrawtext("Setup End", ST7735_GREEN, 0, 20);
delay(666);
   tft.fillScreen(ST7735_BLACK);

 }

uint32_t x=0;
 
void loop(){
  server.handleClient();
  MQTT_connect();
 // tft.fillScreen(ST7735_BLACK);
  testdrawtext("MQTT OK", ST7735_WHITE, 30, 0);
 // delay(1000);
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &message) {
      cursorHoriPos = cursorHoriPos + 10;
      if (cursorHoriPos >= 120){
       tft.fillScreen(ST7735_BLACK);
       cursorHoriPos = 0;
       testdrawtext((char *)message.lastread, ST7735_GREEN, 0, cursorHoriPos);
      }
        testdrawtext((char *)message.lastread, ST7735_GREEN, 0, cursorHoriPos);    
    }
     if (subscription == &allfeed) {
       testdrawtext((char *)allfeed.lastread, ST7735_GREEN, 0, 120);
      }
     
  }
if(! mqtt.ping()) {
    mqtt.disconnect();
  }
}

void testdrawtext(char *text, uint16_t color, int cursorVertPos, int cursorHoriPos) {
  tft.setCursor(cursorVertPos, cursorHoriPos);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}


/*
void concate(char *a, char *b){
char concated;
concat(a, b, concated);
}
*/
void MQTT_connect() {
  int8_t ret;


  // Stop if already connected.

  if (mqtt.connected()) {
    //testdrawtext("MQTT CONNECTED", ST7735_YELLOW, 10, 155);
//delay(2000);
    return;
  }
testdrawtext("MQTT CONNECT", ST7735_BLUE, 10, 135);

 uint8_t retries = 20;
 
 while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       mqtt.disconnect();
       testdrawtext("MQTT ERROR", ST7735_RED, 30, 80);
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         testdrawtext("MQTT ERROR", ST7735_RED, 30, 80);
       delay(5000);  // wait 5 seconds
         // basically die and wait for WDT to reset me
         while (1);
       } 
  }
testdrawtext("MQTT CONNECTED", ST7735_YELLOW, 10, 135);
//delay(2000);
}

void handleRoot() {
 // digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp8266!");
//  digitalWrite(led, 0);
}

void handleNotFound(){
//  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
   for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
//  digitalWrite(led, 0);
}

