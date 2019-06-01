/*
 * HWfancontrol.c:
 *      Simple PWM Fan Control using HW PWM
 *  Note that WiringPi uses it own pin numbers. Hardware PWM is available on pin 1.
 *  Pin 1 in WiringPi is BCM GPIO18 or physical pin 12. Other pins will use sowftware PWM
 */

#include <wiringPi.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <time.h>





//sets the proper parameters for PWM on the GPIO
// BE AWARE THAT WIRINGPI HAS ITS OWN PIN NUMBERS !!
void initGPIO (int pin, int range,int clock) {
  pinMode (pin,PWM_OUTPUT);
  pwmSetMode(PWM_MODE_MS);
  pwmSetRange(range);
  pwmSetClock(clock);

}

//simply returns date and time for output logging
void getTime(char *ret) {
  char buff[20];
  struct tm *sTm;

  time_t now = time (0);
  sTm = localtime (&now);

  strftime (buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", sTm);

  for (int i=0;i<20;i++) {
    ret[i]=buff[i];
  }

}

//Returns the system temperature in Celsius degrees
float readTemp(void) {
  float systemp, millideg;
  FILE *thermal;
  int n;

  thermal = fopen("/sys/class/thermal/thermal_zone0/temp","r");
  n = fscanf(thermal,"%f",&millideg);
  fclose(thermal);
  systemp = millideg / 1000;

  return systemp;
}

int main (int argc, char argv[])
{

  //do not change
  float PWM_CLOCK_DEVIDER = 2;
  float PWM_RANGE = 384;
  float PWM_RATIO = PWM_RANGE/100;
  float PWM_MIN = 10; //minimum % of fan speed (fans usually don't start under this values)

  //setting default values
  int fanPin = 1 ;//WiringPi has its own pin numbers. here we use pin 1 (GPIO 18 in BCM numbering ; a.k.a physical pin 12)
  int checkInterval = 5 * 1000; //in ms
  double targetTemp = 50.0 ;//target temperature in Celsius degrees
  float pTemp = 10; // divide 100 by pTemp to find the temperature diff that will generate 100% fan speed immediately
  float iTemp = 0.75; //let's call this inertia, defines the weight of the last minutes' temperature in the fan speed

  //variables  
  float actualTemp;
  double diff,iDiff,pDiff;
  float sum=0;
  float fanSpeed = 100;
  int realSpeed;

  char logTime[20]={0};

  getTime(logTime);
  printf ("%s - starting...\n",logTime);


  if (wiringPiSetup () == -1)
    exit (1) ;

  
  //initializing the GPIO with proper values for our need
  initGPIO(fanPin,PWM_RANGE,PWM_CLOCK_DEVIDER);

  
  while (1 == 1) {

    actualTemp=readTemp();
    diff=actualTemp-targetTemp;
    sum=sum+diff;
    pDiff=diff*pTemp;
    iDiff=sum*iTemp;
    fanSpeed=pDiff+iDiff;
    if (fanSpeed>100)
      fanSpeed=100;
    if (fanSpeed<PWM_MIN)
      fanSpeed=0;
    if(sum>135)
      sum=135;
    if (sum<-100)
      sum=-100;

    realSpeed=floor(fanSpeed*PWM_RATIO);

    getTime(logTime);
    printf("%s - temp %f, diff = %f, sum = %f, pDiff = %f, iDiff = %f, speed %f\n",logTime,actualTemp,diff,sum,pDiff,iDiff,floor(fanSpeed));
    pwmWrite (1,realSpeed);

    
    delay (checkInterval);


  }


  

  return 0 ;
}