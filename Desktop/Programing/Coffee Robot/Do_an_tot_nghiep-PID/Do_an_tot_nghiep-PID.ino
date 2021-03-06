//#include <digitalWriteFast.h>
#include <Wire.h>
#include <SimpleKalmanFilter.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"


#define pinA1 6
#define pinA2 7
#define pinB1 4
#define pinB2 5
#define pinC1 8
#define pinC2 10

#define encA1 26
#define encA2 27
#define encB1 22
#define encB2 23
#define encC1 24
#define encC2 25

#define Red 12
#define Green 2
#define Blue 3

#define maxspeed 2300

const int Kp = 650;
const int Ki =36;
const int Kd = 4000;
int Iterm = 0;


SimpleKalmanFilter KalmanFilterA(10, 10, 0.01);
SimpleKalmanFilter KalmanFilterB(10, 10, 0.01);
SimpleKalmanFilter KalmanFilterC(10, 10, 0.01);

RF24 radio(9,52);
const uint64_t pipes[2] = { 0xDEDEDEDEE7LL, 0xDEDEDEDEE9LL };
boolean stringComplete = false;  // whether the string is complete
static int dataBufferIndex = 0;
boolean stringOverflow = false;
char charOverflow = 0;
char SendPayload[31] = "";
char RecvPayload[31] = "";
char serialBuffer[31] = "";

volatile int countA = 0;
volatile int countB = 0;
volatile int countC = 0;
long _countA, _countB,_countC;

volatile long totalcountA,totalcountB;
long _totalcountA,_totalcountB;

unsigned long _millis,__millis;

unsigned long _micros;
unsigned long last_micros;
long dmicros;
double dt;
double speedA,filtered_A;
double speedB,filtered_B;
double speedC,filtered_C;

uint32_t valueIR1, valueIR2, valueIR3, valueIR4, valueIR5, valueIR6, valueIR7;

uint32_t thres[7] = {500,500,500,500,500,500,500};
bool ss1=1,ss2=1,ss3=1,ss4=1,ss5=1,ss6=1,ss7=1;
String ss_value = "";

int ool_count = 0;
int lt_count,rt_count;
int error,lasterror;
bool turn_flag,turnl_flag,turnr_flag;
int v1 = 1600;
int v15 = 1840;
int v2 = 2400;


int itst[30] = {0}; // intersection 1:S 2:R 3:SR -1:b
int cur_itst = 0;
int max_itst = 0;
int analyze_flag = 0;
bool line_end = 0;
bool return_flag = 0;
bool stop_flag = 0;

float gain = 0.02;

String SEND = "";
String WAY = "S";
String ORDER = "";
String ORDER1 = "";
String ORDER2 = "";
String last_ORDER = "";
String rev_ORDER = "";
int ord_dis = 0;
int cur_ord = 1;
int cur_rev = 0;

int stt = 0;

int LED_CODE = 0;//0:off 1:red 2:blue 3:green
unsigned long led_millis;
byte pwm_r,pwm_b,pwm_g;
bool dir_r,dir_b,dir_g;

bool OBS,CUP;

void setup() {
  Wire.begin(8);
  Wire.onReceive(I2Cevent); 
  Serial.begin(57600);
  REG_ADC_MR = 0x10380200;                       
  //ADC -> ADC_CHER = 0xff;     
  radio_setup();
  pin_setup();
  analogWriteResolution(12);
  analogReadResolution(10);
  calib_ss();
  convertString("Robot is ready");
  send_message();
  reset_dis();
}

void loop() {
  //test_encoder();
  //test_speed_tconst();
  //cal_distance();
  //tune_PID();
  
  nRF_receive();
  LED_CODE = 0;
  stt = 0;
  
  if (ORDER != ""){
    while((!CUP || OBS) && !turn_flag){
      Astop();
      Bstop();
      Cstop();
      LED_CODE = 1;
      led_control();
    }
    deliver();
    stt = 2;
    LED_CODE = 2;
  }
  if (return_flag ==1){
    while( OBS && !turn_flag && (cur_rev != rev_ORDER.length()-1)){
      Astop();
      Bstop();
      Cstop();
      LED_CODE = 1;
      led_control();
    }
    returnbase();
    stt = 1;
    LED_CODE = 2;
  }
  if (analyze_flag) {
    stt = 3;
    analyze_room();
    LED_CODE = 3;
  }
  send_check(stt);
  led_control();
  
  
  //Serial.println("xx");
}

