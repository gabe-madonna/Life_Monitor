#include <SPI.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <MPU9250.h>
#include <TinyGPS++.h>
#include <Wifi_S08_v2.h>
#include <TimeLib.h>
#include <SerialFlash.h>
#include <math.h>
#define SCREEN U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI

#define ARRAY_SIZE 10
#define GPSSerial Serial3

//const int STEPS_UPDATE_INTERVAL = 100;    // how often to update display (ms)
String current_user = "dimitral";
const String PATH = "/6S08dev/pkarnati/final/sb2.py";     
const String IESC = "iesc-s2.mit.edu";   
const int UPDATE_INTERVAL = 10000;
const int GPS_UPDATE_INTERVAL = 1500;
const int SECONDS_PER_HOUR = 3600;
const int ORDER = 4;
const float STEP_THRESHOLD = 1.2;
const int STEP_INTERVAL = 500;
const int PULSE_PIN = 4;
const int LED_PIN = 13;
const int BUTTON_PIN1 = 2;
const int BUTTON_PIN2 = 6;
const int BUTTON_PIN3 = 5;
const int CALORIE_UPDATE_INTERVAL = 2000;
const int BLINKING_INTERVAL = 600;
boolean gps_working = true;
float workout_calories;
int global_counter = 0;
int count = 0;
int heart_rate = 0;
//int step_counter = 0;
int step_counter = 0;
int last_num_steps = 0;
String heart_data = "";
String post_gps_data = "&lat=360&lon=360";
float accel_data[ARRAY_SIZE] = {0};
String post_data_add_on = "";
float mag_accel;
boolean new_imu = false;
double mit_lat = 42.3597342;
double mit_lon = -71.0944575;
double new_lat;
double new_lon;
double last_lat;
double last_lon;
float workout_distance = 0;
float total_distance = 0;
double speed = 0;
int state = 0;
int workout_state = 0;
String goals = "";
boolean awaiting_response = 0;
int flag = 0;
boolean update_user_data = false;
int substate = 0;
boolean flag_gender = false;
boolean flag_weight = false;
boolean flag_height = false;
String gender = "other";
int age_new = 18;
float height_dec = 5.91666;
float weight = 150;
int goals_steps = 2000;
float goals_weight = 150;
float goals_distance = 8;
int goals_calories = 2000;
int height_feet = 5;
int height_inches = 10;
int k = 0;
int n = 0;
float calories = 0;
String the_time = "";
String keyword = "";
String AMorPM = "";
String hour_of_day = "";
String minute_of_hour = "";
int t;
int last_t;
int bv_1 = 0;
int bv_2 = 0;
int bv_3 = 0;
boolean sending = 0;
String mile_pace;
boolean workout_mode = 1;
String post_data = "";
bool new_goal;
int number;
int health_state = 0;
int n_w = 0;
int n_a = 0;
int n_h = 0;
int digit_1 = weight/100;
int digit_2 = (int)(weight)%100/10;
int digit_3 = (int)(weight)%10;
int digit_4 = 0;
int age_digit_1;
int age_digit_2;
int height_1;
float BMR;
bool retrieving_health_stats = 0;
bool measuring_workout_dist = 0;
int age;
bool gps_data_is_valid = 0;
int gs_digits[5];
int gw_digits[4];
int gd_digits[4];
int gc_digits[4];
bool weight_updated = 0;
bool height_updated = 0;
bool gender_updated = 0;
bool age_updated = 0;
float speeds[15];
bool first_speed = 1;
float gps_lat_avg[3];
float gps_lon_avg[3];
bool first_gps_vals = 1;
float avg_lat;
float avg_lon;
bool first_dist = 1;
bool wifi_was_connected = 1;
bool using_gps_time = 0;
bool data_is_stored_in_sd = 0;
String message = "";
int last_blink_time = 0;
int save_time = 0;

volatile byte Signal;
volatile int rate[10];
volatile unsigned long last_beat_time = 0;
volatile boolean first_time = 1;
volatile boolean second_time = 0;
volatile int IBI;
volatile int BPM;

elapsedMillis update_timer = 0;
elapsedMillis step_interval_timer = 0;
elapsedMillis time_since_update = 0;  // time since last screen update
elapsedMillis calorie_timer = 0;

MPU9250 imu;
ESP8266 wifi = ESP8266(0,true);
TinyGPSPlus gps;
SCREEN oled(U8G2_R2, 20, 15, 16);

class Button{
    int state;
    int flag;
    elapsedMillis t_since_change; //timer since switch changed value
    elapsedMillis t_since_state_2; //timer since entered state 2 (reset to 0 when entering state 0)
    unsigned long debounce_time;
    unsigned long long_press_time;
    int pin;
    bool button_pressed;
  public:
    Button(int p) {    
      state = 0;
      pin = p;
      t_since_change = 0;
      t_since_state_2= 0;
      debounce_time = 10;
      long_press_time = 1000;
      button_pressed = 0;
    }
    void read() {
      bool button_state = digitalRead(pin);  // true if HIGH, false if LOW
      button_pressed = !button_state; // Active-low logic is hard, inverting makes our lives easier.
    }
    int update() {
      read();
      flag = 0;
      if (state==0) { // Unpressed, rest state
        if (button_pressed) {
          state = 1;
          t_since_change = 0;
        }
      } else if (state==1) { //Tentative pressed
        if (!button_pressed) {
          state = 0;
          t_since_change = 0;
        } else if (t_since_change >= debounce_time) {
          state = 2;
          t_since_state_2 = 0;
        }
      } else if (state==2) { // Short press
        if (!button_pressed) {
          state = 4;
          t_since_change = 0;
        } else if (t_since_state_2 >= long_press_time) {
          state = 3;
        }
      } else if (state==3) { //Long press
        if (!button_pressed) {
          state = 4;
          t_since_change = 0;
        }
      } else if (state==4) { //Tentative unpressed
        if (button_pressed && t_since_state_2 < long_press_time) {
          state = 2; // Unpress was temporary, return to short press
          t_since_change = 0;
        } else if (button_pressed && t_since_state_2 >= long_press_time) {
          state = 3; // Unpress was temporary, return to long press
          t_since_change = 0;
        } else if (t_since_change >= debounce_time) { // A full button push is complete
          state = 0;
          if (t_since_state_2 < long_press_time) { // It is a short press
            flag = 1;
          } else {  // It is a long press
            flag = 2;
          }
        }
      }
      return flag;
    }
};
Button button_1(BUTTON_PIN1);
Button button_2(BUTTON_PIN2);
Button button_3(BUTTON_PIN3);

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);
  GPSSerial.begin(9600);
  SPI.setSCK(14);
  Wire.begin();
  oled.begin();
  //oled.setFont(u8g2_font_5x7_mf);
  oled.setFont(u8g2_font_7x13_mf);
  oled.clearBuffer();
  wifi.begin();
  wifi.connectWifi("MIT","");
  //wifi.connectWifi("6s08","iesc6s08");
  pinMode(PULSE_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN1, INPUT_PULLUP);
  pinMode(BUTTON_PIN2, INPUT_PULLUP);
  pinMode(BUTTON_PIN3, INPUT_PULLUP);
  attachInterrupt(PULSE_PIN, ISR, RISING);
  setTime(0, 16, 0, 12, 5, 2017);
  //Alarm.alarmRepeat(0,0,0,reset_function);
  
  byte c = imu.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
  Serial.print("MPU9250 "); Serial.print("I AM "); Serial.print(c, HEX);
  Serial.println("MPU9250 is online...");
  
  // Calibrate gyro and accelerometers, load biases in bias registers
  imu.initMPU9250();
  imu.MPU9250SelfTest(imu.selfTest);
  imu.calibrateMPU9250(imu.gyroBias, imu.accelBias);
  imu.initAK8963(imu.factoryMagCalibration);
  imu.getAres();
  imu.getGres();
  imu.getMres();
}

