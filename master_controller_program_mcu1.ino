//PD2: Ball Return Solenoid Trigger
//PD3: Ball Return Switch 1
//PD4: Ball Return Switch 2
//PD5: Ball Return Switch 3
//PD6: Ball Return Switch 4
//PD7: Ball Return Switch 5
//PB0: Ball Return Switch 6
//PB1: Ball Return Switch 7
//PB2: Ball Return Switch 8

#include <Wire.h>

char received_data;
char switches;
bool br1; bool br2; bool br3; bool br4; bool br5; bool br6; bool br7; bool br8;

void setup() {
  pinMode(2, OUTPUT); //PD2
  pinMode(3, INPUT_PULLUP); //PD3
  pinMode(4, INPUT_PULLUP); //PD4
  pinMode(5, INPUT_PULLUP); //PD5
  pinMode(6, INPUT_PULLUP); //PD6
  pinMode(7, INPUT_PULLUP); //PD7
  pinMode(8, INPUT_PULLUP); //PB0
  pinMode(9, INPUT_PULLUP); //PB1
  pinMode(10, INPUT_PULLUP); //PB2

  Wire.begin(0x01);
  Wire.onRequest(send);
  Wire.onReceive(receive);
}

void loop() {
  br1 = digitalRead(3);
  br2 = digitalRead(4);
  br3 = digitalRead(5);
  br4 = digitalRead(6);
  br5 = digitalRead(7);
  br6 = digitalRead(8);
  br7 = digitalRead(9);
  br8 = digitalRead(10);
  delay(1000);
}

void send() {
  switches = (br8 << 7) | (br7 << 6) | (br6 << 5) | (br5 << 4) | (br4 << 3) | (br3 << 2) | (br2 << 1) | (br1 << 0);
  Wire.write(switches);
}

void receive() {
  while(Wire.available()) {
    received_data = Wire.read();
    if((received_data - '0' == 1) && (switches = 0)) {
      digitalWrite(2, 1);
      delay(1);
      digitalWrite(2, 0);
      received_data = 0;
    }
  }
}