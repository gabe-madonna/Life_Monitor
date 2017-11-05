#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioPlaySdWav           playSdWav1;
AudioOutputI2S           i2s1;
AudioConnection          patchCord1(playSdWav1, 0, i2s1, 0);
AudioConnection          patchCord2(playSdWav1, 1, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

String message = "";
int sound_state = 0;
int sound_substate = 0;
String speech = "";
String send_users = "";

File myFile;
void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);
  AudioMemory(8);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
  delay(1000);
}

void loop() {
    if(message.indexOf("@")!= -1){
      message = message.substring(0,message.indexOf("@"));
      int message_hash = message.indexOf("#");
      String keyword = message.substring(0,message_hash);
      Serial.println(message);
      //store#(bpm)#(lat)#(lon)#(time)
//      if(keyword == "store"){
//        int bpm_end = message.indexOf("#", message_hash + 1);
//        int lat_end = message.indexOf("#", bpm_end + 1);
//        int lon_end = message.indexOf("#", lat_end + 1);
//        String bpm = message.substring(message_hash + 1, bpm_end);
//        String lat = message.substring(bpm_end + 1, lat_end);
//        String lon = message.substring(lat_end + 1, lon_end);
//        String time_stamp = message.substring(lon_end + 1, message.length());
//        myFile = SD.open("no_wifi_data.txt", FILE_WRITE);
//        if(myFile){
//          Serial.print("Writing to file...");
//          myFile.println(
//        }
//      }
      if(keyword == "save"){
        int username_end = message.indexOf("#",message_hash + 1);
        String user = message.substring(message_hash + 1, username_end);
        if(user == "dimitral"){
          myFile.remove("dimitral.txt");
          
        }
        else if(user == "gdolphin"){
          myFile.remove("gdolphin.txt");
        }
        else if(user == "pkarnati"){
          myFile.remove("pkarnati.txt");
        }
        else if(user == "gmadonna"){
          myFile.remove("pkarnati.txt");
        }
        
      }
      if(keyword == "user"){
        String username = message.substring(message_hash + 1, message.length());
        if(username == "dimitral"){
          myFile = SD.open("dimitral.txt");
        }
        else if(username == "gdolphin"){
          myFile = SD.open("gdolphin.txt");
        }
        else if(username == "pkarnati"){
          myFile = SD.open("pkarnati.txt");
        }
        else if(username == "gmadonna"){
          myFile = SD.open("gmadonna.txt");
        }
        if(myFile){
          Serial.print(username);
          Serial.println(".txt");
          while(myFile.available()){
            char byte_from_sd = myFile.read();
            send_users.concat(byte_from_sd);
          }
        }
        myFile.close();
        send_users = username + "#" + send_users + "@";
        Serial1.print(send_users);
        Serial.println(send_users);
        send_users = "";
      }
      if(keyword == "time"){
        int hour_end = message.indexOf(":");
        int minute_end = message.indexOf(" ");
        String the_hour = message.substring(message_hash + 1, hour_end);
        String the_minute = message.substring(hour_end + 1, minute_end);
        String am_or_pm = message.substring(minute_end + 1, message.length());
        sound_state = 1;
        speech = "";
        while(sound_state != 0){
          if(sound_state == 1){
            speech = the_hour + ".WAV";
            playSdWav1.play(speech);
            sound_state = 2;
            delay(10);
          }
          else if(sound_state == 2){
            speech = the_minute + ".WAV";
            playSdWav1.play(speech);
            sound_state = 3;
            delay(10);
          }
          else if(sound_state == 3){
            speech = am_or_pm + ".WAV";
            playSdWav1.play(speech);
            sound_state = 0;
            delay(10);
          }
          while(playSdWav1.isPlaying() == true){}
        }
      }
      if(keyword == "calories"){
        int decimal = message.indexOf(".");
        String cal_so_far_int = message.substring(message_hash + 1,decimal);
        String cal_so_far_dec = message.substring(decimal + 1, message.length());
        sound_state = 1;
        speech = "";
        while(sound_state != 0){
          if(sound_state == 1){
            large_numbers(cal_so_far_int);
            sound_state = 2;
          }
          else if(sound_state == 2){
            playSdWav1.play("point.WAV");
            delay(10);
            sound_state = 3;
          }
          else if(sound_state == 3){
            speech = cal_so_far_dec + ".WAV";
            playSdWav1.play(speech);
            delay(10);
            sound_state = 4;
          }
          else if(sound_state == 4){
            playSdWav1.play("calories.WAV");
            delay(10);
            sound_state = 0;
          }
          while(playSdWav1.isPlaying() == true){}
        }
      }
      if(keyword == "steps"){
        String steps_so_far = message.substring(message_hash + 1,message.length());
        sound_state = 1;
        speech = "";
        while(sound_state != 0){
          if(sound_state == 1){
            large_numbers(steps_so_far);
            sound_state = 2;
          }
          else if(sound_state == 2){
            playSdWav1.play("steps.WAV");
            delay(15);
            sound_state = 0;
          }
          while(playSdWav1.isPlaying() == true){}
        }
        
      }
      if(keyword == "hrate"){
        String hrate = message.substring(message_hash + 1,message.length());
        sound_state = 1;
        speech = "";
        while(sound_state != 0){
          if(sound_state == 1){
            playSdWav1.play("hrate.WAV");
            delay(10);
            sound_state = 2; 
          }
          else if(sound_state == 2){
            large_numbers(hrate);
            delay(10);
            sound_state = 3;
          }
          else if(sound_state == 3){
            playSdWav1.play("BPM.WAV");
            delay(10);
            sound_state = 0;
          }
          while(playSdWav1.isPlaying() == true){}
        } 
      }
      if(keyword == "pace"){
        if(message.indexOf("Unavailable") == -1){
          int minutes_end = message.indexOf(":");
          String minutes = message.substring(message_hash + 1,minutes_end);
          String seconds = message.substring(minutes_end + 1,message.length());
          sound_state = 1;
          speech = "";
          while(sound_state != 0){
            if(sound_state == 1){
              playSdWav1.play("milepace.WAV");
              delay(10);
              sound_state = 2; 
            }
            else if(sound_state == 2){
              speech = minutes + ".WAV";
              playSdWav1.play(speech);
              delay(10);
              sound_state = 3;
            }
            else if(sound_state == 3){
              speech = seconds + ".WAV";
              playSdWav1.play(speech);
              delay(10);
              sound_state = 0;
            }
            while(playSdWav1.isPlaying() == true){}
          } 
        }
        else{
          sound_state = 1;
          while(sound_state != 0){
            if(sound_state == 1){
              playSdWav1.play("milepace.WAV");
              delay(10);
              sound_state = 2; 
            }
            else if(sound_state == 2){
              Serial.println("hi");
              playSdWav1.play("na.WAV");
              delay(10);
              sound_state = 0;
            }
            while(playSdWav1.isPlaying() == true){}
          }
        }
         
      }
      if(keyword == "dist"){
        int period = message.indexOf(".");
        String dist_int = message.substring(message_hash + 1, period);
        String dist_dec = message.substring(period + 1, message.length());
        sound_state = 1;
        speech = "";
        while(sound_state != 0){
          if(sound_state == 1){
            playSdWav1.play("distance.WAV");
            delay(10);
            sound_state = 2; 
          }
          else if(sound_state == 2){
            large_numbers(dist_int);
            sound_state = 3;
          }
          else if(sound_state == 3){
            playSdWav1.play("point.WAV");
            delay(10);
            sound_state = 4;
          }
          else if(sound_state == 4){
            if(dist_dec == "00"){
              dist_dec = "0";
            }
            speech = dist_dec + ".WAV";
            playSdWav1.play(speech);
            delay(10);
            sound_state = 5;
          }
          else if(sound_state == 5){
            playSdWav1.play("miles.WAV");
            delay(10);
            sound_state = 0;
          }
          while(playSdWav1.isPlaying() == true){}
        } 
      }
      if(keyword == "end"){
        int dist_period = message.indexOf(".");
        int cal_period = message.indexOf(".", dist_period + 1);
        int workout_dist_end = message.indexOf("#", message_hash + 1);
        String dist_int = message.substring(message_hash + 1, dist_period);
        String dist_dec = message.substring(dist_period + 1, workout_dist_end);
        String cals_int = message.substring(workout_dist_end + 1, cal_period );
        String cals_dec = message.substring(cal_period + 1, message.length());
        sound_state = 1;
        speech = "";
        while(sound_state != 0){
          if(sound_state == 1){
            playSdWav1.play("distance.WAV");
            delay(10);
            sound_state = 2;
          }
          else if(sound_state == 2){
            large_numbers(dist_int);
            sound_state = 3;
          }
          else if(sound_state == 3){
            playSdWav1.play("point.WAV");
            delay(10);
            sound_state = 4;
          }
          else if(sound_state == 4){
            if(dist_dec == "00"){
              dist_dec = "0";
            }
            speech = dist_dec + ".WAV";
            playSdWav1.play(speech);
            delay(10);
            sound_state = 5;
          }
          else if(sound_state == 5){
            playSdWav1.play("miles.WAV");
            delay(10);
            sound_state = 6;
          }
          else if(sound_state == 6){
            playSdWav1.play("calories.WAV");
            delay(10);
            sound_state = 7;
          }
          else if(sound_state == 7){
            large_numbers(cals_int);
            sound_state = 8;
          }
          else if(sound_state == 8){
            playSdWav1.play("point.WAV");
            delay(10);
            sound_state = 9;
          }
          else if(sound_state == 9){
            if(cals_dec == "00"){
              cals_dec = "0";
            }
            speech = cals_dec + ".WAV";
            playSdWav1.play(speech);
            delay(10);
            sound_state = 0;
          }
          
          while(playSdWav1.isPlaying() == true){}
          
        }
      }
      if(keyword == "stats"){
        int steps_percent_end = message.indexOf("#", message_hash + 1);
        int cal_percent_end = message.indexOf("#", steps_percent_end + 1);
        String steps_percent = message.substring(message_hash + 1, steps_percent_end);
        String cal_percent = message.substring(steps_percent_end + 1, cal_percent_end);
        String dist_percent = message.substring(cal_percent_end + 1, message.length());
        sound_state = 1;
        speech = "";
        while(sound_state != 0){
          if(sound_state == 1){
            playSdWav1.play("steps.WAV");
            delay(10);
            sound_state = 2;
          }
          else if(sound_state == 2){
            speech = steps_percent + ".WAV";
            playSdWav1.play(speech);
            delay(10);
            sound_state = 3;
          }
          else if(sound_state == 3){
            playSdWav1.play("percent.WAV");
            delay(10);
            sound_state = 4;
          }
          else if(sound_state == 4){
            playSdWav1.play("comp.WAV");
            delay(10);
            sound_state = 5;
          }
          else if(sound_state == 5){
            playSdWav1.play("calories.WAV");
            delay(10);
            sound_state = 6;
          }
          else if(sound_state == 6){
            speech = cal_percent + ".WAV";
            playSdWav1.play(speech);
            delay(10);
            sound_state = 7;
          }
          else if(sound_state == 7){
            playSdWav1.play("percent.WAV");
            delay(10);
            sound_state = 8;
          }
          else if(sound_state == 8){
            playSdWav1.play("comp.WAV");
            delay(10);
            sound_state = 9;
          }
          else if(sound_state == 9){
            playSdWav1.play("distance.WAV");
            delay(10);
            sound_state = 10;
          }
          else if(sound_state == 10){
            speech = dist_percent + ".WAV";
            playSdWav1.play(speech);
            delay(10);
            sound_state = 11;
          }
          else if(sound_state == 11){
            playSdWav1.play("percent.WAV");
            delay(10);
            sound_state = 12;
          }
          else if(sound_state == 12){
            playSdWav1.play("comp.WAV");
            delay(10);
            sound_state = 0;
          }
          while(playSdWav1.isPlaying() == true){}
        }
      }
      message = "";
    }
    if(Serial1.available()>0){
      while(Serial1.available() > 0){
        char incomingByte= Serial1.read();
        message.concat(incomingByte);
        //Serial.println(message);
      }
      //Serial.println(message);
      //Serial.println("Start playing");
      //message = "";
    }
    
  
  // do nothing while playing...
}