void loop() {
  if(message.indexOf("@")!= -1){
    Serial.println(message);
    int username_end = message.indexOf("#");
    int gender_end = message.indexOf("#", username_end + 1);
    int height_feet_end = message.indexOf("/");
    int height_inches_end = message.indexOf("#", height_feet_end + 1);
    int weight_end = message.indexOf("#", height_inches_end+1);
    int age_end = message.indexOf("#", weight_end+1);
    int goal_steps_end = message.indexOf("#", age_end+1);
    int goal_weight_end = message.indexOf("#", goal_steps_end+1);
    int goal_dist_end = message.indexOf("#", goal_weight_end+1);
    int goal_cal_end = message.indexOf("#", goal_dist_end+1);
    current_user = message.substring(0,username_end);
    gender = message.substring(username_end + 1,gender_end);
    height_feet = message.substring(gender_end + 1, height_feet_end).toInt();
    height_inches = message.substring(height_feet_end +1, height_inches_end).toInt();
    weight = message.substring(height_inches_end + 1, weight_end).toFloat();
    age_new = message.substring(weight_end + 1, age_end).toInt();
    goals_steps = message.substring(age_end + 1, goal_steps_end).toInt();
    goals_weight = message.substring(goal_steps_end + 1, goal_weight_end).toFloat();
    goals_distance = message.substring(goal_weight_end + 1, goal_dist_end).toFloat();
    goals_calories = message.substring(goal_dist_end + 1, goal_cal_end).toInt();
    message = "";
  }
  if (state==0) {  // Beginning of state=0
    bv_1 = button_1.update(); //get button_1 value
    bv_2 = button_2.update(); //get button_2 value 
    bv_3 = button_3.update(); //get button_3 value
    if (bv_2==2) {
      state=2;
      substate=0;
      k=0;
      //workout_mode = 0;
    }
    else{
      retrieve_health_stats();
      if(retrieving_health_stats == 0){
        workout();
      }
      
    }
  }  // End of state=0

  else if (state==2) {   
    bv_1 = button_1.update(); //get button_1 value
    bv_2 = button_2.update(); //get button_2 value
    bv_3 = button_3.update(); 

    if (substate<=8) {  // Beginning of substates=0-4
        if (k==0) {
          Serial.println("1st menu");
          oled.clearBuffer();
          oled.setCursor(2, 10);
          oled.print("Gender");
          oled.setCursor(2,22);
          oled.print("Height");
          oled.setCursor(2,35);
          oled.print("Weight");
          oled.setCursor(2,48);
          oled.print("Age");
          oled.setCursor(2,61);
          oled.print("Goals");
          oled.setCursor(75,10);
          oled.print("Exit");
          oled.setCursor(75,22);
          oled.print("Users");
          oled.setCursor(75,35);
          oled.print("Stats"); 
          oled.setCursor(75, 48);
          oled.print("Save");
          if(substate <= 4){
            oled.drawFrame(0,12.5*substate,50,13);
          }
          else{
            oled.drawFrame(70, 12.5*(substate-5),50,13);
          }
          
          oled.sendBuffer();
          k=1;
        }
    
        if (bv_1==1) {
          substate=(substate+1)%9;  // in order to scroll through the 1st menu, substates=0-4
          k=0;
        }
        if(bv_2 == 1){
          substate = substate - 1;
          if(substate < 0){
            substate = 8;
          }
          k=0;
        }
        else if (bv_3==1) {
          if (substate==0) {substate=substate+9; }
          else if(substate<5 && substate>0) { substate=substate+11; }
          else if(substate == 5){
            state = 0;
            substate = 0;
            oled.clearBuffer();
            oled.setCursor(5,5);
            oled.print("");
            oled.sendBuffer();
          }
          else if(substate == 6){
            substate = 23;
          }
          else if(substate == 7){
            substate = 27;
          }
          else if(substate == 8){
            save_time = millis();
            substate = 29;
          }
          k=0;
          n_w = 0;
          n_a = 0;
          n_h = 0;
        }
        
    }  // End of substates=0-4
    else if (substate>=9 && substate<=11) { // Beginning of substates=5-7
      if (k==0) {
          Serial.println("2nd menu -> Gender");
          oled.clearBuffer();
          oled.setCursor(2, 10);
          oled.print("Male");
          oled.setCursor(2, 22);
          oled.print("Female");
          oled.setCursor(2, 35);
          oled.print("Other");
          oled.drawFrame(0,12.5*(substate-9),50,13);
          oled.sendBuffer();
          k=1;                                      } // End of k=0

       if (bv_1==1) {
        substate=substate + 1; // in order to scroll through the 2nd menu, substates=5-7
        if(substate > 11){
          substate = 9;
        }
        k=0;
       }
       if(bv_2 == 1){
          substate = substate - 1;
          if(substate < 9){
            substate = 11;
          }
          k=0;
        }
       else if(bv_3==1) {
        if (substate==9) { gender="male"; }
        else if(substate==10) { gender="female"; }
        else if (substate==11) { gender="other"; }
        // Send the updated gender to the server
        post_data_add_on = post_data_add_on + "gender=" + gender + "&";
        gender_updated = 1;
        state=2;
        substate = 0;
        oled.clearBuffer();    // Kind of unecessary code that is used to clear up the screen
        oled.setCursor(0, 8);  // Kind of unecessary code that is used to clear up the screen
        oled.print("");        // Kind of unecessary code that is used to clear up the screen
        oled.sendBuffer();     // Kind of unecessary code that is used to clear up the screen
        workout_mode = 1;
        k = 0;
       }

          
    } // end of substates=5-7

    else if (substate==12) {  // Beginning of substate=8 -> Height Substate
      int height_digits[2] = {height_feet, height_inches};
      if (k==0) {
//        Serial.println("Height");
//        oled.clearBuffer();
//        oled.setCursor(0, 8);
//        oled.print(hght);
//        oled.sendBuffer();
//        k=1;
        Serial.println("Height");
        height_digits[0] = height_feet;
        height_digits[1] = height_inches;
        oled.clearBuffer();
        //oled.setCursor(0, 8);
        //oled.print(weight);
        oled.drawTriangle(9 + 30*n_h,20,17 + 30*n_h,20,13 + 30*n_h,15);
        oled.drawTriangle(9 + 30*n_h,50,17 + 30*n_h,50,13 + 30*n_h,55);
        oled.drawRFrame(5,25,19,20,7);
        oled.drawRFrame(34,25,19,20,7);
        oled.setCursor(10, 40);
        oled.print(String(height_feet));
        oled.setCursor(25, 38);
        oled.print("'");
        oled.setCursor(37, 40);
        oled.print(String(height_inches));
        oled.setCursor(56, 38);
        oled.print('"');
        oled.setCursor(2,10);
        oled.print("Select Height");
        oled.sendBuffer();
        k=1;
      }
      
      if (bv_1==1) { 
        if(n_h == 0){
          height_digits[0] = height_digits[0] - 1;
          if(height_digits[0]<4){
            height_digits[0] = 7;
          }
          height_feet = height_digits[0];
        }
        else{
          height_digits[1] = height_digits[1] - 1;
          if(height_digits[1] < 0){
            height_digits[1] = 11;
          }
          height_inches = height_digits[1];
        }
        k=0; 
      }
      else if (bv_2==1) { 
        //hght=hght+0.01;
        if(n_h == 0){
          height_digits[0] = height_digits[0] + 1;
          if(height_digits[0]>7){
            height_digits[0] = 4;
          }
          height_feet = height_digits[0];
        }
        else{
          height_digits[1] = height_digits[1] + 1;
          if(height_digits[1] > 11){
            height_digits[1] = 0;
          }
          height_inches = height_digits[1];
        }
        k=0; 
        }

      else if (bv_3==1) {
        //flag_height=true;
        //send updated height to the server
        n_h = n_h + 1;
        k = 0;
        if(n_h == 2){
          float height_inches_dec = (float)height_inches/12;
          height_dec = height_feet + height_inches_dec;
          post_data_add_on = post_data_add_on + "height="+String(height_feet) + String(height_inches_dec).substring(1) + "&";
          height_updated = 1;
          state=2;
          substate = 0;
          oled.clearBuffer();    // Kind of unecessary code that is used to clear up the screen
          oled.setCursor(0, 8);  // Kind of unecessary code that is used to clear up the screen
          oled.print("");        // Kind of unecessary code that is used to clear up the screen
          oled.sendBuffer();     // Kind of unecessary code that is used to clear up the screen
          workout_mode = 1;
        }
      }
      
    }  // End of substate=8

    else if (substate==13) {  // Weight substate (substate=9)
      int digits[4] = {digit_1, digit_2, digit_3, digit_4};
      if (k==0) {
        digit_1 = weight/100;
        digit_2 = (int)(weight)%100/10;
        digit_3 = (int)(weight)%10;
        digit_4 = ((int)(10*weight))%10;
        digits[0] = digit_1;
        digits[1] = digit_2;
        digits[2] = digit_3;
        digits[3] = digit_4;
        Serial.println("Weight");
        oled.clearBuffer();
        //oled.setCursor(0, 8);
        //oled.print(weight);
        oled.drawTriangle(9 + 25.5*n_w,20,17 + 25.5*n_w,20,13 + 25.5*n_w,15);
        oled.drawTriangle(9 + 25.5*n_w,50,17 + 25.5*n_w,50,13 + 25.5*n_w,55);
        oled.drawRFrame(5,25,19,20,7);
        oled.drawRFrame(29,25,19,20,7);
        oled.drawRFrame(54,25,19,20,7);
        oled.drawRFrame(81,25,19,20,7);
        oled.setCursor(10, 40);
        oled.print(String(digit_1));
        oled.setCursor(35, 40);
        oled.print(String(digit_2));
        oled.setCursor(60, 40);
        oled.print(String(digit_3));
        oled.setCursor(74, 40);
        oled.print(".");
        oled.setCursor(87, 40);
        oled.print(String(digit_4));
        oled.setCursor(105, 43);
        oled.print("lbs");
        oled.setCursor(2,10);
        oled.print("Select Weight");
        oled.sendBuffer();
        k=1;
      }
      
      if (bv_1==1) { 
        //weight--;
        digits[n_w] = digits[n_w] - 1; 
        if(digits[n_w] < 0){
          digits[n_w] = 9;
        }
        k=0;
        weight = (float)digits[0]*100 + (float)digits[1]*10 + (float)digits[2] + (float)digits[3]/10; 
      }
      
      else if (bv_2==1) { 
        //weight++;
        digits[n_w] = (digits[n_w] + 1)%10; 
        k=0;
        weight = (float)digits[0]*100 + (float)digits[1]*10 + (float)digits[2] + (float)digits[3]/10; 
      }

      else if (bv_3==1) {
        n_w = n_w + 1;
        k = 0;
        //flag_weight=true;
        //send updated weight to the server
        if(n_w == 4){
          post_data_add_on = post_data_add_on + "weight=" + String(weight) + "&";
          weight_updated = 1;
          state=2;
          substate = 0;
          oled.clearBuffer();    // Kind of unecessary code that is used to clear up the screen
          oled.setCursor(0, 8);  // Kind of unecessary code that is used to clear up the screen
          oled.print("");        // Kind of unecessary code that is used to clear up the screen
          oled.sendBuffer();     // Kind of unecessary code that is used to clear up the screen
          workout_mode = 1;
        }
      }
      
    }  // End of substate=9 -> Weight substate

    else if (substate==14) {
      int age_digits[2] = {age_digit_1, age_digit_2};
      if (k==0) {
        Serial.println("Age");
//        oled.clearBuffer();
//        oled.setCursor(0, 8);
//        oled.print(age_new);  // age_new is just the age, but age is a reserved keyword
//        oled.sendBuffer();
//        k=1;
        age_digit_1 = age_new/10;
        age_digit_2 = age_new%10;
        age_digits[0] = age_digit_1;
        age_digits[1] = age_digit_2;
        oled.clearBuffer();
        //oled.setCursor(0, 8);
        //oled.print(weight);
        oled.drawTriangle(9 + 25.5*n_a,20,17 + 25.5*n_a,20,13 + 25.5*n_a,15);
        oled.drawTriangle(9 + 25.5*n_a,50,17 + 25.5*n_a,50,13 + 25.5*n_a,55);
        oled.drawRFrame(5,25,19,20,7);
        oled.drawRFrame(29,25,19,20,7);
        oled.setCursor(10, 40);
        oled.print(String(age_digit_1));
        oled.setCursor(35, 40);
        oled.print(String(age_digit_2));
        oled.setCursor(52, 43);
        oled.print("Years");
        oled.setCursor(2,10);
        oled.print("Select Age");
        oled.sendBuffer();
        k=1;
      }
      
      if (bv_1==1) { 
        //age_new--;
        age_digits[n_a] = age_digits[n_a] - 1;
        if(age_digits[n_a]<0){
          age_digits[n_a] = 9; 
        }
        age_new = 10*age_digits[0] + age_digits[1];
        k=0; 
        }
      else if (bv_2==1) { 
        //age_new++; 
        age_digits[n_a] = (age_digits[n_a] + 1)%10;
        k=0; 
        age_new = 10*age_digits[0] + age_digits[1];
        }

      else if (bv_3==1) {
        //flag_age=true;
        //send updated age to the server
        n_a = n_a + 1;
        k = 0;
        if(n_a == 2){
          post_data_add_on = post_data_add_on + "age=" + String(age_new) + "&";
          age_updated = 1;
          state=2;
          substate = 0;
          oled.clearBuffer();    // Kind of unecessary code that is used to clear up the screen
          oled.setCursor(0, 8);  // Kind of unecessary code that is used to clear up the screen
          oled.print("");        // Kind of unecessary code that is used to clear up the screen
          oled.sendBuffer();     // Kind of unecessary code that is used to clear up the screen
          workout_mode = 1;
        }
        
      }
    }

    else if (substate>=15 && substate<=18) {
      
      if (k==0) {
          Serial.println("4th menu");
          oled.clearBuffer();
          oled.setCursor(2, 10);
          oled.print("Steps Goal");
          oled.setCursor(2,22);
          oled.print("Weight Goal");
          oled.setCursor(2,35);
          oled.print("Distance Goal");
          oled.setCursor(2,48);
          oled.print("Calories Goal");
          oled.drawFrame(0,12.5*(substate-15),100,13);
          oled.sendBuffer();
          k=1;
        }  // End of k=0


      if (bv_1==1) {
          substate = substate + 1;  // in order to scroll through the 4th menu, substates=11-14
          if(substate > 18){
            substate = 15;
          }
          k=0;
        }
        if(bv_2 == 1){
          substate = substate - 1;
          if(substate < 15){
            substate = 18;
          }
          k=0;
        }
        else if (bv_3==1) {
          substate=substate+4;
          k=0;
          new_goal=true;
          n_w = 0;
        }
        
    }  // End of substates=11-14
    else if(substate == 19){
      if (k==0) {
        gs_digits[0] = goals_steps/10000;
        gs_digits[1] = (int)goals_steps%10000/1000;
        gs_digits[2] = (int)(goals_steps)%1000/100;
        gs_digits[3] = (int)(goals_steps)%100/10;
        gs_digits[4] = ((int)(goals_steps))%10;   
        Serial.println("Steps");
        oled.clearBuffer();
        //oled.setCursor(0, 8);
        //oled.print(weight);
        oled.drawTriangle(7 + 22*n_w,20,16 + 22*n_w,20,12 + 22*n_w,15);
        oled.drawTriangle(7 + 22*n_w,50,16 + 22*n_w,50,12 + 22*n_w,55);
        oled.drawRFrame(3,25,19,20,7);
        oled.drawRFrame(25,25,19,20,7);
        oled.drawRFrame(47,25,19,20,7);
        oled.drawRFrame(69,25,19,20,7);
        oled.drawRFrame(91,25,19,20,7);
        oled.setCursor(8, 40);
        oled.print(String(gs_digits[0]));
        oled.setCursor(31, 40);
        oled.print(String(gs_digits[1]));
        oled.setCursor(54, 40);
        oled.print(String(gs_digits[2]));
        oled.setCursor(76, 40);
        oled.print(String(gs_digits[3]));
        oled.setCursor(98, 40);
        oled.print(String(gs_digits[4]));
        oled.setCursor(2, 10);
        oled.print("Daily Steps Goal");
        oled.sendBuffer();
        k=1;
      }
      if (bv_1==1) { 
        //weight--;
        gs_digits[n_w] = gs_digits[n_w] - 1; 
        if(gs_digits[n_w] < 0){
          gs_digits[n_w] = 9;
        }
        k=0;
        goals_steps = gs_digits[0]*10000 + gs_digits[1]*1000 + gs_digits[2]*100 + gs_digits[3]*10 + gs_digits[4]; 
      }
      
      else if (bv_2==1) { 
        //weight++;
        gs_digits[n_w] = (gs_digits[n_w] + 1)%10; 
        k=0;
        goals_steps = gs_digits[0]*10000 + gs_digits[1]*1000 + gs_digits[2]*100 + gs_digits[3]*10 + gs_digits[4]; 
      }

      else if (bv_3==1) {
        n_w = n_w + 1;
        k = 0;
        //flag_weight=true;
        //send updated weight to the server
        if(n_w == 5){
          post_data_add_on = post_data_add_on + "goals_steps=" + String(goals_steps) + "&";
          state=2;
          substate = 0;
          oled.clearBuffer();    // Kind of unecessary code that is used to clear up the screen
          oled.setCursor(0, 8);  // Kind of unecessary code that is used to clear up the screen
          oled.print("");        // Kind of unecessary code that is used to clear up the screen
          oled.sendBuffer();     // Kind of unecessary code that is used to clear up the screen
          //workout_mode = 1;
        }
      }
      }
    else if(substate == 20){
      if (k==0) {
        gw_digits[0] = goals_weight/100;
        gw_digits[1] = (int)(goals_weight)%100/10;
        gw_digits[2] = (int)(goals_weight)%10;
        gw_digits[3] = ((int)(10*goals_weight))%10;   
        Serial.println("Weight");
        oled.clearBuffer();
        //oled.setCursor(0, 8);
        //oled.print(weight);
        oled.drawTriangle(9 + 25.5*n_w,20,17 + 25.5*n_w,20,13 + 25.5*n_w,15);
        oled.drawTriangle(9 + 25.5*n_w,50,17 + 25.5*n_w,50,13 + 25.5*n_w,55);
        oled.drawRFrame(5,25,19,20,7);
        oled.drawRFrame(29,25,19,20,7);
        oled.drawRFrame(54,25,19,20,7);
        oled.drawRFrame(81,25,19,20,7);
        oled.setCursor(10, 40);
        oled.print(String(gw_digits[0]));
        oled.setCursor(35, 40);
        oled.print(String(gw_digits[1]));
        oled.setCursor(60, 40);
        oled.print(String(gw_digits[2]));
        oled.setCursor(74, 40);
        oled.print(".");
        oled.setCursor(87, 40);
        oled.print(String(gw_digits[3]));
        oled.setCursor(105, 43);
        oled.print("lbs");
        oled.setCursor(2,10);
        oled.print("Target Weight");
        oled.sendBuffer();
        k=1;
      }
      
      if (bv_1==1) { 
        //weight--;
        gw_digits[n_w] = gw_digits[n_w] - 1; 
        if(gw_digits[n_w] < 0){
          gw_digits[n_w] = 9;
        }
        k=0;
        goals_weight = (float)gw_digits[0]*100 + (float)gw_digits[1]*10 + (float)gw_digits[2] + (float)gw_digits[3]/10; 
      }
      
      else if (bv_2==1) { 
        //weight++;
        gw_digits[n_w] = (gw_digits[n_w] + 1)%10; 
        k=0;
        goals_weight = (float)gw_digits[0]*100 + (float)gw_digits[1]*10 + (float)gw_digits[2] + (float)gw_digits[3]/10; 
      }

      else if (bv_3==1) {
        n_w = n_w + 1;
        k = 0;
        //flag_weight=true;
        //send updated weight to the server
        if(n_w == 4){
          post_data_add_on = post_data_add_on + "goals_weight=" + String(goals_weight) + "&";
          state=2;
          substate = 0;
          oled.clearBuffer();    // Kind of unecessary code that is used to clear up the screen
          oled.setCursor(0, 8);  // Kind of unecessary code that is used to clear up the screen
          oled.print("");        // Kind of unecessary code that is used to clear up the screen
          oled.sendBuffer();     // Kind of unecessary code that is used to clear up the screen
          //workout_mode = 1;
        }
      }
      }
      else if(substate == 21){
      if (k==0) {
        gd_digits[0] = goals_distance/100;
        gd_digits[1] = (int)(goals_distance)%100/10;
        gd_digits[2] = (int)(goals_distance)%10;
        gd_digits[3] = ((int)(10*goals_distance))%10;   
        Serial.println("Distance");
        oled.clearBuffer();
        //oled.setCursor(0, 8);
        //oled.print(weight);
        oled.drawTriangle(9 + 25.5*n_w,20,17 + 25.5*n_w,20,13 + 25.5*n_w,15);
        oled.drawTriangle(9 + 25.5*n_w,50,17 + 25.5*n_w,50,13 + 25.5*n_w,55);
        oled.drawRFrame(5,25,19,20,7);
        oled.drawRFrame(29,25,19,20,7);
        oled.drawRFrame(54,25,19,20,7);
        oled.drawRFrame(81,25,19,20,7);
        oled.setCursor(10, 40);
        oled.print(String(gd_digits[0]));
        oled.setCursor(35, 40);
        oled.print(String(gd_digits[1]));
        oled.setCursor(60, 40);
        oled.print(String(gd_digits[2]));
        oled.setCursor(74, 40);
        oled.print(".");
        oled.setCursor(87, 40);
        oled.print(String(gd_digits[3]));
        oled.setCursor(105, 43);
        oled.print("mi");
        oled.setCursor(2,10);
        oled.print("Daily Miles Goal");
        oled.sendBuffer();
        k=1;
      }
      
      if (bv_1==1) { 
        //weight--;
        gd_digits[n_w] = gd_digits[n_w] - 1; 
        if(gd_digits[n_w] < 0){
          gd_digits[n_w] = 9;
        }
        k=0;
        goals_distance = (float)gd_digits[0]*100 + (float)gd_digits[1]*10 + (float)gd_digits[2] + (float)gd_digits[3]/10; 
      }
      
      else if (bv_2==1) { 
        //weight++;
        gd_digits[n_w] = (gd_digits[n_w] + 1)%10; 
        k=0;
        goals_distance = (float)gd_digits[0]*100 + (float)gd_digits[1]*10 + (float)gd_digits[2] + (float)gd_digits[3]/10; 
      }

      else if (bv_3==1) {
        n_w = n_w + 1;
        k = 0;
        //flag_weight=true;
        //send updated weight to the server
        if(n_w == 4){
          post_data_add_on = post_data_add_on + "goals_distance=" + String(goals_distance) + "&";
          state=2;
          substate = 0;
          oled.clearBuffer();    // Kind of unecessary code that is used to clear up the screen
          oled.setCursor(0, 8);  // Kind of unecessary code that is used to clear up the screen
          oled.print("");        // Kind of unecessary code that is used to clear up the screen
          oled.sendBuffer();     // Kind of unecessary code that is used to clear up the screen
          //workout_mode = 1;
        }
      }
      }
      else if(substate == 22){
      if (k==0) {
        gc_digits[0] = goals_calories/1000;
        gc_digits[1] = goals_calories%1000/100;
        gc_digits[2] = goals_calories%100/10;
        gc_digits[3] = goals_calories%10;   
        Serial.println("Calories");
        oled.clearBuffer();
        //oled.setCursor(0, 8);
        //oled.print(weight);
        oled.drawTriangle(9 + 23.5*n_w,20,17 + 23.5*n_w,20,13 + 23.5*n_w,15);
        oled.drawTriangle(9 + 23.5*n_w,50,17 + 23.5*n_w,50,13 + 23.5*n_w,55);
        oled.drawRFrame(5,25,19,20,7);
        oled.drawRFrame(28,25,19,20,7);
        oled.drawRFrame(51,25,19,20,7);
        oled.drawRFrame(74,25,19,20,7);
        oled.setCursor(10, 40);
        oled.print(String(gc_digits[0]));
        oled.setCursor(33, 40);
        oled.print(String(gc_digits[1]));
        oled.setCursor(56, 40);
        oled.print(String(gc_digits[2]));
        oled.setCursor(79, 40);
        oled.print(String(gc_digits[3]));
        oled.setCursor(98, 43);
        oled.print("cal");
        oled.setCursor(2,10);
        oled.print("Calories to Burn");
        oled.sendBuffer();
        k=1;
      }
      
      if (bv_1==1) { 
        //weight--;
        gc_digits[n_w] = gc_digits[n_w] - 1; 
        if(gc_digits[n_w] < 0){
          gc_digits[n_w] = 9;
        }
        k=0;
        goals_calories = gc_digits[0]*1000 + gc_digits[1]*100 + gc_digits[2]*10 + gc_digits[3]; 
      }
      
      else if (bv_2==1) { 
        //weight++;
        gc_digits[n_w] = (gc_digits[n_w] + 1)%10; 
        k=0;
        goals_calories = gc_digits[0]*1000 + gc_digits[1]*100 + gc_digits[2]*10 + gc_digits[3]; 
      }

      else if (bv_3==1) {
        n_w = n_w + 1;
        k = 0;
        //flag_weight=true;
        //send updated weight to the server
        if(n_w == 4){
          post_data_add_on = post_data_add_on + "goals_calories=" + String(goals_calories) + "&";
          state=2;
          substate = 0;
          oled.clearBuffer();    // Kind of unecessary code that is used to clear up the screen
          oled.setCursor(0, 8);  // Kind of unecessary code that is used to clear up the screen
          oled.print("");        // Kind of unecessary code that is used to clear up the screen
          oled.sendBuffer();     // Kind of unecessary code that is used to clear up the screen
          //workout_mode = 1;
        }
      }
      }
      else if(substate >= 23 && substate <=26){
        if (k==0) {
        Serial.println("Users");
        oled.clearBuffer();
        oled.setCursor(2, 10);
        oled.print("dimitral");
        oled.setCursor(2, 22);
        oled.print("gdolphin");
        oled.setCursor(2, 35);
        oled.print("pkarnati");
        oled.setCursor(2, 47);
        oled.print("gmadonna");
        oled.drawFrame(0,12.5*(substate-23),60,13);
        oled.sendBuffer();
        k=1;
      }
      if (bv_1==1) {
          substate = substate + 1;  // in order to scroll through the 4th menu, substates=11-14
          if(substate > 26){
            substate = 23;
          }
          k=0;
        }
        if(bv_2 == 1){
          substate = substate - 1;
          if(substate < 23){
            substate = 26;
          }
          k=0;
        }
        else if (bv_3==1) {
          if(substate == 23){
            Serial2.print("user#dimitral@");
          }
          else if(substate == 24){
            Serial2.print("user#gdolphin@");
          }
          else if(substate == 25){
            Serial2.print("user#pkarnati@");
          }
          else if(substate == 26){
            Serial2.print("user#gmadonna@");          
          }
          substate = 0;
          state = 2;
          k = 0;
        }
      }
      else if(substate == 27){
        if(k == 0){
          oled.clearBuffer();
          oled.setCursor(2,10);
          oled.print("User: ");
          oled.print(current_user);
          oled.setCursor(2,22);
          oled.print("Gender: ");
          oled.print(gender);
          oled.setCursor(2, 35);
          oled.print("Weight: ");
          oled.print(weight);
          oled.print(" lbs");
          oled.setCursor(2, 48);
          oled.print("Height: ");
          oled.print(height_feet);
          oled.print("'");
          oled.print(height_inches);
          oled.print('"');
          oled.setCursor(2,61);
          oled.print("Age: ");
          oled.print(age_new);
          oled.setCursor(90, 61);
          oled.print("Next");
          if(millis() - last_blink_time > 2*BLINKING_INTERVAL){
            last_blink_time = millis();
          }
          else if(millis() - last_blink_time > BLINKING_INTERVAL){
            oled.drawFrame(87, 50, 32, 13);
          }
          else if(millis() - last_blink_time < BLINKING_INTERVAL){
            
          }
          oled.sendBuffer();
          k = 0;
        }
        
        if (bv_3==1) {
          k = 0;
          substate = 28;
          oled.clearBuffer();
          oled.setCursor(0,8);
          oled.print("");
          oled.sendBuffer();
        }
        
      }
      else if(substate == 28){
        if(k == 0){
          oled.clearBuffer();
          oled.setCursor(2,10);
          oled.print("Daily Goals");
          oled.setCursor(2,23);
          oled.print("Cals to Burn:");
          oled.print(goals_calories);
          oled.setCursor(2, 35);
          oled.print("Weight: ");
          oled.print(goals_weight);
          oled.print(" lbs");
          oled.setCursor(2, 48);
          oled.print("Distance: ");
          oled.print(goals_distance);
          oled.print(" mi");
          oled.setCursor(2,61);
          oled.print("Steps: ");
          oled.print(goals_steps);
          oled.setCursor(90, 61);
          oled.print("Back");
          if(millis() - last_blink_time > 2*BLINKING_INTERVAL){
            last_blink_time = millis();
          }
          else if(millis() - last_blink_time > BLINKING_INTERVAL){
            oled.drawFrame(87, 50, 32, 13);
          }
          else if(millis() - last_blink_time < BLINKING_INTERVAL){
            
          }
          oled.sendBuffer();
          k = 0;
        }
        if (bv_3==1) {
          k = 0;
          state = 2;
          substate = 0;
          oled.clearBuffer();
          oled.setCursor(0,8);
          oled.print("");
          oled.sendBuffer();
        }
      }
      else if(substate == 29){
        if(millis() - save_time > 1200){
          k = 0;
          state = 2;
          substate = 0;
          oled.clearBuffer();
          oled.setCursor(0,8);
          oled.print("");
          oled.sendBuffer(); 
        }
        //save#gdolphin#(gender)#(height)#(weight)#(age)#(goal steps)#(goal weight)#(goal distance)#(goal calories)
        else{
          String saved_data = "save" + "#" + current_user + "#" + gender + "#" + String(height_feet) + "/" + String(height_inches) + "#" + String(weight) + "#" + String(age_new) + "#" + String(goals_steps) + "#" + String(goals_weight) + "#" + String(goals_distance) + "#" + String(goals_calories);
          oled.clearBuffer();
          oled.setCursor(2,10);
          oled.print("Saved!");
          oled.sendBuffer();
          Serial2.print(saved_data);
        }
      }
    }
    
  digitalWrite(LED_PIN, LOW);
  imu.readAccelData(imu.accelCount);
  imu.ax = (float)imu.accelCount[0]*imu.aRes;
  imu.ay = (float)imu.accelCount[1]*imu.aRes;
  imu.az = (float)imu.accelCount[2]*imu.aRes;
  mag_accel = sqrt(imu.ax*imu.ax + imu.ay*imu.ay + imu.az*imu.az);
  update_array(accel_data, mag_accel);
  float smoothed_data[ARRAY_SIZE] = {0};
  smooth_data(accel_data, smoothed_data, ORDER);
  float peak = find_peak(smoothed_data);
  if(peak > STEP_THRESHOLD && step_interval_timer > STEP_INTERVAL){
    step_counter = step_counter + 1;
    step_interval_timer = 0;
  }
  if(weight_updated == 1 or height_updated == 1 or age_updated == 1 or gender_updated == 1){
    calc_BMR();
    Serial.print("BMR:");
    Serial.println(BMR);
    weight_updated = 0;
    height_updated = 0;
    age_updated = 0;
    gender_updated = 0;
  }
  if(calorie_timer > CALORIE_UPDATE_INTERVAL){
    //float calories_addition = calc_calories_burnt(calorie_timer) + BMR*calorie_timer/86400000;
    float calories_addition = calc_calories_burnt(calorie_timer);
    if(workout_state == 1){
      workout_calories = workout_calories + calories_addition;
    }
    calories = calories + calories_addition;
    calorie_timer = 0;
  }
  if(gps_working){
    while(GPSSerial.available()){
      if(gps.encode(GPSSerial.read())){
        print_data();
        if(gps.location.isValid() and update_timer > GPS_UPDATE_INTERVAL){
         if(wifi.isConnected() == 0 and wifi_was_connected == 1){
            setTime(gps.time.hour(), gps.time.minute(), gps.time.second(), gps.date.day(), gps.date.month(), gps.date.year());
            adjustTime(-4*SECONDS_PER_HOUR);
            using_gps_time = 1;
         }
          store_GPS_data();
        }
      } 
    }
    if(wifi.hasResponse()){
      String time_response = wifi.getResponse();
      Serial.println(time_response);
      int day_beg = time_response.indexOf("#");
      int day_end = time_response.indexOf("month");
      int month_beg = time_response.indexOf("#", day_beg + 1);
      int month_end = time_response.indexOf("year");
      int year_beg = time_response.indexOf("#",month_beg + 1);
      int year_end = time_response.indexOf("hour");
      int hour_beg = time_response.indexOf("#",year_beg + 1);
      int hour_end = time_response.indexOf("minute");
      int minute_beg = time_response.indexOf("#", hour_beg + 1);
      int minute_end = time_response.indexOf("</html>");
      String the_day = time_response.substring(day_beg + 1, day_end);
      String the_month = time_response.substring(month_beg + 1, month_end);
      String the_year = time_response.substring(year_beg + 1, year_end);
      String the_hour = time_response.substring(hour_beg + 1, hour_end);
      String the_minute = time_response.substring(minute_beg + 1, minute_end);
      setTime(the_hour.toInt(), the_minute.toInt(), 0, the_day.toInt(), the_month.toInt(), the_year.toInt());
    }
    if(wifi.isConnected() && !wifi.isBusy() && update_timer > UPDATE_INTERVAL){
      int steps_in_interval = step_counter - last_num_steps;
      last_num_steps = step_counter;
      post_gps_data = "&lat=" + String(avg_lat,6) + "&lon=" + String(avg_lon,6);
      post_data = post_data_add_on + "kerberos=" + current_user + "&heart_rate=" + String(BPM) + post_gps_data + "&steps=" + String(steps_in_interval);
      post_data_add_on = "";
      wifi.sendRequest(POST, IESC, 80, PATH, post_data, false);
      Serial.println("sent");
      update_timer = 0;
      post_gps_data = "&lat=&lon=";
      wifi_was_connected = 1;
    }
    //store#(bpm)#(lat)#(lon)#(time)
//    else if(wifi.isConnected() == 0 && update_timer > UPDATE_INTERVAL && using_gps_time == 1){
//      data_is_stored_in_sd = 1;
//      String time_stamp = String(hour()) + ":" + String(minute()) + ":" + String(second());
//      String stored_vals = "store#" + String(BPM) + "#" + String(avg_lat,6) + "#" + String(avg_lon,6) + "#" + time_stamp; 
//      Serial2.print(stored_vals);
//      wifi_was_connected = 0;
//    }
//    if(wifi.isConnected() == 1 && wifi_was_connected == 0 && data_is_stored_in_sd == 1){
//      
//    }
  }

  if(Serial2.available()>0){
      while(Serial2.available() > 0){
        char incomingByte= Serial2.read();
        message.concat(incomingByte);
      }
    }
}

