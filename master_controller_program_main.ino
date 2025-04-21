//PD2: Flipper 5V Activate
//PD3: Drop Down Solenoid Activate (drop down target reset)
//PD4: Drop Down "Column1"
//PD5: Drop Down "Row1"
//PD6: Drop Down "Row2"
//PD7: Drop Down "Row3"
//PB0: Ball Return Detect (ready to trigger ball return solenoid)
//PB1: Ball Return Jam Detect
//PB2: Tilt Switch

#include <Wire.h>

char i2c_received_byte;
int tilt_count = 0;
int lives = 3;
long score = 0;

bool row1_last_state = 0;
bool row2_last_state = 0;
bool row3_last_state = 0;
bool prev_tilt = 0;
bool col;
bool row1;
bool row2;
bool row3;
bool return_detect;
bool jam_detect;
bool tilt;
int incoming_byte;
bool game_active = 0;

void setup() {
  pinMode(2, OUTPUT); //PD2
  pinMode(3, OUTPUT); //PD3
  pinMode(4, INPUT); //PD4
  pinMode(5, INPUT); //PD5
  pinMode(6, INPUT); //PD6
  pinMode(7, INPUT); //PD7
  pinMode(8, INPUT_PULLUP); //PB0
  pinMode(9, INPUT_PULLUP); //PB1
  pinMode(10, INPUT_PULLUP); //PB2

  Wire.begin();
  Serial.begin(9600);
  Serial.print("SCOR_DRIV");
  
  digitalWrite(2, LOW); //set flippers to not be able to be flipped unless the game is active
  digitalWrite(3, LOW);
  delay(1000); //allow other MCUs to startup properly
}

void loop() {
  col = digitalRead(4);
  row1 = digitalRead(5);
  row2 = digitalRead(6);
  row3 = digitalRead(7);
  return_detect = digitalRead(8);
  jam_detect = digitalRead(9);
  tilt = digitalRead(10);

  if (Serial.available() > 0) {
	incoming_byte = Serial.read();
	if(incoming_byte == 255) {
		game_active = 1;
	} else {
		game_active = 0;
	}
  }

  if(game_active == 1) {
	digitalWrite(2, HIGH);  // allow flippers to be flipped
  } else {
	digitalWrite(2, LOW);  
  }

  if(row1 != row1_last_state) {  
    row1_last_state = row1;  
    if(row1 == HIGH) {
		score += 200;
	}
  }
  if(row2 != row2_last_state) {  
    row2_last_state = row1;  
    if(row2 == HIGH) {
		score += 200;
	}
  }
  if(row3 != row3_last_state) {  
    row3_last_state = row1;  
    if(row3 == HIGH) {
		score += 200;
	}
  }

  if((row1 == 1) && (row2 == 1) && (row3 == 1) && (col == 1)) { // if all the drop down switches are activated, reset and log score
    digitalWrite(3, HIGH);
    delay(10);
    digitalWrite(3, LOW);
	score += 1000;
  }

	Wire.requestFrom(0x01, 1); // Request 1 byte from slave at address 0x02
	if (Wire.available()) {
	  i2c_received_byte = Wire.read(); // Read byte from slave
	  Serial.println("data from 0x01: "); //debug
	  Serial.print(i2c_received_byte);
	}
	delay(10);
	Wire.requestFrom(0x02, 1);
	if (Wire.available()) {
	  i2c_received_byte = Wire.read(); // Read byte from slave
	  Serial.println("data from 0x02: "); //debug
	  Serial.print(i2c_received_byte);
	}
	delay(10);
	Wire.requestFrom(0x03, 1);
	if (Wire.available()) {
	  i2c_received_byte = Wire.read(); // Read byte from slave
	  Serial.println("data from 0x03: "); //debug
	  Serial.print(i2c_received_byte);
	}

  if((return_detect == 0) && (lives != 0)) {
    lives -= 1;
    Wire.beginTransmission(0x01);
    Wire.write(0x11); //send to ball return that a ball is able to be fired.
    Wire.endTransmission();
	
    if(jam_detect == 1) {
      Wire.beginTransmission(0x01);
      Wire.write(0x22);
      Wire.endTransmission();
    }
  }
  if(prev_tilt == 0 && tilt == 1) {
    tilt_count += 1;
  }
  prev_tilt = tilt;
  
  if(tilt_count > 2) {
    digitalWrite(2, LOW); //if the player is warned over 2 times for tilting the machine, disable the flippers for 5 seconds, stop scoring.
    delay(5000);
	digitalWrite(2, HIGH);
    tilt_count = 0;
  }

  Serial.print(score); //send score to raspberry pi
  score = 0;

  delay(100);
}
