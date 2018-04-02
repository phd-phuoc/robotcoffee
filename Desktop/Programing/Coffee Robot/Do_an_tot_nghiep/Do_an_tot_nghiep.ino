#include <digitalWriteFast.h>
#include <TimerOne.h>
#include <SimpleKalmanFilter.h>
//#include <Wire.h>

#define pinA1 6
#define pinA2 7
#define pinB1 4
#define pinB2 5
#define pinC1 8
#define pinC2 9

#define encA1 2
#define encA2 3
#define encB1 18
#define encB2 19
#define encC1 20
#define encC2 21


SimpleKalmanFilter KalmanFilterA(10, 10, 0.01);
SimpleKalmanFilter KalmanFilterB(10, 10, 0.01);
SimpleKalmanFilter KalmanFilterC(10, 10, 0.01);


volatile int countA = 0;
volatile int countB = 0;
volatile int countC = 0;
long _countA, _countB,_countC;

unsigned long last_millis;

unsigned long _micros;
unsigned long last_micros;
long dmicros;
double dt;
double speedA,filtered_A;
double speedB,filtered_B;
double speedC,filtered_C;

int valueIR1, valueIR2, valueIR3, valueIR4, valueIR5;
bool ss1,ss2,ss3,ss4,ss5;

void setup() {
  Serial.begin(115200);
  Serial3.begin(9600);
//  Wire.begin();
  ADCSRA = ((ADCSRA&(B11111000)) | B00000010);
  pinMode(pinA1,OUTPUT);
  pinMode(pinA2,OUTPUT);
  pinMode(pinB1,OUTPUT);
  pinMode(pinB2,OUTPUT);
  pinMode(pinC1,OUTPUT);
  pinMode(pinC2,OUTPUT);
  pinModeFast(encA1,INPUT);
  pinModeFast(encA2,INPUT);
  pinModeFast(encB1,INPUT);
  pinModeFast(encB2,INPUT);
  pinModeFast(encC1,INPUT);
  pinModeFast(encC2,INPUT);
  attachInterrupt(digitalPinToInterrupt(encA1),ItrA1,CHANGE);
  attachInterrupt(digitalPinToInterrupt(encA2),ItrA2,CHANGE);
  attachInterrupt(digitalPinToInterrupt(encB1),ItrB1,CHANGE);
  attachInterrupt(digitalPinToInterrupt(encB2),ItrB2,CHANGE);
  attachInterrupt(digitalPinToInterrupt(encC1),ItrC1,CHANGE);
  attachInterrupt(digitalPinToInterrupt(encC2),ItrC2,CHANGE);

//  Aforth(200);
//  Bforth(200);
//  Cleft(200);
}

byte c;
int v1 = 150;
int v2 = 150;
void loop() {
  if (Serial3.available()>0){
      c = Serial3.read();
      Serial.println(c);
  }
  if (c==1){
    Aforth(v1);
    Bforth(v2);
    if (v1>v2) Cleft(v1-v2);
    if (v1<v2) Cright(v2-v1);
    if (v1==v2) Cstop();
  }
  if (c==2){
    Aback(v1);
    Bback(v2);
    if (v1>v2) Cright(v1-v2);
    if (v1<v2) Cleft(v2-v1);
    if (v1==v2) Cstop();
  }
  if (c==3){
    Astop();
    Bstop();
    Cstop();
  }
  if (c==16){
    v1 = 200;
    v2 = 100;
  }
  if (c==8){
    v1 = 100;
    v2 = 200;
  }
  if (c==24){
    v1 = 150;
    v2 = 150;
  }
  if (c==48){
    Aforth(150);
    Bback(150);
    Cleft(150);
    delay(300);
    Astop();
    Bstop();
    Cstop();
    c=0;
  }
  if (c==96){
    Aback(150);
    Bforth(150);
    Cright(150);
    delay(300);
    Astop();
    Bstop();
    Cstop();
    c=0;
  }
  //test_encoder();
 get_line_ss();
 test_speed_tconst();
//  test_speed_posconst();
// test_move();
  
//  Serial.print(line1);
//  Serial.print(line2);
//  Serial.print(line3);
//  Serial.print(line4);
//  Serial.println(line5);
 
}

/////////////////////////////////////////////////////
void get_line_ss(){
  valueIR1 = analogRead(A0);
  valueIR2 = analogRead(A1);
  valueIR3 = analogRead(A2);
  valueIR4 = analogRead(A3);
  valueIR5 = analogRead(A4);
  ss1 = ValueIR(valueIR1); 
  ss2 = ValueIR(valueIR2);
  ss3 = ValueIR(valueIR3);
  ss4 = ValueIR(valueIR4);
  ss5 = ValueIR(valueIR5);
//  Serial.print(ss1);
//  Serial.print(ss2);
//  Serial.print(ss3);
//  Serial.print(ss4);
//  Serial.println(ss5);
}
void test_speed_posconst(){
  if (countA > 100 ){
    _countA = countA;
    _countB = countB;

    dmicros = micros() - last_micros;
    last_micros = micros();
    dt = dmicros/60000000.0;
    
    speedA = float (_countA)/22/dt;
    speedB = float (_countB)/22/dt;
    filtered_A = filtered_A*0.92 + speedA*0.08;
    filtered_B = filtered_B*0.92 + speedB*0.08;
    
    Serial.print(_countA);
    Serial.print("  ");
    Serial.println(_countB);
    countA = 0;
    countB = 0;
  }
}