void print_data() {
  Serial.print("\nTime: ");
  Serial.print(hour(), DEC); Serial.print(':');
  Serial.print(minute(), DEC); Serial.print(':');
  Serial.print(second(), DEC); Serial.print('.');
  Serial.println(gps.time.centisecond());
  Serial.print("Date: ");
  Serial.print(day(), DEC); Serial.print('/');
  Serial.print(month(), DEC); Serial.print('/');
  Serial.println(year(), DEC);
  Serial.print("Fix: "); Serial.println(gps.location.isValid());
  if (gps.location.isValid()) {
    Serial.print("Location: ");
    Serial.print(gps.location.lat(), 6); 
    Serial.print(", ");
    Serial.println(gps.location.lng(), 6);

    Serial.print("Speed (mph): "); Serial.println(gps.speed.mph());
    Serial.print("Course: "); Serial.println(gps.course.deg());
    Serial.print("Altitude (m): "); Serial.println(gps.altitude.meters());
    Serial.print("Satellites: "); Serial.println(gps.satellites.value());
  }
}
void store_GPS_data() {
  //t = millis() - last_t;
  double distance;
  new_lat = gps.location.lat();
  new_lon = gps.location.lng();
  
  avg_lat = 0;
  avg_lon = 0;
  if(first_gps_vals){
    for(int i = 0; i<3; i++){
      gps_lat_avg[i] = new_lat;
      gps_lat_avg[i] = new_lon;
      avg_lat = new_lat;
      avg_lon = new_lon;
    }
    first_gps_vals = 0;
  }
  else{
    for (int i = 0; i<2; i++){
      gps_lat_avg[i] = gps_lat_avg[i+1];
      gps_lon_avg[i] = gps_lat_avg[i+1];
      avg_lat = avg_lat + gps_lat_avg[i];
      avg_lon = avg_lon + gps_lon_avg[i];
    }
    gps_lat_avg[2] = new_lat;
    gps_lon_avg[2] = new_lon;
    avg_lat = (avg_lat + new_lat)/3;
    avg_lon = (avg_lon + new_lon)/3;
  }
  if(first_dist == 1){
    distance = 0;
    first_dist = 0;
  }
  else{
    distance = measure_distance(avg_lat, avg_lon, last_lat, last_lon);
  }
  calc_speed();
  total_distance = total_distance + distance;
  if(measuring_workout_dist){
    workout_distance = workout_distance + distance;
  }
  //last_t = millis();
  last_lat = avg_lat;
  last_lon = avg_lon;
}
void update_array(float *ar, float newData)
{
  int s = ARRAY_SIZE;
  for (int i=0; i<=s-2; i++) {
    ar[i] = ar[i+1];
  }
  ar[s-1] = newData;
}