void large_numbers(String number_string){
  int number = number_string.toInt();
  int substate = 0;
  int thousands_dig = number/1000;
  int hundreds_dig = (number/100)%10;
  int other_dig = number%100;
  substate = 1;
  String num_speech = "";
  Serial.println(number);
  Serial.println(thousands_dig);
  Serial.println(hundreds_dig);
  Serial.println(other_dig);
  while(substate != 0){
    if(substate == 1){
      if(number >= 1000){
        num_speech = String(thousands_dig) + ".WAV";
        playSdWav1.play(num_speech);
        delay(10);
      }
      substate = 2;
    }
    else if(substate == 2){
      if(number >= 1000){
        playSdWav1.play("1000.WAV");
        delay(10);
      }
      substate = 3;
    }
    else if(substate == 3){
      if(hundreds_dig >= 1){
        num_speech = String(hundreds_dig) + "00.WAV";
        playSdWav1.play(num_speech);
        delay(10);
      }
      substate = 4;
    }        
    else if(substate == 4){
      if(number > 100 && other_dig != 0){
        playSdWav1.play("and.WAV");
        delay(10);
      }
      substate = 5;
    }
    else if(substate == 5){
      if(other_dig != 0){
        num_speech = String(other_dig) + ".WAV";
        playSdWav1.play(num_speech);
      }
      else if(other_dig == 0){
        playSdWav1.play("0.WAV");
      }
      delay(10);
      substate = 0;
    }    
   while(playSdWav1.isPlaying() == true){}
  
  }
}


