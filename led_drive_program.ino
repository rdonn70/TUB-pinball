/*
LED Driver Code

Hardware: 12 daisy chained 8 bit shift registers
Software: Takes in 12 bytes of serial data, outputs this data to the LEDs. There are 32 LEDs available to call, each LED has R, G, B LEDs for a total of 96 addressable LEDs.

*/

char serial_data = 0; // char for storing read serial data.
uint8_t led_code[12]; // array to hold 1 byte chunks of data * 12 shift registers = (96 LEDs [R, G, B])

void setup() {
  pinMode(5, OUTPUT); //PD5 - register clock pin (latch)
  pinMode(4, OUTPUT); //PD4 - shift register clock pin
  pinMode(3, OUTPUT); //PD3 - clear registers pin (active low)
  pinMode(6, OUTPUT); //PD6 - serial data pin
  
  digitalWrite(3, LOW);
  delay(100);
  digitalWrite(3, HIGH); //clear registers, then allow for data
  
  Serial.begin(9600);
  Serial.print("LED_DRIVE");
}

void loop() {
  if(Serial.available() >= 12) {
    for(int i = 0; i < 12; i++) {
      serial_data = Serial.read();
      led_code[i] = serial_data - '0';
    }

    digitalWrite(5, LOW);
    for(int i = 0; i < 12; i++) {
      shiftOut(6, 4, LSBFIRST, led_code);
    }
    digitalWrite(5, HIGH);
  }
}
