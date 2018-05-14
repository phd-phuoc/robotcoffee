#include "Servo.h"
#include <Wire.h>

#define PIN_SERVO 10
#define horn 8

#define cup_t 7
#define cup_e 6

#define t1 3
#define e1 2 
#define t2 4
#define e2 5
#define t3 12
#define e3 13

bool x[4] = {0,0,0,0};
unsigned long c[4];
unsigned long d[4] = {200,200,200,200};
unsigned long ld[4] = {200,200,200,200};

int short_threshold = 10;
int medium_threshold = 18;
int long_threshold = 25;

volatile bool dir;
volatile bool turn;
volatile int pos ;

Servo servo; 

bool stop_signal = 0;
bool cup_signal = 0;
byte idx = 0;


int mid_thres = 20;
int side_thres = 35;

unsigned long _millis;

void setup() {
  Wire.begin();
  Serial.begin(57600);
  pinMode(horn,OUTPUT);
  pinMode(cup_t,OUTPUT);
  pinMode(cup_e,INPUT);
  pinMode(t1,OUTPUT);
  pinMode(e1,INPUT);
  pinMode(t2,OUTPUT);
  pinMode(e2,INPUT);
  pinMode(t3,OUTPUT);
  pinMode(e3,INPUT);
  servo.attach(PIN_SERVO);
}

void loop(){
   detect_cup();
   if(turn){
    if ( millis() - _millis > 30){
      _millis = millis();  
      servo.write(pos);
      if (!dir) pos+=2; 
      else pos-=2;
      if (pos>150) dir = 1;
      else if(pos<30) dir = 0;  
    }
  }
   read_us(1, t1, e1, side_thres);
   read_us(2, t2, e2, mid_thres);
   read_us(3, t3, e3, side_thres);
   if((x[1] || x[2] || x[3]) == 1){
    if (!stop_signal){
      stop_signal = 1;
      if (cup_signal)
        digitalWrite(horn,HIGH);
      send_command(1); //co vc
    }
   }
   else if (stop_signal){
      stop_signal = 0;
      digitalWrite(horn,LOW);
      send_command(2); // ko co vc
   }
//   Serial.print(x[0]); Serial.print(" ");
//   Serial.print(x[1]); Serial.print(" ");
//   Serial.print(x[2]); Serial.print(" ");
//   Serial.print(x[3]); Serial.println(" ");
}


void detect_cup(){
   if (pos >130 || pos < 50) 
      read_us(0, cup_t, cup_e, short_threshold);
   else if(pos > 100 || pos < 80) 
      read_us(0, cup_t, cup_e, medium_threshold);
   else 
      read_us(0, cup_t, cup_e, long_threshold);
      
   if (x[0] == 0){
      turn = 1;
      if (cup_signal==1) {
        cup_signal=0;
        send_command(4);//ko co vc
      }
   } else {
      if(cup_signal==0) {
        cup_signal=1;
        send_command(8);//co vc
      }
      turn = 0;
   }
   
}
void send_command(int cm){
    Wire.beginTransmission(8); // transmit to device #8
    Wire.write(cm);              
    Wire.endTransmission();    
    Serial.println(cm);
}

void read_us(byte idx, int trigger, int echo, int threshold){
    digitalWrite(trigger,HIGH);
    delayMicroseconds(5);
    digitalWrite(trigger,LOW);
    
    c[idx]=pulseIn(echo,HIGH);
    d[idx]=c[idx]/2*344/10000;
    
    if (d[idx]>threshold && ld[idx]>threshold ) x[idx] = 0;
    if (d[idx]<threshold && ld[idx]<threshold ) x[idx] = 1; 
    ld[idx] = d[idx];
    delay(1); 
}
 

