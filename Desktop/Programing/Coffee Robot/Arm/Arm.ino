#include <Servo.h> 
#define LIGHT 2
#define btn1 12
#define btn2 8
#define btn3 3
#define cup 9
 
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
String rdString = "";
uint32_t value;

double h =  90; //110
double L1 = 108;
double L2 = 200;
double x,y;

double z=-25;
double zz=h-30;

float theta1,theta2,theta3;

int last_pos;

int rev_angle;

int angle11,angle12,angle13;
int angle21,angle22,angle23;

int t = 0;
const int step1 = 150;

bool RESET = 0;
bool PAUSE = 0;

void setup() {
  pinMode(13,OUTPUT);
  pinMode(btn1,INPUT_PULLUP);
  pinMode(btn2,INPUT_PULLUP);
  pinMode(btn3,INPUT_PULLUP);
  pinMode(9,INPUT_PULLUP);
  pinMode(LIGHT,OUTPUT);
  attachInterrupt(digitalPinToInterrupt(btn3),IntBtn,FALLING);
  
  servo1.attach(10);
  servo2.attach(5);
  servo3.attach(11);
  servo4.attach(6);

  Serial.begin(9600);

  angle11 =179;
  angle12 = 149;
  angle13 = 15;
  
  servo1.write(angle11);
  servo2.write(angle12);
  servo3.write(angle13);
  servo4.write(147);
  
}

void loop() {
  buttonHandle();
  receive_move();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void receive_move(){
  while (Serial.available()>0) {
    char c = Serial.read();
    rdString += c;

    if (c =='\n'){
      value = rdString.toInt();
      rdString = "";
      x = value / 1000000;
      y = value / 1000 % 1000;
      if (x>300)
        x = 300 - x;
        
      theta1 = (PI-atan2(sqrt(sq(x)+sq(y)),(zz))-acos((sq(L1)-sq(L2)+sq(zz)+sq(sqrt(sq(x)+sq(y))))/(2*L1*sqrt(sq(zz)+sq(sqrt(sq(x)+sq(y)))))))*180/PI;
      theta2 = (PI-acos((sq(L1)+sq(L2)-sq(zz)-sq(sqrt(sq(x)+sq(y))))/(2*L1*L2)))*180/PI;
      theta3 = atan2(y,x)*180/PI;
   if(sqrt(sq(x)+sq(y))<=290){
      if(x>200){
            angle21 = -theta1+158;
            angle22 = theta2+28;
            angle23 = -theta3+180;
      } else if(0<x<100) {
            angle21 = -theta1+158;
            angle22 = theta2+28;
            angle23 = -theta3+184;
      } else if(x<0) {
            angle21 = -theta1+158;
            angle22 = theta2+19;
            angle23 = -theta3+184;
      } else {
            angle21 = -theta1+158;
            angle22 = theta2+19;
            angle23 = -theta3+180;
      }
   }
   else    Serial.print('l');
      digitalWrite(2,LOW);
      dichuyen();
      digitalWrite(2,HIGH);
      delay(200);
      if (RESET==1) {
        RESET = 0;
        Serial.print('f');
      } else {
        Serial.print('k');
      }
        
    }
  }
  digitalWrite(2,HIGH);
}

void dichuyen(){
    MOVE(servo3,angle13,angle23);
    MOVE(servo4,147,80);
    MOVE2(servo1,angle11,angle21,servo2,angle12,angle22);
    MOVE(servo4,80,130);
    t=0;
    while(t<step1-1){
      moveservo(servo2,angle22,angle12);
      moveservo(servo1,angle21,angle11);
      delay(3);
      t++;
    } 
//  MOVE2(servo2,angle22,angle12,servo1,angle21,angle11);
    if (digitalRead(cup)==HIGH) RESET = 1; 
    int cur_pos_sv3;
    if (!RESET){
      cur_pos_sv3 = angle13 + 13*last_pos;
      MOVE(servo3,angle23,cur_pos_sv3);
      last_pos++;
      if (last_pos == 3) last_pos = 0;
      
      t=0;
      while(t<step1-1){
        moveservo(servo2,angle12,15);
        moveservo(servo1,angle11,55);
        delay(3);
        t++;
      } 
  //   MOVE2(servo1,angle11,angle21,servo2,angle12,angle22);
      MOVE(servo4,130,80);
      servo4.write(110);
      MOVE(servo4,110,80);
      MOVE2(servo1,55,angle11,servo2,15,angle12);  
      MOVE(servo4,80,147);
      MOVE(servo3,cur_pos_sv3,angle13); 
   } else {
    MOVE(servo3,angle23,angle13);
   }
    
    
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////


void _receive(){
  while (Serial.available()>0) {
    char c = Serial.read();
    rdString += c;

    if (c =='\n'){
      value = rdString.toInt();
      rdString = "";
      Serial.println(value);
      servo3.write(value);
    }
  }
}

void buttonHandle(){
  if(digitalRead(btn1)==LOW){
    while (digitalRead(btn1)==LOW) {};
    Serial.print('f');
    delay(100);
  }
  if(digitalRead(btn2)==LOW){
    while (digitalRead(btn2)==LOW) {};
    Serial.print('p');
    delay(100);
  }
}
////////////////////////////////////////
void IntBtn(){
  if(digitalRead(btn3)==LOW){
    while (digitalRead(btn3)==LOW) {};
    //Serial.print('r');
    RESET = 1;
    delay(100);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////

void MOVE(Servo sv,int a1,int a2){
  t=0;
  while(t<step1-1){
    moveservo(sv,a1,a2);delay(5);
    t++;
  }
   delay(5);  
}


void MOVE2(Servo sv1,int a1,int a2,Servo sv2,int a3,int a4){
  t=0;
  while(t<step1-1){
    moveservo(sv1,a1,a2);
    moveservo(sv2,a3,a4);
    delay(5);
    t++;
  }  
}

float moveservo(Servo z,int angleHead1,int angleBack1){
  if(angleBack1>angleHead1){
    int x1 = angleBack1-angleHead1;
    if(t<step1){
     float angle1=angleHead1+x1/PI*(((PI*t)/step1)-cos((PI*t)/step1)*sin((PI*t)/step1));
     z.write(angle1);
     delay(3);
     return angle1;
   }
  }
  else {
    int x11 = angleHead1-angleBack1;
    if(t<step1){
      float angle2=angleHead1-x11/PI*(((PI*t)/step1)-cos((PI*t)/step1)*sin((PI*t)/step1));
      z.write(angle2);
      delay(3);
      return angle2;
    }
  }
}