///////////////////////////////////////////////////////////////////////////////////
void returnbase(){
  get_line_ss();
  if (ss_value == "11011"){
    error = 0;
    turn_flag = 0;
    ool_count = 0;
    if (stop_flag)
      finish_return();
  }
  //////////////////////////////////////////
  else if(ss_value== "11001"){ 
    error = 1;
    turn_flag = 0;
    ool_count = 0;
    if (stop_flag)
      finish_return();
  }
  else if(ss_value == "10011"){ 
    error = -1;
    turn_flag = 0;
    ool_count = 0;
    if (stop_flag)
      finish_return();
  }
  ////////////////////////////////////////
  else if(ss_value == "11101" ){
    error = 2;
    turn_flag = 0;
    ool_count = 0;
    if (stop_flag)
      finish_return();
  }
  else if(ss_value == "10111" ){ 
    error = -2;
    turn_flag = 0;
    ool_count = 0;
    if (stop_flag)
      finish_return();
  }
  ///////////////////////////////////////////
  else if(ss_value == "11100" && !turn_flag){ 
    error = 3;
    ool_count = 0;
  }
  else if(ss_value == "00111" && !turn_flag){ 
    error = -3;
    ool_count = 0;
  }
  ///////////////////////////////////////////
  else if(ss_value == "11110" && !turn_flag){
    error = 4;
    ool_count = 0;
  }
  else if(ss_value == "01111" && !turn_flag){
    error = -4;
    ool_count = 0;
  }
  ////////////////////////////////////////////////
  else if(ss_value == "11111" && !turn_flag){
    ool_count++;
    if (ool_count > 220){
      line_end = 1;
      turn_right();
      turn_flag = 1;
      ool_count = 0;
      if (cur_rev == rev_ORDER.length()-1) {
        stop_flag = 1;
      }
    }
  }
  else if(ss_value == "00000" ||
          ss_value == "00001" ||
          ss_value == "10000" ){
    go_forth();
    _millis = millis();
    while (millis() - _millis < 400 ) {
      get_line_ss();
      if (!ss1) turnr_flag = 1;
      if (!ss7) turnl_flag = 1;
    }
    if (turnl_flag && turnr_flag){
      turnl_flag = 0;
      turnr_flag = 0;
      get_line_ss();
      if (ss_value != "11111"){
        if (rev_ORDER[cur_rev] == 'L') {
          turn_left();
          delay(600);
        }
        if (rev_ORDER[cur_rev] == 'R') {
          turn_right();
          delay(600);
        }
        if (rev_ORDER[cur_rev] == 'S') go_forth();
        cur_rev++;
        reset_dis();
        turn_flag = 1;
      } else {
        if (rev_ORDER[cur_rev] == 'L') turn_left();
        if (rev_ORDER[cur_rev] == 'R') turn_right();
        cur_rev++;
        delay(600);
        reset_dis();
        turn_flag = 1;
      }
    }
  }

  if (!turn_flag) cPID();
  
  if ((!ss1 && ss7 && !turn_flag) || turnr_flag){
    lt_count++;
    if (lt_count>0){
      lt_count = 0;
      if (!turnr_flag) {
        go_forth();
        delay(350);
      } else turnr_flag = 0;
      get_line_ss();
      if (ss_value != "11111"){
        if (rev_ORDER[cur_rev] == 'L'){ 
          turn_left();
          delay(600);
        }
        if (rev_ORDER[cur_rev] == 'S') go_forth();
        cur_rev++;
        reset_dis();
        turn_flag=1;
      } else {
        if (rev_ORDER[cur_rev] == 'L') {
          turn_left();
          delay(600);
        }
        
        if (rev_ORDER[cur_rev] == 'S') go_forth();
        cur_rev++;
        reset_dis();
        turn_flag=1;
      }
    } 
  }
  else if ((!ss7 && ss1 && !turn_flag) || turnl_flag){
    rt_count++;
    if (rt_count>0){
      rt_count = 0;
      if (!turnl_flag){
        go_forth();
        delay(350);
      } else turnl_flag = 0;
      get_line_ss();
      if (ss_value != "11111"){
        if (rev_ORDER[cur_rev] == 'R') {
          turn_right();
          delay(600);
        }
        if (rev_ORDER[cur_rev] == 'S') go_forth();
        cur_rev++;
        reset_dis();
      } else {  
        if (rev_ORDER[cur_rev] == 'R') turn_right();
        cur_rev++;
        delay(600);
        reset_dis();
        turn_flag=1;
      }
    }
  }
  // report vi tri luc ve, kho'
  //if (millis() - _millis > 500 ) {
    //_millis = millis();
    //convertString(ORDER.substring(0,cur_ord)+String(count_dis()),"feedingback");
    //send_message();
  //}
  
}

void finish_return(){
  Astop();
  Bstop();
  Cstop();
  stop_flag = 0;
  return_flag = 0;
  convertString("*");
  send_message();
  turn_flag = 1;
}

