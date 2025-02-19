/*
LED Driver Code

Hardware: 12 daisy chained 8 bit shift registers
Software: Takes in 13 bytes of serial data, outputs byte 0 to LED power control and bytes 1-13 of data to the LEDs. There are 32 LEDs available to call, each LED has R, G, B LEDs for a total of 96 addressable LEDs.

*/

char serial_data = 0; // char for storing read serial data.
uint8_t led_code[12]; // array to hold 1 byte chunks of data * 12 shift registers = (96 LEDs [R, G, B])

void setup() {
  pinMode(5, OUTPUT); //PD5 - register clock pin (latch)
  pinMode(4, OUTPUT); //PD4 - shift register clock pin
  pinMode(3, OUTPUT); //PD3 - clear registers pin (active low)
  pinMode(6, OUTPUT); //PD6 - serial data pin
  pinMode(9, OUTPUT); //PB1 - LED Power, Active High (can do PWM for dimming)
  
  digitalWrite(3, LOW);
  delay(100);
  digitalWrite(3, HIGH); //clear registers, then allow for data
  
  Serial.begin(9600);
  Serial.print("LED_DRIVE");
}

void loop() { 
  if(Serial.available() >= 13) {
    serial_data = Serial.read();
    led_code[0] = serial_data - '0'; // First byte of the 13 bytes controls the LED brightness (PWM)

    for(int i = 1; i < 12; i++) {
      serial_data = Serial.read();
      led_code[i] = serial_data - '0';
    }

    digitalWrite(5, LOW);
    for(int i = 1; i < 13; i++) {
      shiftOut(6, 4, LSBFIRST, led_code[i]);
    }
    digitalWrite(5, HIGH);
    
    if(led_code[0] == 1) {
      analogWrite(9, 25);
    } else if(led_code[0] == 2) {
      analogWrite(9, 50);
    } else if(led_code[0] == 3) {
      analogWrite(9, 75);
    } else if(led_code[0] == 4) {
      analogWrite(9, 100);
    } else if(led_code[0] == 5) {
      analogWrite(9, 125);
    } else if(led_code[0] == 6) {
      analogWrite(9, 150);
    } else if(led_code[0] == 7) {
      analogWrite(9, 175);
    } else if(led_code[0] == 8) {
      analogWrite(9, 200);
    } else if(led_code[0] == 9) {
      analogWrite(9, 250);
    } else {
      analogWrite(9, 0);
    }
  }
}
