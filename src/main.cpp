#include <Arduino.h>
#include <Wire.h>

#define CLOCK_I2C_ADDR 0x68
#define RELAY_PIN 2

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.println("initializing...");
  digitalWrite(RELAY_PIN, LOW);
  pinMode(RELAY_PIN, OUTPUT);
}

struct {
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hours;
  uint8_t dayOfWeek;
  uint8_t dayOfMonth;
  uint8_t month;
  uint16_t year;
} datetime;

void writeDatetime() {

}

void readDatetime() {
  uint8_t data;
  Wire.beginTransmission(CLOCK_I2C_ADDR);
  Wire.write(0);
  Wire.endTransmission(false);
  Wire.requestFrom(CLOCK_I2C_ADDR, 7);

  // Seconds
  data = Wire.read();
  datetime.seconds = ((data >> 4) * 10) + (data & 0x0f);

  // Minutes
  data = Wire.read();
  datetime.minutes = ((data >> 4) * 10) + (data & 0x0f);

  // Hours
  data = Wire.read();
  datetime.hours = (((data  >> 4) & 0x01) * 10) + (data & 0x0f);

  // Day of week
  data = Wire.read();
  datetime.dayOfWeek = data & 0x03;

  // Day of month
  data = Wire.read();
  datetime.dayOfMonth = ((data >> 4) * 10) + (data & 0x0f);

  // Month
  data = Wire.read();
  datetime.month = (((data  >> 4) & 0x01) * 10) + (data & 0x0f);

  // Year
  data = Wire.read();
  datetime.year = ((data  >> 4) * 10) + (data & 0x0f);
  Wire.endTransmission();
}

void printDatetime() {
  Serial.print(datetime.dayOfMonth);
  Serial.print("/");
  Serial.print(datetime.month);
  Serial.print("/");
  Serial.print(datetime.year);
  Serial.print(" ");
  Serial.print(datetime.hours);
  Serial.print(":");
  Serial.print(datetime.minutes);
  Serial.print(":");
  Serial.println(datetime.seconds);
}

void loop() {
  readDatetime();
  printDatetime();
  digitalWrite(RELAY_PIN, datetime.minutes % 2 == 0);
  delay(1000);
}