/////////////////////////////////////////////////////////////////////////////////
void deliver(){
  get_line_ss();
  if (ss_value == "11011"){
    error = 0;
    turn_flag = 0;
    ool_count = 0;
  }
  //////////////////////////////////////////
  else if(ss_value== "11001"){ 
    error = 1;
    turn_flag = 0;
    ool_count = 0;
  }
  else if(ss_value == "10011"){ 
    error = -1;
    turn_flag = 0;
    ool_count = 0;
  }
  //////////////////////////////////////////
  else if(ss_value == "11101" ){
    error = 2;
    turn_flag = 0;
    ool_count = 0;
  }
  else if(ss_value == "10111" ){ 
    error = -2;
    turn_flag = 0;
    ool_count = 0;
  }
  ///////////////////////////////////////////
  else if(ss_value == "11100" && !turn_flag){ 
    error = 3;
  }
  else if(ss_value == "00111" && !turn_flag){ 
    error = -3;
  }
  ///////////////////////////////////////////
  else if(ss_value == "11110" && !turn_flag){
    error = 4;
  }
  else if(ss_value == "01111" && !turn_flag){
    error = -4;
  }
  ////////////////////////////////////////////////
  else if(ss_value == "11111" && !turn_flag){
    ool_count++;
    if (ool_count > 400){
      line_end = 1;
      turn_right();
      turn_flag = 1;
      cur_itst--;
      ool_count = 0;
      
      if (cur_itst <0) {
        Astop();
        Bstop();
        Cstop();
        analyze_flag = 0;
      }
      
      if (cur_ord == ORDER.length()) {
        ORDER1 = last_ORDER;
        ORDER = "";
        Astop();
        Bstop();
        Cstop();
        convertString("%");
        send_message();
        return_flag = 1;
        int ret = 1;
        while(CUP){
          if (!nRF_other_order()) ret = 0;
          if (ORDER!="") {
            return_flag = 0;
            convertString(ORDER);
            send_message();
          }
        }
        if (return_flag) {
          convertString("b");
          send_message();
        }
        if (ret) {
          turn_right();
          turn_flag = 1;
        }
        delay(600);        
      }
    }
  }
  else if(ss_value == "00000" ||
          ss_value == "00001" ||
          ss_value == "10000" ){
    go_forth();
    _millis = millis();
    while (millis() - _millis < 400 ) {
      get_line_ss();
      if (!ss1) turnr_flag = 1;
      if (!ss7) turnl_flag = 1;
    }
    if (turnl_flag && turnr_flag){
      turnl_flag = 0;
      turnr_flag = 0;
      get_line_ss();
      if (ss_value != "11111"){
        if (ORDER[cur_ord] == 'L') {
          turn_left();
          delay(400);
        }
        if (ORDER[cur_ord] == 'R'){ 
          turn_right();
          delay(400);
        }
        if (ORDER[cur_ord] == 'S') go_forth();
        cur_ord++;
        reset_dis();
        turn_flag = 1;
      } else {
        if (ORDER[cur_ord] == 'L') turn_left();
        if (ORDER[cur_ord] == 'R') turn_right();
        cur_ord++;
        delay(400);
        reset_dis();
        turn_flag = 1;
      }
    }
  }

  if (!turn_flag) cPID();
  
  if ((!ss1 && ss7 && !turn_flag) || turnr_flag){
    lt_count++;
    if (lt_count>0){
      lt_count = 0;
      if (!turnr_flag) {
        go_forth();
        delay(400);
      } else turnr_flag = 0;
      get_line_ss();
      if (ss_value != "11111"){
        if (ORDER[cur_ord] == 'L'){ 
          turn_left();
          delay(400);
        }
        if (ORDER[cur_ord] == 'S') go_forth();
        cur_ord++;
        reset_dis();
        turn_flag=1;
      } else {
        if (ORDER[cur_ord] == 'L'){ 
          turn_left();
          delay(400);
        }
        if (ORDER[cur_ord] == 'S') go_forth();
        cur_ord++;
        reset_dis();
        turn_flag=1;
      }
    }
  }
  else if ((!ss7 && ss1 && !turn_flag) || turnl_flag){
    rt_count++;
    if (rt_count>0){
      rt_count = 0;
      if (!turnl_flag){
        go_forth();
        delay(400);
      } else turnl_flag = 0;
      get_line_ss();
      if (ss_value != "11111"){
        if (ORDER[cur_ord] == 'R'){ 
          turn_right();
          delay(400);
        }
        if (ORDER[cur_ord] == 'S') go_forth();
        cur_ord++;
        reset_dis();
      } else {  
        if (ORDER[cur_ord] == 'R') turn_right();
        cur_ord++;
        delay(400);
        reset_dis();
        turn_flag=1;
      }
    }
  }

  if (cur_ord == ORDER.length() && !turn_flag ) 
    if (count_dis() > ord_dis){
      ORDER1 = last_ORDER;
      ORDER = "";
      Astop();
      Bstop();
      Cstop();
      convertString("%");
      send_message();
      return_flag = 1;
      bool ret = 1;
      while(CUP){
        if (!nRF_other_order()) ret = 0;
        if (ORDER!="") {
          return_flag = 0;
          convertString(ORDER);
          send_message();
        }
      }
      if (return_flag) {
        convertString("b");
        send_message();
      }
      if (ret) {
        turn_right();
        turn_flag = 1;
      }
      delay(600);     
      
    }

  if (millis() - _millis > 500 && !return_flag) {
    _millis = millis();
    convertString(ORDER.substring(0,cur_ord)+String(count_dis()));
    send_message();
  }
  
}

//////////////////////////////////////////////////////////////////////////////////////
void analyze_room(){
  get_line_ss();
  if (ss_value == "11011"){
    error = 0;
    turn_flag = 0;
    ool_count = 0;
    if (analyze_flag==2) complete_analyze();
  }
  //////////////////////////////////////////
  else if(ss_value== "11001"){ 
    error = 1;
    turn_flag = 0;
    ool_count = 0;
    if (analyze_flag==2) complete_analyze();
  }
  else if(ss_value == "10011"){ 
    error = -1;
    turn_flag = 0;
    ool_count = 0;
    if (analyze_flag==2) complete_analyze();
  }
  ////////////////////////////////////////
  else if(ss_value == "11101" ){
    error = 2;
    turn_flag = 0;
    ool_count = 0;
    if (analyze_flag==2) complete_analyze();
  }
  else if(ss_value == "10111" ){ 
    error = -2;
    turn_flag = 0;
    ool_count = 0;
    if (analyze_flag==2) complete_analyze();
  }
  ///////////////////////////////////////////
  else if(ss_value == "11100" && !turn_flag){ 
    error = 3;
    ool_count = 0;
  }
  else if(ss_value == "00111" && !turn_flag){ 
    error = -3;
    ool_count = 0;
  }
  ///////////////////////////////////////////
  else if(ss_value == "11110" && !turn_flag){
    error = 4;
    ool_count = 0;
  }
  else if(ss_value == "01111" && !turn_flag){
    error = -4;
    ool_count = 0;
  }
  ////////////////////////////////////////////////
  else if(ss_value == "11111" && !turn_flag){
    ool_count++;
    if (ool_count > 500){
      SEND = "dc"+WAY + get_dis();
      convertString(SEND);
      send_message();
      //WAY.remove(WAY.length()-1,1);
      line_end = 1;
      turn_right();
      turn_flag = 1;
      cur_itst--;
      ool_count = 0;
      if (cur_itst <0) {
        convertString("@");
        send_message();
        //complete_analyze();
        turn_right();
        turn_flag = 1;
        analyze_flag = 2;
      }
    }
  }
  else if(ss_value == "00000" ||
          ss_value == "00001" ||
          ss_value == "10000" ){
    go_forth();
    _millis = millis();
    while (millis() - _millis < 250 ) {
      get_line_ss();
      if (!ss1) turnr_flag = 1;
      if (!ss7) turnl_flag = 1;
    }
    if (turnl_flag && turnr_flag){
      turnl_flag = 0;
      turnr_flag = 0;
      get_line_ss();
      if (ss_value != "11111"){
        xulynga4();
        turn_left();
        delay(600);
        reset_dis();
        turn_flag = 1;
      } else {
        xulynga3T();
        turn_left();
        delay(600);
        reset_dis();
        turn_flag = 1;
      }
    }
  }

  if (!turn_flag) cPID();
  
  if ((!ss1 && ss7 && !turn_flag) || turnr_flag){
    lt_count++;
    if (lt_count>0){
      lt_count = 0;
      if (!turnr_flag) {
        go_forth();
        delay(250);
        //reset_dis();
      } else turnr_flag = 0;
      get_line_ss();
      if (ss_value != "11111"){
        xulynga3R();
        turn_left();
        delay(600);
        reset_dis();
        turn_flag=1;
      } else {
        xulynga2L();
        turn_left();
        delay(600);
        reset_dis();
        turn_flag=1;
      }
    }
  }
  else if ((!ss7 && ss1 && !turn_flag) || turnl_flag){
    rt_count++;
    if (rt_count>0){
      rt_count = 0;
      if (!turnl_flag){
        go_forth();
        delay(250);
      } else turnl_flag = 0;
      get_line_ss();
      if (ss_value != "11111"){
        xulynga3L();
      } else {
        xulynga2R();
        turn_right();
        delay(600);
        reset_dis();
        turn_flag=1;
      }
    }
  }
  
}

