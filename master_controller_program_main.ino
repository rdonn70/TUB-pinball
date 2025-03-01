//PD2: Flipper 5V Activate
//PD3: Drop Down Solenoid Activate (drop down target reset)
//PD4: Drop Down "Column1"
//PD5: Drop Down "Row1"
//PD6: Drop Down "Row2"
//PD7: Drop Down "Row3"
//PB2: Tilt Switch
//PB1: Ball Return Jam Detect
//PB0: Ball Return Detect (ready to trigger ball return solenoid)

#include <Wire.h>

char received_data;
int tilt_count = 0;
int lives = 3;

void setup() {
  pinMode(2, OUTPUT); //PD2
  pinMode(3, OUTPUT); //PD3
  pinMode(4, INPUT); //PD4
  pinMode(5, INPUT); //PD5
  pinMode(6, INPUT); //PD6
  pinMode(7, INPUT); //PD7
  pinMode(8, INPUT); //PB0
  pinMode(9, INPUT); //PB1
  pinMode(10, INPUT_PULLUP); //PB2

  Wire.begin();
  Wire.onReceive(receive);
  Serial.begin(9600);
  Serial.print("SCOR_DRIV");
}

void loop() {
  bool col = digitalRead(4);
  bool row1 = digitalRead(5);
  bool row2 = digitalRead(6);
  bool row3 = digitalRead(7);
  bool return_detect = digitalRead(8);
  bool jam_detect = digitalRead(9);
  bool tilt = digitalRead(10);

  digitalWrite(2, 1); //set flippers to always be ready to be flipped

  if((row1 == 1) && (row2 == 1) && (row3 == 1) && (col == 1)) { // if all the drop down switches are activated, 
    digitalWrite(3, 1);
    delay(1);
    digitalWrite(3, 0);
  }
  if((return_detect == 1) && (lives != 0)) {
    lives -= 1;
    Wire.beginTransmission(0x01);
    Wire.write(1); //send to ball return that a ball is able to be fired.
    Wire.endTransmission(true);
    if(jam_detect == 1) {
      Wire.beginTransmission(0x01);
      Wire.write(1);
      Wire.endTransmission(true);
    }
  }
  if(tilt == 1) {
    tilt_count += 1;
  }
  if(tilt_count >= 2) {
    digitalWrite(2, 0); // if the player is warned 2+ times for tilting the machine, disable the flippers for 5 seconds.
    delay(5000);
    tilt_count = 0;
  }

  delay(100);
}

void receive() {
  while(Wire.available()) {
    received_data = Wire.read();
  }
}