// m point moving average filter of array ain
void smooth_data(float *ain, float *aout, int m)
{
  int s = ARRAY_SIZE;
  for (int n = 0; n < s ; n++) {
    int kmin = n>(m-1) ? n - m + 1: 0;
    aout[n] = 0;
  
    for (int k = kmin; k <= n; k++) {
      int d = m > n+1 ? n+1 : m;
      aout[n] += ain[k] / d;
    }
  }
}


// find peak in the array
float find_peak(float *ar)
{
  
  int s = ARRAY_SIZE;   // s is length of vector ar
  float p = 0;
  for (int i = 1; i<s-1; i++) {
    if ((ar[i] >= ar[i-1]) && (ar[i] >= ar[i+1])) {
      p = ar[i];
      
    }
  }
  return p;
}

void ISR(){
  Signal = digitalRead(PULSE_PIN);
  digitalWrite(LED_PIN, Signal);
  if(first_time){
    second_time = 1;
    first_time = 0;
    return;  
  }
  IBI = millis() - last_beat_time;
  if(second_time){
    for(int i = 0; i < 10; i++){
      rate[i] = IBI;
    }
    second_time = 0;
  }
  int average_IBI = 0;
  for(int j = 0; j < 9; j++){
    rate[j] = rate[j+1];
    average_IBI = average_IBI + rate[j];
  }
  rate[9] = IBI;
  average_IBI = average_IBI + rate[9];
  average_IBI = average_IBI/10;
  BPM = 60000/average_IBI;
  last_beat_time = millis();
}