void complete_analyze(){
  Astop();
  Bstop();
  Cstop();
  analyze_flag = 0;
  turn_flag = 1;
}
/////////////////////////////////////////////////////////////////////////////////////
void xulynga4(){
  if (itst[cur_itst] == 0){
    itst[cur_itst] = 3;
    turn_flag = 1;
    if (!line_end) {
          SEND = "4 " + WAY + get_dis();
          convertString(SEND);
          send_message();
        } else {
          line_end = 0;
          reset_dis();
          //SEND = "4L11";
          //convertString(SEND);
        }
    WAY += "L";
    cur_itst++;
  }
  else if (itst[cur_itst] == 3){
    itst[cur_itst] = 1;
    turn_flag = 1;
    if (!line_end) {
          SEND = "4 " + WAY + get_dis();
          convertString(SEND);
          send_message();
        } else {
          line_end = 0;
          reset_dis();
          //SEND = "4S11";
          //convertString(SEND);
          }
    WAY.remove(WAY.length()-1,1);
    WAY += "S";
    cur_itst++;
  }
  else if (itst[cur_itst] == 1){
    itst[cur_itst] = -1;
    turn_flag = 1;
    if (!line_end) {
          SEND = "4 " + WAY + get_dis();
          convertString(SEND);
          send_message();
        } else {
          line_end = 0;
          reset_dis();
          //SEND = "4R11";
          //convertString(SEND);
          }
    WAY.remove(WAY.length()-1,1);
    WAY += "R";
    cur_itst++;
  }
  else if (itst[cur_itst] == -1){
    itst[cur_itst] = 0;
    cur_itst--;
    turn_flag = 1;
    WAY.remove(WAY.length()-1,1);
    reset_dis();
    //SEND = "4b11";
    //convertString(SEND);
  }
}

void xulynga3T(){
    if (itst[cur_itst] == 0){
      itst[cur_itst] = 12; 
      turn_flag = 1;
      if (!line_end) {
          SEND = "3t" + WAY + get_dis();
          convertString(SEND);
          send_message();
        } else{
          line_end = 0;
          reset_dis();
          //SEND = "3L11";
          //convertString(SEND);
          }
      WAY += "L";
      cur_itst++;
    }
    else if (itst[cur_itst] == -21){
      itst[cur_itst] = 0;
      cur_itst--;
      turn_flag = 1;
      WAY.remove(WAY.length()-1,1);
      reset_dis();
      //SEND = "3b11";
      //convertString(SEND);
    }
    else if (itst[cur_itst] == 31){
      itst[cur_itst] = -31;
      turn_flag = 1;
      if (!line_end) {
          SEND = "3t" + WAY + get_dis();
          convertString(SEND);
          send_message();
        } else {
          line_end = 0;
          reset_dis();
          //SEND = "3S11";
          //convertString(SEND);
          }
      WAY.remove(WAY.length()-1,1);
      WAY += "S";
      cur_itst++;
    }
}

void xulynga3L(){
    if (itst[cur_itst] == 0){
      itst[cur_itst] = 22; 
      //turn_flag = 1;
      if (!line_end) {
          SEND = "3l" + WAY + get_dis();
          convertString(SEND);
          send_message();
        } else {
          line_end = 0;
          reset_dis();
          //SEND = "3S11";
          //convertString(SEND);
          }
      WAY += "S";
      cur_itst++;
    }
    else if (itst[cur_itst] == 12){
      itst[cur_itst] = -11;
      //turn_flag = 1;
      if (!line_end) {
          SEND = "3l" + WAY + get_dis();
          convertString(SEND);
          send_message();
        } else {
          line_end = 0;
          reset_dis();
          //SEND = "3R11";
          //convertString(SEND);
          };
      WAY.remove(WAY.length()-1,1);
      WAY += "R";
      cur_itst++;
    }
    else if (itst[cur_itst] == -31){
      itst[cur_itst] = 0;
      cur_itst--;
      //turn_flag = 1;
      WAY.remove(WAY.length()-1,1);
      reset_dis();
      //SEND = "3b11";
      //convertString(SEND);
    }
    
}

void xulynga3R(){
    if (itst[cur_itst] == 0){
      itst[cur_itst] = 31; 
      turn_flag = 1;
      if (!line_end) {
          SEND = "3r" + WAY + get_dis();  
          convertString(SEND);
          send_message();
        } else {
          line_end = 0;
          reset_dis();
          //SEND = "3L11";
          //convertString(SEND);
        }
      WAY += "L";
      cur_itst++;
    }
    else if (itst[cur_itst] == -11){
      itst[cur_itst] = 0;
      cur_itst--;
      turn_flag = 1;
      WAY.remove(WAY.length()-1,1);
      reset_dis();
      //SEND = "3b11";
      //convertString(SEND);
    }
    else if (itst[cur_itst] == 22){
      itst[cur_itst] = -21;
      turn_flag = 1; 
      if (!line_end) {
          SEND = "3r" + WAY + get_dis();
          convertString(SEND);
          send_message();
        } else { 
          line_end = 0;
          reset_dis();
          //SEND = "3R11";
          //convertString(SEND);
        }
      WAY.remove(WAY.length()-1,1);
      WAY += "R";
      cur_itst++;
    }
}

