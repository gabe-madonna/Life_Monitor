#include "Arduino.h"
#include "steps.h"

#define ARRAY_SIZE 10

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
