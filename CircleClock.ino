/*
 CircleClock V1
 MI&AS
 
 Added significant commenting
 */

#include <TimerOne.h>


//Short and long are in multiples of the interrupt eg, 40ms per interrupt, shortpress is used to avoid debouncing and long is used fo
//heking the input functionality.
#define SHORTPRESS 1
#define LONGPRESS 10


enum states {
  normalOperation,setHour,setMinute,setSecond} 
currentState = normalOperation;


static int led[] = {
  //LED Pins Ph=place holder
  //PH,1,2,3,4,5,6,7,8,9,10,11,12
  A4,9,8,7,6,5,4,3,2,1,0,A5,0

};
static int button[] = { 
  A2, A3, A6, A7};
static int pwmPeriod = 1;
static int pwmSteps = 255;

//volatile variables can be changed inside an interrupt service routine
volatile int milliseconds = 0;
volatile byte seconds = 0;
volatile byte minutes = 43;
volatile byte hours = 1;
volatile byte mode = 0;
volatile byte button1Pressed = 0;
volatile byte button2Pressed = 0;
volatile byte button3Pressed = 0;
volatile byte button4Pressed = 0;


void setup()
{             
  Timer1.initialize(40000); // set timer period in microseconds, will execute 25 times per second
  //Timer1.initialize(500); // set timer period in microseconds
  Timer1.attachInterrupt( timerIsr ); // attach the service routine here
  for (int i=0; i<12; i++){
    pinMode(led[i], OUTPUT);     
  }
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  /* pinMode(A6, INPUT);
   p1inMode(A7, INPUT);
   */

  /*
  pinMode(A0, OUTPUT);
   pinMode(A1, OUTPUT);
   //PB2-5, D10-13
   pinMode(10, OUTPUT);
   pinMode(11, OUTPUT);
   pinMode(12, OUTPUT);
   pinMode(13, OUTPUT);
   
   digitalWrite(A0, LOW);
   digitalWrite(A1, LOW);
   digitalWrite(10, LOW);
   digitalWrite(11, LOW);
   digitalWrite(12, LOW);
   digitalWrite(13, LOW);
   */
  //ADCSRA &= ~bit(ADEN);
}

void loop() {
  int ledBrightness[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0
  };


  while(1){

    //decide what brightness value to set the clock to, based on the current time in Hours, Minutes, Seconds and 40 Millisecond increment

    //if you want to do fancy animations, put them here

    //Default Animation - Hours brighter than minutes, seconds travelling around, with 5 brightness levels at each position


    //start with ledBrightness off, 
    for(int i=0;i<12;i++){
      ledBrightness[i] = 0;
    }


    switch(currentState)
    {
    case normalOperation:
      //decide which leds to turn on
      ledBrightness[seconds%12] = 1 + (seconds%5);  
      //ledBrightness[seconds/5] = 1 + (seconds%5);  
      //ledBrightness[seconds/5]=5-( 1 + (seconds%5));
      ledBrightness[minutes/5] = 38-( 2 + 2* pow(int((minutes%5)),2));
      //ledBrightness[(minutes/5+1)%12] =( 2 + 2* pow(int((minutes%5)),2));
      ledBrightness[hours%12] = 128;
      break;
    case setHour:
      ledBrightness[hours%12] = 128;
      break;
    case setMinute:
      ledBrightness[minutes/5] = 38-( 2 + 2* pow(int((minutes%5)),2));
      break;
    case setSecond:
      ledBrightness[seconds%12] = 1 + (seconds%5);  
    default:
      break;
    }
    display(ledBrightness);

    //make use of buttons here
  }
}

void timerIsr()
{
  //called 25 times per second, updates the time by 40ms each time

  // Count 1 Second
  if (digitalRead(A2) == 0)
  {   
    button1Pressed += 1;
  }
  else{
    if(button1Pressed>LONGPRESS)
    {
      currentState=(enum states)(currentState+1);
      if ((int)(currentState)>3)
      {
        currentState=normalOperation;
      }
      digitalWrite(led[1],HIGH);
      digitalWrite(led[2],HIGH);
      digitalWrite(led[3],HIGH);
      digitalWrite(led[4],HIGH);
    }
    else if (button1Pressed>SHORTPRESS){
      switch(currentState)
      {
      case normalOperation:
        break;
      case setHour:
        hours++;
        if(hours==24){
          hours=0;
        }
        break;
      case setMinute:
        minutes++;
        if(minutes==60){
          minutes=0;
        }
        break;
      case setSecond:
        seconds++;
      default:
        break;
      }

    }
    //Now clear the button press.
    //If nothing is pressed long enough we clear it here, and if something was pressed we need to clear the counter anyway.
    button1Pressed=0;

  }




  if (digitalRead(A3) == 0)
  {
    if (button2Pressed < 3) button2Pressed += 1; //button pulls to gnd 
  }
  else{
    if(button2Pressed != 0) button2Pressed -= 1;
  }
  /*
   if (digitalRead(A6) == 0){
   if (button3Pressed < 3) button3Pressed += 1; //button pulls to gnd 
   }
   else{
   if(button3Pressed != 0) button3Pressed -= 1;
   }
   if (digitalRead(A7) == 0){
   if (button4Pressed < 3) button4Pressed += 1; //button pulls to gnd 
   }
   else{
   if(button4Pressed != 0) button4Pressed -= 1;
   }
   */

  //Updates the time, 40ms at a time, rolling over the larger units as necessary
  milliseconds += 40; //25 updates per seconds
  if (milliseconds == 1000){ 
    milliseconds = 0;
    seconds++;
    if (seconds == 60){
      seconds = 0;
      minutes++;
      if (minutes == 60){
        minutes = 0;
        hours++;
        if (hours == 24){
          hours = 0;
        }
      }
    }
  }
}

void display(int *ledBrightness)
{
  //This code performs the manual PWM of the lights
  //ledBrightness contains the brightness values of each led
  //led[i] specifies which pin each LED is attached to.

  int count = pwmSteps; //set at 255
  int i=0;

  //set all leds to low
  for(int i=0;i<12;i++){
    digitalWrite(led[i],LOW);
  }

  count = pwmSteps;
  //count steps from 255 to 0, for each step a loop runs through all the led numbers and decides when to turn the led on.
  //counting downwards allows for simple 'when-to-turn-on' logic
  while(count >= 0){
    for(i=0;i<12;i++){
      if(ledBrightness[i] > count){
        digitalWrite(led[i],HIGH);
      } 
    }
    count--; 
  }

  //set all to low when leaving display function
  for(i=0;i<12;i++){
    digitalWrite(led[i],LOW);
  }

}



















