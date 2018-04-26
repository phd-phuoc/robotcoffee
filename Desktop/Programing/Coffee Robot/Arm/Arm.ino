#include <Servo.h> 
//#include <MsTimer2.h>
 
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
String rdString = "";
uint32_t value;

double h = 110 ;
double L1 = 108;
//double L2 = 190;
double L2 = 175;
double x,y;
double z=-20;
float theta1,theta2,theta3;

int rev_angle;

int angle11,angle12,angle13;
int angle21,angle22,angle23;

int t = 0;
const int step1=150;

void setup() {
  pinMode(13,OUTPUT);
  pinMode(12,INPUT_PULLUP);
  pinMode(5,OUTPUT);
  pinMode(11,OUTPUT);
  pinMode(2,OUTPUT);
  servo1.attach(10);
  servo2.attach(5);
  servo3.attach(11);
  servo4.attach(6);

  Serial.begin(9600);

  angle11 = 67;
  angle12 = 105;
  angle13 = 15;
  
  servo1.write(angle11);
  servo2.write(angle12);
  servo3.write(angle13);
  servo4.write(147);
}

void loop() {
  _send();
  //move_arm();
  receive_move();
  //receive();
}

void move_arm(double x,double y,double z){
  double yy = sqrt(sq(x)+sq(y));
//  theta1 = (PI-atan2(yy,(h-z))-acos((sq(L1)-sq(L2)+sq(h-z)+sq(yy))/(2*L1*sqrt(sq(h-z)+sq(yy)))))*180/PI;
//  theta2 = (PI-acos((sq(L1)+sq(L2)-sq(h-z)-sq(yy))/(2*L1*L2)))*180/PI;
//  theta3 = atan2(y,x);

  
}

void receive_move(){
  while (Serial.available()>0) {
    char c = Serial.read();
    rdString += c;

    if (c =='\n'){
      value = rdString.toInt();
      rdString = "";
      x = value / 1000000;
      y = value / 1000 % 1000;
//      z = value % 1000;
      if (x>300)
        x = 300 - x;
      
      Serial.print(x);
      Serial.print("\t");
      Serial.print(y);
      Serial.print("\t");
      Serial.println(z);
      
      double yy = sqrt(sq(x)+sq(y));
      
//      theta1 = (PI-atan2(yy,(h-z))-acos((sq(L1)-sq(L2)+sq(h-z)+sq(yy))/(2*L1*sqrt(sq(h-z)+sq(yy)))))*180/PI;
//      theta2 = (PI-acos((sq(L1)+sq(L2)-sq(h-z)-sq(yy))/(2*L1*L2)))*180/PI;
//      theta3 = atan2(y,x)*180/PI;
//      angle21 = -theta1+140;
//      angle22 = theta2+105;
//      angle23 = -theta3+180;

//      theta1 = (PI-atan2(yy,(h-z))+acos((sq(L1)-sq(L2)+sq(h-z)+sq(yy))/(2*L1*sqrt(sq(h-z)+sq(yy)))))*180/PI;
//      theta2 = (PI-acos((sq(L1)+sq(L2)-sq(h-z)-sq(yy))/(2*L1*L2)))*180/PI;
//      theta3 = atan2(y,x)*180/PI;
//      angle21 = -theta1+140;
//      angle22 = -theta2+105;
//      angle23 = -theta3+180;

//        theta1=(asin((pow(x,2)+pow(y,2)+pow(z,2)+pow(L1,2)-pow(L2,2))/(2*L1*sqrt(pow(x,2)+pow(y,2)+pow(z,2))))-acos(abs(z)/sqrt((pow(x,2)+pow(y,2)+pow(z,2)))))*180/pi;
//        anpha=atan2((abs(z)-L1*sin(theta1))/(sqrt(pow(x,2)+pow(y,2))-L1*cos(theta1)))*180/pi;
//        theta2= theta1-anpha;
//        theta3=atan2(y,x)*180/pi;
//        
//        angle21=-theta1+120;
//        angle23=-theta3+180;
//        if(angle21<0){
//        angle22=-theta2+105;}
//        else angle22=theta2+105;
//    if(sqrt(sq(x)+sq(y))>266){
//        theta1=asin((sq(x)+sq(y)+sq(z)+sq(L1)-sq(L2))/(2*L1*sqrt(sq(x)+sq(y)+sq(z))))-acos(abs(z)/sqrt((sq(x)+sq(y)+sq(z))));
//        float anpha=atan2((abs(z)-L1*sin(abs(theta1))),(sqrt(sq(x)+sq(y))-L1*cos(abs(theta1))));
//        theta2= (-theta1-anpha)*180/PI;
//        theta3=atan2(y,x)*180/PI;
//        
//        angle21=-theta1*180/PI+65;
//        angle23=-theta3+183;
//        angle22=theta2+95;
//       }
     if(sqrt(sq(x)+sq(y))>266){
        theta1=asin((sq(x)+sq(y)+sq(z)+sq(L1)-sq(L2))/(2*L1*sqrt(sq(x)+sq(y)+sq(z))))-acos(abs(z)/sqrt((sq(x)+sq(y)+sq(z))));
        float anpha=atan2((abs(z)-L1*sin(abs(theta1))),(sqrt(sq(x)+sq(y))-L1*cos(abs(theta1))));
        theta2= (-theta1-anpha)*180/PI;
        theta3=atan2(y,x)*180/PI;
        
        angle21=theta1*180/PI+65;
        angle23=-theta3+183;
        angle22=theta2+95;
       }
    else
       theta1=-(asin((sq(x)+sq(y)+sq(z)+sq(L1)-sq(L2))/(2*L1*sqrt(sq(x)+sq(y)+sq(z))))-acos(abs(z)/sqrt((sq(x)+sq(y)+sq(z)))));
        float anpha=atan2((abs(z)-L1*sin(abs(-theta1))),(sqrt(sq(x)+sq(y))-L1*cos(abs(-theta1))));
        theta2= (theta1-anpha)*180/PI;
        theta3=atan2(y,x)*180/PI;
        
        angle21=theta1*180/PI+65;
        angle23=-theta3+183;
        angle22=-theta2+105;

      Serial.print(angle21);
      Serial.print("\t");
      Serial.print(angle22);
      Serial.print("\t");
      Serial.println(angle23);
      
      digitalWrite(2,LOW);
      
      MOVE(servo3,angle13,angle23);
      MOVE(servo4,147,100);
      MOVE2(servo1,angle11,angle21,servo2,angle12,angle22);
      MOVE(servo4,100,147);
      MOVE2(servo2,angle22,angle12,servo1,angle21,angle11);
      MOVE(servo3,angle23,angle13);
      MOVE(servo4,147,100);  
      MOVE(servo4,100,147); 
    }
  }
  digitalWrite(2,HIGH);
}

void MOVE(Servo sv,int a1,int a2){
  t=0;
  while(t<step1-1){
    moveservo(sv,a1,a2);delay(5);
    t++;
  }  
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
void _send(){
  if(digitalRead(12)==LOW){
    while (digitalRead(12)==LOW){};
    Serial.print('f');
    delay(100);}
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
