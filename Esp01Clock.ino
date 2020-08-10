#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "WiFiManager.h"          //https://github.com/tzapu/WiFiManager

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example


#include "NTP.h"
#include <Time.h>
#include <Timezone.h>
// 北京时间时区
#define STD_TIMEZONE_OFFSET +8
TimeChangeRule mySTD = {"", First,  Sun, Jan, 0, STD_TIMEZONE_OFFSET * 60};
Timezone myTZ(mySTD, mySTD);
time_t previousMinute = 0;


void setup() {
  Serial.begin(115200);
  //esp01 GPIO0 和 GPIO2 充当IIC总线
  Wire.begin(0, 2);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds
  // Clear the buffer
  display.clearDisplay();

  wifiStatus("WIFI CONNECTING");

  WiFiManager wifiManager;

  wifiManager.setAPCallback(configModeCallback);
  if (!wifiManager.autoConnect("Esp01Clock")) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  wifiStatus("WIFI CONNECTED");

  initNTP();

}

void loop() {

  //  Update the display only if time has changed
  if (timeStatus() != timeNotSet) {
    if (minute() != previousMinute) {
      previousMinute = minute();
      // Update the display
      updateDisplay();
    }
  }
  delay(500);
}
void updateDisplay(void) {

  TimeChangeRule *tcr;        // Pointer to the time change rule

  // Read the current UTC time from the NTP provider
  time_t utc = now();

  // Convert to local time taking DST into consideration
  time_t localTime = myTZ.toLocal(utc, &tcr);

  // Map time to pixel positions
  int weekdays =   weekday(localTime);
  int days    =   day(localTime);
  int months  =   month(localTime);
  int years   =   year(localTime);
  int seconds =   second(localTime);
  int minutes =   minute(localTime);
  int hours   =   hour(localTime) ;   //12 hour format use : hourFormat12(localTime)  isPM()/isAM()

  /////process time///////
  String h = "";
  String m = "";
  String s = "";
  if (hours < 10 ) {
    h = "0" + String(hours);
  } else {
    h = String(hours);
  }
  if (minutes < 10 ) {
    m = "0" + String(minutes);
  }
  else {
    m = String(minutes);
  }
  if (seconds < 10 ) {
    s = "0" + String(seconds);
  } else {
    s = String(seconds);
  }
  /////process date///////
  String mm = "";
  String dd = "";
  String w = dayStr(weekdays);

  if (months < 10 ) {
    mm = "0" + String(months);
  } else {
    mm = String(months);
  }

  if (days < 10 ) {
    dd = "0" + String(days);
  } else {
    dd = String(days);
  }
  String myTime = h + ":" + m ;
  String myDate = mm + "/" + dd ;
  String myWeek = w.substring(0, 3);
  Serial.println("-----------------");
  Serial.println("myTime: " + myTime);
  Serial.println("myDate: " + myDate);
  Serial.println("myWeek: " + myWeek);

  //////process display////////
  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(5, 10);
  display.println(myTime);

  display.setCursor(8, 50);
  display.setTextSize(2);
  display.print(myDate);
  display.setCursor(80, 50);
  display.print(myWeek);
  display.display();
}
void configModeCallback (WiFiManager *myWiFiManager) {
  wifiStatus("WIFI CONNECT FAIL");
}

void wifiStatus(String myWiFiStatus) {
  display.clearDisplay();
  display.setCursor(0, 30);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(myWiFiStatus);
  display.display();
}