// health stats in form:  stats#(steps percent)#(calories percent)#(distance percent)

void retrieve_health_stats(){
  if(health_state == 0){
    if(bv_3 == 2){
      health_state = 1;
      retrieving_health_stats = 1;
    }
  }
  else if(health_state == 1){
      String play_goals_so_far = "stats#"; 
      float percent_steps = 100*(float)step_counter/(float)goals_steps;
      String percent_steps_completed;
      String percent_calories_burnt;
      String percent_distance_completed;
      oled.clearBuffer();
      oled.setCursor(5,10);
      oled.print("Health Stats (%)");
      if(percent_steps >= 100){
        percent_steps_completed = "100";
      }
      else{
        percent_steps_completed = String((int)percent_steps);
      }
      oled.setCursor(5,25);
      oled.print("Steps:");
      oled.setCursor(70, 25);
      oled.print(percent_steps_completed);
      play_goals_so_far = play_goals_so_far + percent_steps_completed + "#";
      float percent_calories = 100*calories/goals_calories;
      if(percent_calories >= 100){
         percent_calories_burnt = "100";
      }
      else{
        percent_calories_burnt = String((int)percent_calories);
      }
      oled.setCursor(5, 37);
      oled.print("Calorie:");
      oled.setCursor(70, 37);
      oled.print(percent_calories_burnt);
      play_goals_so_far = play_goals_so_far + percent_calories_burnt + "#";
      float percent_dist = 100*total_distance/goals_distance;
      if(percent_dist >= 100){
        percent_distance_completed = "100";
      }
      else{
        percent_distance_completed = String((int)percent_dist);
      }
      oled.setCursor(5, 49);
      oled.print("Distance:");
      oled.setCursor(75, 49);
      oled.print(percent_distance_completed);
      play_goals_so_far = play_goals_so_far + percent_distance_completed + "@";
      Serial2.print(play_goals_so_far);
      health_state = 0;
      oled.sendBuffer();
      retrieving_health_stats = 0;
  }      
}