void xulynga2L(){
    if (itst[cur_itst] == 0){
      itst[cur_itst] = -41; 
      turn_flag = 1;
      if (!line_end) {
          SEND = "2l" + WAY + get_dis();
          convertString(SEND);
          send_message();
        } else {
          line_end = 0;
          reset_dis();
          //SEND = "2L11";
          //convertString(SEND);
        }
      WAY += "L";
      cur_itst++;
    }
    else if (itst[cur_itst] == -51){
      itst[cur_itst] = 0;
      cur_itst--;
      turn_flag = 1;
      WAY.remove(WAY.length()-1,1);
      reset_dis();
      //SEND = "2b11";
      //convertString(SEND);
    }
    
}

void xulynga2R(){
    if (itst[cur_itst] == 0){
      itst[cur_itst] = -51; 
      turn_flag = 1;
      if (!line_end) {
          SEND = "2r" + WAY + get_dis();
          convertString(SEND);
          send_message();
        } else {
          line_end = 0;
          reset_dis();
          //SEND = "2R11";
          //convertString(SEND);
        }
      WAY += "R";
      cur_itst++;
    }
    else if (itst[cur_itst] == -41){
      itst[cur_itst] = 0;
      cur_itst--;
      turn_flag = 1;
      WAY.remove(WAY.length()-1,1);
      reset_dis();
      //SEND = "2b11";
      //convertString(SEND);
    }
}
///////////////////////////////////////////////////////////////////
void cPID(){
  Iterm += Ki * error;
  Iterm=constrain(Iterm,0,maxspeed);
  int motorSpeed = Kp * error + Iterm + Kd * (error - lasterror);
  lasterror = error;
   
  int rightMotorSpeed = maxspeed + motorSpeed;
  int leftMotorSpeed  = maxspeed - motorSpeed;
  
  if (rightMotorSpeed > maxspeed ) rightMotorSpeed = maxspeed; 
  if (leftMotorSpeed > maxspeed ) leftMotorSpeed = maxspeed;
  if (rightMotorSpeed < 0)rightMotorSpeed = 0;    
  if (leftMotorSpeed < 0)leftMotorSpeed = 0;
    
  Aforth(rightMotorSpeed);
  Bforth(leftMotorSpeed);
  Cstop();
}

///////////////////////////////////////////////////////////////////////////////
void get_line_ss(){
  valueIR1 = valueIR1*gain + (1-gain)*analogRead(A0);
  valueIR2 = valueIR2*gain + (1-gain)*analogRead(A1);
  valueIR3 = valueIR3*gain + (1-gain)*analogRead(A2);
  valueIR4 = valueIR4*gain + (1-gain)*analogRead(A3);
  valueIR5 = valueIR5*gain + (1-gain)*analogRead(A4);
  valueIR6 = valueIR6*gain + (1-gain)*analogRead(A5);
  valueIR7 = valueIR7*gain + (1-gain)*analogRead(A6);
  ss1 = ValueIR(valueIR1,thres[0]); 
  ss2 = ValueIR(valueIR2,thres[1]);
  ss3 = ValueIR(valueIR3,thres[2]);
  ss4 = ValueIR(valueIR4,thres[3]);
  ss5 = ValueIR(valueIR5,thres[4]);
  ss6 = ValueIR(valueIR6,thres[5]);
  ss7 = ValueIR(valueIR7,thres[6]);

  ss_value="";
  //ss_value += ss1;
  ss_value += ss2;
  ss_value += ss3;
  ss_value += ss4;
  ss_value += ss5;
  ss_value += ss6;
  //ss_value += ss7;
}
//////////////////////////////////////////////////////////////////////////////////
void test_speed_tconst(){
  Aforth(255);
  Bforth(255);
  Cleft(255);
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
    
    speedA = float (_countA)/44/dt;
    speedB = float (_countB)/44/dt;
    speedC = float (_countC)/44/dt;
    
    filtered_A = KalmanFilterA.updateEstimate(speedA);
    filtered_B = KalmanFilterB.updateEstimate(speedB);
    filtered_C = KalmanFilterC.updateEstimate(speedC);

    Serial.print(filtered_C);
    Serial.print("  ");
//    Serial.print(speedC);
//    Serial.print("  ");
    Serial.print(filtered_B);
//    Serial.print("  ");
//    Serial.print(speedB);
    Serial.print("  ");
    Serial.println(filtered_A);
//    Serial.print("  ");
//    Serial.println(speedA);
  }
}

String get_dis(){
  _totalcountA=totalcountA;
  _totalcountB=totalcountB;
  long totalcount = (_totalcountA+_totalcountB)/2;
  int dis = double(totalcount)/660/2*18.22 + 15;
  //int dis = double(_totalcountB)/660/2*18.22 + 15;
  totalcountA = 0;
  totalcountB = 0;
  return String(dis);
}

int count_dis(){
  _totalcountA=totalcountA;
  _totalcountB=totalcountB;
  long totalcount = (_totalcountA+_totalcountB)/2;
  int dis = double(totalcount)/660/2*18.22 + 7;
//  int dis = double(_totalcountB)/660/2*18.22 + 15;
  return dis;
}

void reset_dis(){
  totalcountA = 0;
  totalcountB = 0;
}

