//PD2: Slingshot Left-side Solenoid
//PD3: Slingshot Right-side Solenoid
//PD4: Scoop Weldment Solenoid
//PD5: Motor Driver/Optional Mosfet device
//PD6: Scoop Weldment Switch
//PD7: Start Button Switch Press
//PB0: Start Button LED
//PB1: Slingshot Left-side Switches
//PB2: Slingshot Right-side Switches

#include <Wire.h>

char received_data;
bool slingL; bool slingR; bool scoop; bool start;
bool rd1; bool rd2; bool rd3; bool rd4; bool rd5; bool rd6; bool rd7; bool rd8;

void setup() {
  pinMode(2, OUTPUT); //PD2
  pinMode(3, OUTPUT); //PD3
  pinMode(4, OUTPUT); //PD4
  pinMode(5, OUTPUT); //PD5
  pinMode(6, INPUT); //PD6
  pinMode(7, INPUT_PULLUP); //PD7
  pinMode(8, OUTPUT); //PB0
  pinMode(9, INPUT_PULLUP); //PB1
  pinMode(10, INPUT_PULLUP); //PB2

  Wire.begin(0x02);
  Wire.onRequest(send);
  Wire.onReceive(receive);
}

void loop() {
  slingL = digitalRead(9);
  slingR = digitalRead(10);
  scoop = digitalRead(6);
  start = digitalRead(7);
  
  if(slingL == 0) {
    digitalWrite(2, 1);
    delay(1);
    digitalWrite(2, 0);
  }
  if(slingR == 0) {
    digitalWrite(3, 1);
    delay(1);
    digitalWrite(3, 0);
  }
  if(rd8 == 1) { // scoop weldment activate
    digitalWrite(4, 1);
    delay(1);
    digitalWrite(4, 0);
  }
  delay(100);
}

void send() {
  char send_data = ((!slingL) << 7) | ((!slingR) << 6) | ((!start) << 5) | (scoop << 4) | (0 << 3) | (0 << 2) | (0 << 1) | (0 << 0);
  Wire.write(send_data);
  slingL = 1;
  slingR = 1;
}

void receive() {
  while(Wire.available()) {
    received_data = Wire.read();
    rd1 = (received_data >> 0) & 1; //LSB
    rd2 = (received_data >> 1) & 1; 
    rd3 = (received_data >> 2) & 1; 
    rd4 = (received_data >> 3) & 1; 
    rd5 = (received_data >> 4) & 1; 
    rd6 = (received_data >> 5) & 1; 
    rd7 = (received_data >> 6) & 1; // start LED trigger
    rd8 = (received_data >> 7) & 1; //MSB - scoop trigger
  }
}