void test_speed_tconst(){
  _micros = micros();
  dmicros = _micros - last_micros;

  if (dmicros > 20000 ) {
    last_micros = _micros;
    _countA = countA;
    _countB = countB;
    _countC = countC;
    countA = 0;
    countB = 0;
    countC = 0;
    dt = dmicros/60000000.0;
    
    speedA = float (_countA)/22/dt;
    speedB = float (_countB)/22/dt;
    speedC = float (_countC)/22/dt;
    
//    filtered_A = filtered_A*0.9 + speedA*0.1;
//    filtered_B = filtered_B*0.9 + speedB*0.1;
    filtered_A = KalmanFilterA.updateEstimate(speedA);
    filtered_B = KalmanFilterB.updateEstimate(speedB);
    filtered_C = KalmanFilterC.updateEstimate(speedC);

//    Serial.print(filtered_C);
//    Serial.print("  ");
//    Serial.print(speedC);
//    Serial.print("  ");
//    Serial.print(filtered_B);
//    Serial.print("  ");
//    Serial.print(speedB);
//    Serial.print("  ");
//    Serial.print(filtered_A);
//    Serial.print("  ");
//    Serial.println(speedA);
  }
}
////////////////////////////////////
void test_encoder() {
  analogWrite(pinA1,250);
  digitalWrite(pinA2,LOW);
  analogWrite(pinB1,250);
  digitalWrite(pinB2,LOW);
  last_millis = millis();
  while (millis()-last_millis<2000) {}
  last_millis = millis();
  Serial.print(countA);
  Serial.print("  ");
  Serial.println(countB);
  
  analogWrite(pinA2,250);
  digitalWrite(pinA1,LOW);
  analogWrite(pinB2,250);
  digitalWrite(pinB1,LOW);
  while (millis()-last_millis<2000) {}
  Serial.print(countA);
  Serial.print("  ");
  Serial.println(countB);
}

void test_move(){
  Aforth(200);
  Bforth(200);
  Cstop();
  delay(1000);
  Aback(200);
  Bback(200);
  Cstop();
  delay(1000);
  Aback(100);
  Bforth(100);
  Cleft(200);
  delay(1000);
  Aforth(100);
  Bback(100);
  Cright(200);
  delay(1000);
}
////////////////////////////////////////////
void Aforth(int PWM){
  analogWrite(pinA1,PWM);
  digitalWrite(pinA2,LOW);
}
void Aback(int PWM){
  analogWrite(pinA2,PWM);
  digitalWrite(pinA1,LOW);
}
void Astop(){
  digitalWrite(pinA2,LOW);
  digitalWrite(pinA1,LOW);
}
void Bforth(int PWM){
  analogWrite(pinB1,PWM);
  digitalWrite(pinB2,LOW);
}
void Bback(int PWM){
  analogWrite(pinB2,PWM);
  digitalWrite(pinB1,LOW);
}
void Bstop(){
  digitalWrite(pinB2,LOW);
  digitalWrite(pinB1,LOW);
}
void Cleft(int PWM){
  analogWrite(pinC1,PWM);
  digitalWrite(pinC2,LOW);
}
void Cright(int PWM){
  analogWrite(pinC2,PWM);
  digitalWrite(pinC1,LOW);
}
void Cstop(){
  digitalWrite(pinC2,LOW);
  digitalWrite(pinC1,LOW);
}
bool ValueIR(int valueIR)
{
  if (valueIR <= 400){
    //Serial.print("1");
    return 1;
  }
  else {
    //Serial.print("0");
    return 0;
  }
}
////////////////////////////////////////////
void ItrA1() {
  if (digitalReadFast(encA1) != digitalReadFast(encA2)){
      countA--;
    }
    else  {
      countA++;
    }
}

void ItrA2() {
  if (digitalReadFast(encA1) != digitalReadFast(encA2)){
      countA++;
    }
    else  {
      countA--;
    }
}

void ItrB1() {
  if (digitalReadFast(encB1) != digitalReadFast(encB2)){
      countB--;
    }
    else  {
      countB++;
    }
}

void ItrB2() {
  if (digitalReadFast(encB1) != digitalReadFast(encB2)){
      countB++;
    }
    else  {
      countB--;
    }
}

void ItrC1() {
  if (digitalReadFast(encC1) != digitalReadFast(encC2)){
      countC--;
    }
    else  {
      countC++;
    }
}

void ItrC2() {
  if (digitalReadFast(encC1) != digitalReadFast(encC2)){
      countC++;
    }
    else  {
      countC--;
    }
}