void calib_ss(){
  uint32_t maxthres[7] = {analogRead(A0),analogRead(A1),analogRead(A2),analogRead(A3),analogRead(A4),analogRead(A5),analogRead(A6)};
  uint32_t minthres[7] = {analogRead(A0),analogRead(A1),analogRead(A2),analogRead(A3),analogRead(A4),analogRead(A5),analogRead(A6)};
//  while((ADC->ADC_ISR & 0xff)==0);
//  uint32_t maxthres[7] = {ADC->ADC_CDR[7],ADC->ADC_CDR[6],ADC->ADC_CDR[5],ADC->ADC_CDR[4],ADC->ADC_CDR[3],ADC->ADC_CDR[2],ADC->ADC_CDR[1]};
//  while((ADC->ADC_ISR & 0xff)==0);
//  uint32_t minthres[7] = {ADC->ADC_CDR[7],ADC->ADC_CDR[6],ADC->ADC_CDR[5],ADC->ADC_CDR[4],ADC->ADC_CDR[3],ADC->ADC_CDR[2],ADC->ADC_CDR[1]};
  unsigned long lmillis;
  Aforth(v1);
  Bback(v1);
  Cleft(v1);
  lmillis = millis();
  while(millis()-lmillis<900){
    valueIR1 = valueIR1*gain + (1-gain)*analogRead(A0);
    valueIR2 = valueIR2*gain + (1-gain)*analogRead(A1);
    valueIR3 = valueIR3*gain + (1-gain)*analogRead(A2);
    valueIR4 = valueIR4*gain + (1-gain)*analogRead(A3);
    valueIR5 = valueIR5*gain + (1-gain)*analogRead(A4);
    valueIR6 = valueIR6*gain + (1-gain)*analogRead(A5);
    valueIR7 = valueIR7*gain + (1-gain)*analogRead(A6);
    if (valueIR1>maxthres[0]) maxthres[0] = valueIR1;
    if (valueIR1<minthres[0]) minthres[0] = valueIR1;
    if (valueIR2>maxthres[1]) maxthres[1] = valueIR2;
    if (valueIR2<minthres[1]) minthres[1] = valueIR2;
    if (valueIR3>maxthres[2]) maxthres[2] = valueIR3;
    if (valueIR3<minthres[2]) minthres[2] = valueIR3;
    if (valueIR4>maxthres[3]) maxthres[3] = valueIR4;
    if (valueIR4<minthres[3]) minthres[3] = valueIR4;
    if (valueIR5>maxthres[4]) maxthres[4] = valueIR5;
    if (valueIR5<minthres[4]) minthres[4] = valueIR5;
    if (valueIR6>maxthres[5]) maxthres[5] = valueIR6;
    if (valueIR6<minthres[5]) minthres[5] = valueIR6;
    if (valueIR7>maxthres[6]) maxthres[6] = valueIR7;
    if (valueIR7<minthres[6]) minthres[6] = valueIR7;
  }
  Aback(v1);
  Bforth(v1);
  Cright(v1);
  lmillis = millis();
  while(millis()-lmillis<1550){
    valueIR1 = valueIR1*gain + (1-gain)*analogRead(A0);
    valueIR2 = valueIR2*gain + (1-gain)*analogRead(A1);
    valueIR3 = valueIR3*gain + (1-gain)*analogRead(A2);
    valueIR4 = valueIR4*gain + (1-gain)*analogRead(A3);
    valueIR5 = valueIR5*gain + (1-gain)*analogRead(A4);
    valueIR6 = valueIR6*gain + (1-gain)*analogRead(A5);
    valueIR7 = valueIR7*gain + (1-gain)*analogRead(A6);
    if (valueIR1>maxthres[0]) maxthres[0] = valueIR1;
    if (valueIR1<minthres[0]) minthres[0] = valueIR1;
    if (valueIR2>maxthres[1]) maxthres[1] = valueIR2;
    if (valueIR2<minthres[1]) minthres[1] = valueIR2;
    if (valueIR3>maxthres[2]) maxthres[2] = valueIR3;
    if (valueIR3<minthres[2]) minthres[2] = valueIR3;
    if (valueIR4>maxthres[3]) maxthres[3] = valueIR4;
    if (valueIR4<minthres[3]) minthres[3] = valueIR4;
    if (valueIR5>maxthres[4]) maxthres[4] = valueIR5;
    if (valueIR5<minthres[4]) minthres[4] = valueIR5;
    if (valueIR6>maxthres[5]) maxthres[5] = valueIR6;
    if (valueIR6<minthres[5]) minthres[5] = valueIR6;
    if (valueIR7>maxthres[6]) maxthres[6] = valueIR7;
    if (valueIR7<minthres[6]) minthres[6] = valueIR7;
  }
  Aforth(v1);
  Bback(v1);
  Cleft(v1);
  delay(900);
  Astop();
  Bstop();
  Cstop();
  for (int i = 0;i<7;i++){
    thres[i] = (maxthres[i]+minthres[i])/2;
  }
}


////////////////////////////////////////////
void go_forth(){
  Aforth(v1);
  Bforth(v1);
  Cstop(); 
}
void turn_left(){
  Aback(v1);
  Bforth(v1);
  Cright(v1);
  turn_flag = 1;
}
void turn_right(){
  Aforth(v1);
  Bback(v1);
  Cleft(v1);
  turn_flag = 1;
}

void Aforth(int Pwm){
  analogWrite(pinA1,Pwm);
  digitalWrite(pinA2,LOW);
}
void Aback(int Pwm){
  analogWrite(pinA2,Pwm);
  digitalWrite(pinA1,LOW);
}
void Astop(){
  digitalWrite(pinA2,LOW);
  digitalWrite(pinA1,LOW);
}
void Bforth(int Pwm){
  analogWrite(pinB1,Pwm);
  digitalWrite(pinB2,LOW);
}
void Bback(int Pwm){
  analogWrite(pinB2,Pwm);
  digitalWrite(pinB1,LOW);
}
void Bstop(){
  digitalWrite(pinB2,LOW);
  digitalWrite(pinB1,LOW);
}
void Cleft(int Pwm){
  analogWrite(pinC1,Pwm);
  digitalWrite(pinC2,LOW);
}
void Cright(int Pwm){
  analogWrite(pinC2,Pwm);
  digitalWrite(pinC1,LOW);
}
void Cstop(){
  digitalWrite(pinC2,LOW);
  digitalWrite(pinC1,LOW);
}

