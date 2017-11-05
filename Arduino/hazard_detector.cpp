#include "Arduino.h"
#include "hazard_detector.h"

int ORDER = 4;
bool bool_temp;


void smooth_data_overall(float *ain, float *aout, int count, int order) {  // for the hazard detector             // New code -> This whole function is new!!!
  for (int i=0; i<count; i++) {
    //aout[i]=0; // I believe that this is redundant
    if (i<order/2) {
      for (int j=0; j<(i+order/2); j++) {
        aout[i]=aout[i]+ain[j];
      }
      aout[i]=aout[i]/(order/2+i+1);
    }
    else if (i>=(count-order/2)) {
      for (int j=(i-order/2); j<count; j++) {
        aout[i]=aout[i]+ain[j];
      }
      aout[i]=aout[i]/(order/2+count-i);
    }

    else {
      for (int j=i-order/2; j<=i+order/2; j++) {
        aout[i]=aout[i]+ain[j];
      }
      aout[i]=aout[i]/(order+1);
    }
  }
}  // End of function void smooth_data_overall()

bool hazard_detector (int count,float acceleration_data[], float gyro_data[]) {
      bool hazard=false;
      count=count-1;
      float smoothed_data_accel[count];
      float smoothed_data_gyro[count];
      for (int j=0; j<count; j++) {
        smoothed_data_accel[j]=0;
        smoothed_data_gyro[j]=0;
      }
      smooth_data_overall(acceleration_data, smoothed_data_accel, count, ORDER);
      smooth_data_overall(gyro_data, smoothed_data_gyro, count, ORDER);
  
      for (int i=0; i<count; i++) {
        Serial.println(smoothed_data_accel[i]);
        Serial.println(smoothed_data_gyro[i]);
        Serial.println(acceleration_data[i]);
      } 
  
      Serial.println("Count");
      Serial.println(count); 
      for (int i=0; i<=count; i++) { // Beginning of getting hit by a car detector
      if (smoothed_data_accel[i]>=1.9) {
        bool_temp=true;
        for (int j=(i+1); j<=i+200; j++) {
          if (j<=count) {
            if (smoothed_data_accel[j]<1.9) {
              bool_temp=false;
            }
          }
        }

        if (bool_temp) {
          hazard=true;
          bool_temp=false;
        }
      } // End of getting hit by a car detector


     if (smoothed_data_gyro[i]>=150.0) {        // Start of falling down detector
      bool_temp=true;
      for (int j=(i+1); j<=i+100; j++) {
        if (j<=count) {
          if (smoothed_data_gyro[j]<150.0) {
            bool_temp=false;
          }
        }
      }
      if (bool_temp) {
        hazard=true;
        bool_temp=false;
      }
     } // End of falling down detector
     
    } // End of crash detector

    if (hazard) {
      Serial.println("HAZARDDDDD!");
      //hazard=false;
    }
    return hazard;
}

