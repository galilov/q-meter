
#define RELAY_PIN   5
#define SHUNT_PIN   A1
#define LED_DIN     10
#define LED_CS      11
#define LED_CLK     12
#define BTN_START   9
#define BTN_STOP    8
#define SHUNT_R     1.948
#define MAX_ADC_VAL 1023
#define A_REF_MILLIVOLTS      1100L
#define RECALC_INTERVAL_MILLIS 500

#include "led.h"

Led _led(LED_DIN, LED_CLK, LED_CS);

char _buf[20];
uint32_t _accumulatedAdcVal = 0;
uint32_t _nAnalogReadIterations = 0;
uint32_t _millisStartInterval = UINT32_MAX;
uint64_t _chargeMicroQ = 0;

void turnRelayOn() {
  digitalWrite(RELAY_PIN, HIGH);
}

void turnRelayOff() {
  digitalWrite(RELAY_PIN, LOW);
}

int getRelayStatus() {
  return digitalRead(RELAY_PIN);
}

int getBtnStartStatus() {
  return digitalRead(BTN_START);
}

int getBtnStopStatus() {
  return digitalRead(BTN_STOP);
}

int electricCurrentMa(int adcValue) {
  return (int)(A_REF_MILLIVOLTS * adcValue / MAX_ADC_VAL / SHUNT_R);
}

void showElectricCurrentError() {
  _led.print("Curr Err");
}

void updateAdcAverageValue() {
  int val = analogRead(SHUNT_PIN);
  _accumulatedAdcVal += val;
  _nAnalogReadIterations++;
//  Serial.print(_accumulatedAdcVal);
//  Serial.print(" ");
//  Serial.print(_nAnalogReadIterations);
//  Serial.print(" ");
//  Serial.println(_accumulatedAdcVal/_nAnalogReadIterations);
}

int getAdcValueAverage() {
  if (_nAnalogReadIterations == 0) return 0;
  return (int)(_accumulatedAdcVal / _nAnalogReadIterations);
}

void clearAdcAccumulatedValue() {
  _nAnalogReadIterations = 0;
  _accumulatedAdcVal = 0;
}


void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BTN_START, INPUT);
  pinMode(BTN_STOP, INPUT);
  analogReference(INTERNAL); // 1.1v for ATmega328P
  _led.setup();
  _led.print("HELLO");
  turnRelayOff();
  //Serial.begin(115200);
  delay(500);
}

// the loop function runs over and over again forever
void loop() {
  uint32_t currentMs = millis();
  if (_millisStartInterval == UINT32_MAX) {
    _millisStartInterval = currentMs;
  }

  updateAdcAverageValue();

  uint32_t deltaMs = currentMs - _millisStartInterval;
  //Serial.println(deltaMs);
  if (deltaMs > RECALC_INTERVAL_MILLIS) {
    _millisStartInterval = currentMs;
    int valAdc = getAdcValueAverage();
    int currentMa = electricCurrentMa(valAdc);
    _chargeMicroQ += (currentMa * deltaMs);
    clearAdcAccumulatedValue();
    sprintf(_buf, "%03u %04u", currentMa, (int)(_chargeMicroQ / 1000000L));
    _led.print(_buf);
  }

  if (getRelayStatus() == HIGH && getBtnStopStatus() == LOW) {
    turnRelayOff();
  } else if (getRelayStatus() == LOW && getBtnStartStatus() == LOW) {
    turnRelayOn();
  }

}