bool ValueIR(int valueIR,int thres)
{
  if (valueIR < thres){
    return 1; // MAU TRANG
  }
  else {
    return 0; // MAU DEN
  }
}


////////////////////////////////////////////
void ItrA1() {
  if (digitalRead(encA1) != digitalRead(encA2)){
      //countA--;
      totalcountA--;
    }
    else  {
      //countA++;
      totalcountA++;
    }
}

void ItrA2() {
  if (digitalRead(encA1) != digitalRead(encA2)){
      //countA++;
      totalcountA++;
    }
    else  {
      //countA--;
      totalcountA--;
    }
}

void ItrB1() {
  if (digitalRead(encB1) != digitalRead(encB2)){
      //countB--;
      totalcountB--;
    }
    else  {
      //countB++;
      totalcountB++;
    }
}

void ItrB2() {
  if (digitalRead(encB1) != digitalRead(encB2)){
      //countB++;
      totalcountB++;
    }
    else  {
      //countB--;
      totalcountB--;
    }
}

void ItrC1() {
  if (digitalRead(encC1) != digitalRead(encC2)){
      countC--;
    }
    else  {
      countC++;
    }
}

void ItrC2() {
  if (digitalRead(encC1) != digitalRead(encC2)){
      countC++;
    }
    else  {
      countC--;
    }
}

//////////////////////////////////////////////////////////////////////////////
void led_control(){
  if (LED_CODE == 0){
    digitalWrite(Red,LOW);
    digitalWrite(Green,LOW);
    digitalWrite(Blue,LOW);
  } else if (LED_CODE == 1){
    if (millis() - led_millis > 100) {
      led_millis = millis();
      if (!dir_r) pwm_r+=20;
      else pwm_r-=20;
      
      if (pwm_r > 255) dir_r = 1;
      else if (pwm_r < 0) dir_r = 0;
      
      analogWrite(Red,pwm_r);
    }
  } else if (LED_CODE == 2){
    if (millis() - led_millis > 100) {
      led_millis = millis();
      if (!dir_b) pwm_b+=20;
      else pwm_b-=20;
      
      if (pwm_b > 255) dir_b = 1;
      else if (pwm_b < 0) dir_b = 0;
      
      analogWrite(Blue,pwm_b);
    }
  } else if (LED_CODE == 3){
    if (millis() - led_millis > 100) {
      led_millis = millis();
      if (!dir_g) pwm_g+=20;
      else pwm_g-=20;
      
      if (pwm_g > 255) dir_g = 1;
      else if (pwm_g < 0) dir_g = 0;
      
      analogWrite(Green,pwm_g);
    }
  }
}
//////////////////////////////////////////////////////////////////////////////
void send_check(int st){
  if (millis() - __millis > 3000) {
    __millis = millis();
    if (st==1){
      convertString("rr");//res ret
      send_message();   
    } else if (st==2){
      convertString("dd");//res dlvr
      send_message();
    } else if (st==3){
      convertString("zz"); // res anlz
      send_message();
    } else if (st==0){
      convertString("kk"); //res stby
      send_message();
    }
    //Serial.println(st);
  }
  
}


void _serialEvent() {
  while (Serial.available() > 0 ) {
      char incomingByte = Serial.read();
      
      if (stringOverflow) {
         serialBuffer[dataBufferIndex++] = charOverflow;  // Place saved overflow byte into buffer
         serialBuffer[dataBufferIndex++] = incomingByte;  // saved next byte into next buffer
         stringOverflow = false;                          // turn overflow flag off
      } else if (dataBufferIndex > 31) {
         stringComplete = true;        // Send this buffer out to radio
         stringOverflow = true;        // trigger the overflow flag
         charOverflow = incomingByte;  // Saved the overflow byte for next loop
         dataBufferIndex = 0;          // reset the bufferindex
         break; 
      } 
      else if(incomingByte=='\n'){
          serialBuffer[dataBufferIndex] = 0; 
          stringComplete = true;
      } else {
          serialBuffer[dataBufferIndex++] = incomingByte;
          serialBuffer[dataBufferIndex] = 0; 
      }          
  } 
}
void convertString(String str){
  //send_i2c(intersection);
  str+='#'; str+='\n';
  int len = str.length();
  int cur = 0;
   while (cur < len ) {
      char incomingByte = str[cur];
      cur++;
      if (stringOverflow) {
         serialBuffer[dataBufferIndex++] = charOverflow;  // Place saved overflow byte into buffer
         serialBuffer[dataBufferIndex++] = incomingByte;  // saved next byte into next buffer
         stringOverflow = false;                          // turn overflow flag off
      } else if (dataBufferIndex > 31) {
         stringComplete = true;        // Send this buffer out to radio
         stringOverflow = true;        // trigger the overflow flag
         charOverflow = incomingByte;  // Saved the overflow byte for next loop
         dataBufferIndex = 0;          // reset the bufferindex
         break; 
      } 
      else if(incomingByte=='\n'){
          serialBuffer[dataBufferIndex] = 0; 
          stringComplete = true;
      } else {
          serialBuffer[dataBufferIndex++] = incomingByte;
          serialBuffer[dataBufferIndex] = 0; 
      }          
  } 
  
}

