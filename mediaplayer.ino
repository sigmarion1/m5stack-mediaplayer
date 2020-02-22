
#include <M5Stack.h>
#include <WiFi.h>
#include "time.h"
#include "AudioFileSourceSD.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

struct Files
{
  String addr;
  Files *next;
};

Files *firstImage = NULL;
Files *currentImage = NULL;
Files *firstMusic = NULL;
Files *currentMusic = NULL;


bool isPlaying = false;
unsigned long currentMillis;


String wifi_ssid = "x";
String wifi_password = "x";

const char* ntpServer = "kr.pool.ntp.org";
const long  gmtOffset_sec = 32400; 
const int   daylightOffset_sec = 0; 

AudioGeneratorMP3 *mp3;
AudioFileSourceSD *file;
AudioOutputI2S *out;
AudioFileSourceID3 *id3;




// The setup() function runs once each time the micro-controller starts
void setup() {

  M5.begin();
  
  /*
    Power chip connected to gpio21, gpio22, I2C device
    Set battery charging voltage and current
    If used battery, please call this function in your project
  */
  M5.Power.begin();

  M5.Lcd.setBrightness(20);

  /*
  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(65, 10);
  M5.Lcd.println("Button example");
  M5.Lcd.setCursor(3, 35);
  M5.Lcd.println("Press button B for 700ms");
  M5.Lcd.println("to clear screen.");
  M5.Lcd.setTextColor(RED);
*/
  /*
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  if(checkConnection()){
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();
  }
  WiFi.disconnect();
  */

//  M5.Lcd.drawJpgFile(SD, "/p2.jpg");

  M5.Lcd.print("Total File Count :");
  M5.Lcd.print(createFileList());
  
}


String parseExt(String str){
  String output = "";

  for (int i = 0; i < str.length(); i++)
  {
    if((char)str[i] == '.'){
      output = "";
    }
    else{
      output += (char)str[i];
    }
  }
  return output;
}

char* strToChar(String str) {
  int len = str.length() + 1;
  char* buf = new char[len];
  strcpy(buf, str.c_str());
  return buf;
}

int createFileList() {
  int i = 0;
  File root = SD.open("/");
  if (root)
  {
    while (true)
    {
      File entry =  root.openNextFile();
      if (!entry) break;
      if (!entry.isDirectory())
      {
        String ext = parseExt(entry.name());


        if (ext == "MP3" || ext == "mp3") 
        {
          Files *tmp = new Files;
          tmp->addr = entry.name();
          M5.Lcd.print(entry.name());
          
          if (firstMusic == NULL)
          {
            firstMusic = tmp;
            currentMusic = tmp;
          }
          else{
            currentMusic->next = tmp;
            currentMusic = currentMusic->next;
          }

          i++;
        }

        else if (ext == "JPG" || ext =="jpg")
        {
          Files *tmp = new Files;
          tmp->addr = entry.name();
          M5.Lcd.print(entry.name());

          if (firstImage == NULL){
            firstImage = tmp;
            currentImage = tmp;
          }
          else{
            currentImage->next = tmp;
            currentImage = currentImage->next;
          }
          i++;
        }
      }
      entry.close();
    }
  }

    if (firstMusic != NULL){
      currentMusic->next = firstMusic;
    }

    if (firstImage != NULL){
      currentImage->next = firstImage;
    }

  root.close();
  return i;
}



bool checkConnection() {
  int count = 0;
  Serial.print("Waiting for Wi-Fi connection");
  M5.Lcd.print("Waiting for Wi-Fi connection");
  while ( count < 30 ) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      M5.Lcd.println();
      Serial.println("Connected!");
      M5.Lcd.println("Connected!");
      return (true);
    }
    delay(500);
    Serial.print(".");
    M5.Lcd.print(".");
    count++;
  }
  Serial.println("Timed out.");
  M5.Lcd.println("Timed out.");
  return false;
}

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    M5.Lcd.println("Failed to obtain time");
    return;
  }
  M5.Lcd.println(&timeinfo, "%H:%M:%S");
}

void playMP3(char *filename){
  file = new AudioFileSourceSD(filename);
  id3 = new AudioFileSourceID3(file);
  out = new AudioOutputI2S(0, 1); // Output to builtInDAC
  out->SetOutputModeMono(true);
  out->SetGain(0.5);
  mp3 = new AudioGeneratorMP3();
  mp3->begin(id3, out);
  isPlaying = true;
}
 
// Add the main program code into the continuous loop() function
void loop() {

  if(isPlaying){
    if(mp3->isRunning()) {
      if (!mp3->loop()){
        mp3->stop();
        isPlaying = false;
      }
    }
  }

  // update button state
    M5.update();


  // if you want to use Releasefor("was released for"), use .wasReleasefor(int time) below
  if (M5.BtnA.wasReleased()) {
    if(isPlaying){
      mp3->stop();
      isPlaying = false;
    }
    currentMusic = currentMusic->next;
    playMP3(strToChar(currentMusic->addr));
    /*
    if(isPlaying){
      mp3->stop();
      isPlaying = false;
    }
    */

  } else if (M5.BtnB.wasReleased()) {
    if(isPlaying){
      mp3->stop();
      isPlaying = false;
    }
    currentImage = currentImage->next;
    M5.Lcd.clear(BLACK);
    M5.Lcd.drawJpgFile(SD, strToChar(currentImage->addr), 0, 0);

  } else if (M5.BtnC.wasReleased()) {
      if(isPlaying){
      mp3->stop();
      isPlaying = false;
    }

    M5.Lcd.clear(BLACK);
    M5.Lcd.setCursor(40, 120);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.setTextSize(3);
    printLocalTime();
  }
  
}