//Response leave in form -->  time#6:45 PM (6:45 PM)  OR  cur#5:55#150#3.02  (mile pace, heart rate, distance(miles))
// OR  end#10.51#642  (miles total, calories)
void workout(){
  if(sending){
    if(keyword == "time"){
      String time_send = keyword + "#" + the_time + "@";
      oled.clearBuffer();
      oled.setCursor(5,10);
      oled.print("Time:");
      oled.setCursor(50,10);
      oled.print(the_time);
      oled.sendBuffer();
      Serial2.print(time_send);
    }
    else if(keyword == "calories"){
      String cal_send = keyword + "#" + calories + "@";
      oled.clearBuffer();
      oled.setCursor(5,10);
      oled.print("Calories:");
      oled.setCursor(75,10);
      oled.print(calories);
      oled.sendBuffer();
      Serial2.print(cal_send);
    }
    else if(keyword == "steps"){
      String send_steps = "steps#" + String(step_counter) + "@";
      oled.clearBuffer();
      oled.setCursor(5,10);
      oled.print("Steps:");
      oled.setCursor(50, 10);
      oled.print(String(step_counter));
      oled.setCursor(5,25);
      oled.print("Distance: ");
      oled.print(String(total_distance));
      oled.sendBuffer();
      Serial2.print(send_steps);
    }
    else if(keyword == "heart"){
      String send_heart = "hrate#" + String(BPM) + "@";
      oled.clearBuffer();
      oled.setCursor(5,10);
      oled.print("Heart Rate:");
      oled.setCursor(85, 10);
      oled.print(String(BPM));
      oled.sendBuffer();
      Serial2.print(send_heart);
    }
    else if(keyword == "pace"){
      String send_pace = "pace#" + mile_pace + "@";
      oled.clearBuffer();
      oled.setCursor(5,10);
      oled.print("Mile pace:");
      oled.setCursor(80, 10);
      oled.print(mile_pace);
      oled.sendBuffer();
      Serial2.print(send_pace);
    }
    else if(keyword == "dist"){
      String send_dist = "dist#" + String(workout_distance) + "@";
      oled.clearBuffer();
      oled.setCursor(5,10);
      oled.print("Distance:");
      oled.setCursor(75, 10);
      oled.print(workout_distance);
      oled.sendBuffer();
      Serial2.print(send_dist);
    }
    else if(keyword == "end"){
      String end_workout_send = keyword + "#" + String(workout_distance) + "#" + String(workout_calories) + "@";
      oled.clearBuffer();
      oled.setCursor(1,10);
      oled.print("End of workout!");
      oled.setCursor(1,25);
      oled.print("Total Dist:");
      oled.setCursor(82,25);
      oled.print(workout_distance);
      oled.setCursor(1,40);
      oled.print("Calories:");
      oled.setCursor(75,40);
      oled.print(workout_calories);
      oled.sendBuffer();
      Serial2.print(end_workout_send);
      workout_distance = 0;
      measuring_workout_dist = 0;
      workout_calories = 0;
    }
    sending = 0;
  }
  else{
      if(workout_state == 0){
      if(bv_1 == 1){
        get_time();
        keyword = "time";
        sending = 1;
        
      }
      if(bv_2 == 1){
        keyword = "calories";
        sending = 1;
      }
      if(bv_3 == 1){
        keyword = "steps";
        sending = 1;
      }
      else if(bv_1 == 2){
        workout_state = 1;
        Serial.println("Workout mode...");
        oled.clearBuffer();
        oled.setCursor(5,10);
        oled.print("Workout Mode!");
        oled.sendBuffer();
        workout_distance = 0;
        workout_calories = 0;
      }
    }
    else if(workout_state == 1){
      if(bv_1 == 1){
        calc_mile_pace();
        keyword = "pace";
        sending = 1;
      }
      if(bv_2 == 1){
        keyword = "heart";
        sending = 1;
      }
      if(bv_3 == 1){
        keyword = "dist";
        sending = 1;
        measuring_workout_dist = 1;
      }
      else if(bv_1 == 2){
        workout_state = 0;
        keyword = "end";
        sending = 1;
        Serial.println("Ended workout...");
        oled.clearBuffer();
        oled.setCursor(5,10);
        oled.print("Ended workout");
        oled.sendBuffer();
      }
    }    
  }
}
void get_time(){
  the_time = "";
  AMorPM = "";
  if(hour() > 12){
    hour_of_day = String(hour()-12);
    the_time = the_time + hour_of_day;
    AMorPM = "PM";
  }
  else if(hour() == 0){
    hour_of_day = "12";
    the_time = the_time + hour_of_day;
    AMorPM = "AM";
  }
  else{
    hour_of_day = String(hour());
    the_time = the_time + hour_of_day;
    AMorPM = "AM";
  }
  //Serial.println(hour());
  //Serial.println(minute());
  minute_of_hour = String(minute());
  if(minute_of_hour.toInt()<10){
    minute_of_hour = "0" + minute_of_hour;
  }
  the_time = the_time + ":" + minute_of_hour + " " + AMorPM;
  awaiting_response = 1;
  Serial.println(the_time);
  //get and say through speaker the time of day for convenience
}
void print_height() {
  oled.clearBuffer();
  oled.setCursor(0, 8);
  Serial.print(height_feet + "'" + height_inches + '"');
  oled.print(height_feet + "'" + height_inches + '"');
  oled.setCursor(20, 8);
  oled.sendBuffer();
}
void print_weight() {
  oled.clearBuffer();
  oled.setCursor(0, 8);
  Serial.print(String(weight));
  Serial.println(" kg");
  oled.print(String(weight));
  oled.setCursor(20, 8);
  oled.print(" kg");
  oled.sendBuffer();
}
void calc_mile_pace(){
  if(speed < 1.2){
    mile_pace = "Unavailable";
  }
  else{
      //double min_per_mile = 1/speed;
    double min_per_mile = 60/speed;
    int min_only = (int)min_per_mile;
    int seconds_only = (int)(60*(min_per_mile - min_only));
    if(seconds_only < 10){
      mile_pace = String(min_only) + ":" + "0" + String(seconds_only);
    }
    else{
      mile_pace = String(min_only) + ":" + String(seconds_only);
    }
  }
  
}
double measure_distance(double lat, double lon, double old_lat, double old_lon){
  const double rEarth = 6371000.0; // in meters
  double x = pow( sin( ((lat - old_lat)*M_PI/180.0) / 2.0), 2.0 );
  double y = cos(old_lat*M_PI/180.0) * cos(lat*M_PI/180.0);
  double z = pow( sin( ((lon - old_lon)*M_PI/180.0) / 2.0), 2.0 );
  double a = x + y * z;
  double c = 2.0 * atan2(sqrt(a), sqrt(1.0-a));
  double d = rEarth * c;
  //Serial.printlnf("%12.9f, %12.9f, %12.9f, %12.9f, %12.9f, %12.9f", x, y, z, a, c, d);
  return d*.000621371; // in meters
  //double dist = gps.distanceBetween(old_lat, old_lon, lat, lon)*.000621371; //convert from meters to miles
  //return dist;  
}
void calc_speed(){
  //speed = 60*distance/double(t);  // miles/minute
  float temp_speed = gps.speed.mph();   // miles/hour
  float ave_speed = 0;
  if(first_speed){
    for(int i = 0; i < 15; i++){
      speeds[i] = temp_speed;
    }
    first_speed = 0;
    speed = temp_speed;
  }
  else{
    for(int i = 0; i < 14; i++){
      speeds[i] = speeds[i+1];
      ave_speed = ave_speed + speeds[i];
    }
    speeds[14] = temp_speed;
    ave_speed = ave_speed + speed;
    speed = ave_speed/15;   
  }
}
void reset_function(){
  step_counter = 0;
  calories = 0;
  total_distance = 0;
}
void calc_BMR(){
  float weight_in_kg = weight*0.453592;  //convert to kg
  float height = height_dec*30.48;
  if(gender == "male"){
    BMR = 10*weight_in_kg + 6.25*height - 5*age_new + 5;
  }
  else if(gender == "female"){
    BMR = 10*weight_in_kg + 6.25*height - 5*age_new -161;
  }
  else if(gender == "other"){
    BMR = 10*weight_in_kg + 6.25*height - 5*age_new - 78;
  }
}
float calc_calories_burnt(int t){
  //t is in milliseconds
  float weight_in_kg = weight*.453592;
  float burned_calories;
  if(gender == "male"){
    burned_calories = 0.000016667*t*(0.6309*BPM + 0.1988*weight_in_kg + 0.2017*age_new - 55.0969)/4.184;
  }
  else if(gender == "female"){
    burned_calories = 0.000016667*t*(0.4472*BPM - 0.1263*weight_in_kg + 0.074*age_new - 20.4022)/4.184;
  }
  else{
    burned_calories = 0.000016667*t*(.53905*BPM - .03625*weight_in_kg + .13785*age_new - 37.74955)/4.184;
  }
  return burned_calories;
}