void nRF_receive(void) {
  int len = 0;
  if ( radio.available() ) {
      bool done = false;
      while (  radio.available() ) {
        len = radio.getDynamicPayloadSize();
        radio.read(&RecvPayload,len);
        delay(1);
      }
    RecvPayload[len] = 0;
    //Serial.println(RecvPayload);
    if (RecvPayload[0] == '$') {
      analyze_flag=1;
    } else if (RecvPayload[0] == '^'){
      convertString("R " + SEND.substring(2) );
      send_message();
    }
    else {
      ORDER = "";
      rev_ORDER = "";
      cur_ord = 1;
      cur_rev = 0;
      ord_dis = 0;
      int i = 0; 
      while (RecvPayload[i]!=0){
        if (RecvPayload[i]-48 <= 9 && RecvPayload[i]-48 >= 0)
          ord_dis = ord_dis*10 + RecvPayload[i] - 48;
        else
          ORDER += RecvPayload[i];
        i++;
      }
      for (int i = ORDER.length()-1; i >= 0; i--){
        if (ORDER[i]=='S') rev_ORDER += 'S';
        if (ORDER[i]=='L') rev_ORDER += 'R';
        if (ORDER[i]=='R') rev_ORDER += 'L';
      }
      
    }
    //Serial.println(ORDER);
//    Serial.println(ord_dis);
    RecvPayload[0] = 0; 
    last_ORDER = ORDER;
  }  
}

//////////////////////////////////////////////////////////////////
bool nRF_other_order(void) {
  int len = 0;
  bool ret = 1;
  if ( radio.available() ) {
      bool done = false;
      while (  radio.available() ) {
        len = radio.getDynamicPayloadSize();
        radio.read(&RecvPayload,len);
        delay(1);
      }
    RecvPayload[len] = 0;
    ORDER2 = "";
    rev_ORDER = "";
    cur_ord = 0;
    cur_rev = 0;
    ord_dis = 0;
    int i = 0; 
    while (RecvPayload[i]!=0){
        if (RecvPayload[i]-48 <= 9 && RecvPayload[i]-48 >= 0)
          ord_dis = ord_dis*10 + RecvPayload[i] - 48;
        else
          ORDER2 += RecvPayload[i];
        i++;
      }
      
    i = 0;
    while (ORDER1[i] == ORDER2[i]){
      i++;
    } 
    
    if (i==ORDER1.length()) {ret = 0;i--;}
    if (i==ORDER2.length()) i--;
    
    for (int j = ORDER1.length()-1; j > i; j--){
      if (ORDER1[j]=='S') ORDER += 'S';
      if (ORDER1[j]=='L') ORDER += 'R';
      if (ORDER1[j]=='R') ORDER += 'L';
    }
    
    if (ORDER1[i]=='S'){
      if (ORDER2[i]=='L') ORDER += 'R';
      if (ORDER2[i]=='R') ORDER += 'L'; 
    } else
    if (ORDER1[i]=='L'){
      if (ORDER2[i]=='S') ORDER += 'L';
      if (ORDER2[i]=='R') ORDER += 'S'; 
    } else
    if (ORDER1[i]=='R'){
      if (ORDER2[i]=='S') ORDER += 'R';
      if (ORDER2[i]=='L') ORDER += 'S'; 
    } 
    
    for (int j = i+1; j < ORDER2.length(); j++){
      ORDER += ORDER2[j];
    }
    
    for (int i = ORDER2.length()-1; i >= 0; i--){
      if (ORDER2[i]=='S') rev_ORDER += 'S';
      if (ORDER2[i]=='L') rev_ORDER += 'R';
      if (ORDER2[i]=='R') rev_ORDER += 'L';
    }
  RecvPayload[0] = 0; 
  last_ORDER = ORDER2;
  } else  
    return_flag = 1;
  return ret;
}

/////////////////////////////////////////////////////////////////////
void I2Cevent(int num){
  byte c = Wire.read();
  //Serial.println(c);
  if (c==1) OBS = 1;
  if (c==2) OBS = 0;
  if (c==4) CUP = 0;
  if (c==8) CUP = 1;
}

void send_message(void){
  
  if (stringComplete) { 
        strcat(SendPayload,serialBuffer);      
        // swap TX & Rx addr for writing
        radio.openWritingPipe(pipes[1]);
        radio.openReadingPipe(0,pipes[0]);  
        radio.stopListening();
        bool ok = radio.write(&SendPayload,strlen(SendPayload));
        
        //Serial.write(SendPayload);          
        stringComplete = false;
     
        radio.openWritingPipe(pipes[0]);
        radio.openReadingPipe(1,pipes[1]); 
        radio.startListening();  
        SendPayload[0] = 0;
        dataBufferIndex = 0;
  }
}
/////////////////////////////////////////////////////////////////////
void radio_setup(){
  printf_begin();
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(70);
  radio.enableDynamicPayloads();
  radio.setRetries(15,15);
  radio.setCRCLength(RF24_CRC_16);
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);  
  radio.startListening();
  radio.printDetails();
}

void pin_setup(){
  pinMode(pinA1,OUTPUT);
  pinMode(pinA2,OUTPUT);
  pinMode(pinB1,OUTPUT);
  pinMode(pinB2,OUTPUT);
  pinMode(pinC1,OUTPUT);
  pinMode(pinC2,OUTPUT);
  pinMode(encA1,INPUT);
  pinMode(encA2,INPUT);
  pinMode(encB1,INPUT);
  pinMode(encB2,INPUT);
  pinMode(encC1,INPUT);
  pinMode(encC2,INPUT);
  attachInterrupt(digitalPinToInterrupt(encA1),ItrA1,CHANGE);
  attachInterrupt(digitalPinToInterrupt(encA2),ItrA2,CHANGE);
  attachInterrupt(digitalPinToInterrupt(encB1),ItrB1,CHANGE);
  attachInterrupt(digitalPinToInterrupt(encB2),ItrB2,CHANGE);
  //attachInterrupt(digitalPinToInterrupt(encC1),ItrC1,CHANGE);
  //attachInterrupt(digitalPinToInterrupt(encC2),ItrC2,CHANGE);
}

