#include "homephone.h"
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

homephone lcd(7,5,6,4,8);

RF24 radio(9,10);

const uint64_t pipes[2] = { 0xDEDEDEDEE7LL, 0xDEDEDEDEE9LL };

boolean stringComplete = false;  // whether the string is complete
static int dataBufferIndex = 0;
boolean stringOverflow = false;
char charOverflow = 0;

char SendPayload[31] = "";
char RecvPayload[31] = "";
char serialBuffer[31] = "";

String IP;
bool writeLCD = false;
int stt; //0:off 1:standby 2:working

void setup(void) {

  Serial.begin(9600);
  printf_begin();
  
  lcd.begin();
  lcd.setContrast(0x10);
  lcd.clearDisplay();
  lcd.display();
  
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
  //radio.printDetails();

  //Serial.println();
  delay(500);
}

void loop(void) {
  
  nRF_receive();
  serial_receive();
  
}

void serialEvent() {
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
} // end serialEvent()

void nRF_receive(void) {
  int len = 0;
  if ( radio.available() ) {
      bool done = false;
      while (  radio.available() ) {
        len = radio.getDynamicPayloadSize();
        radio.read(&RecvPayload,len);
        delay(1);
      }
  
    RecvPayload[len] = 0; // null terminate string
    
    Serial.write(RecvPayload);
    RecvPayload[0] = 0;  // Clear the buffers
  }  

} // end nRF_receive()

void serial_receive(void){
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
        delay(1);
  }
}


