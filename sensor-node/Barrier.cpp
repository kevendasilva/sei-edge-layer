#include <Arduino.h>
#include "Barrier.h"

// Define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034

// Positions actuator
#define INITIAL_POSITION 25
#define FINAL_POSITION 195

Barrier::Barrier(int ledFailPin, int ledSuccessPin, int trigSensorPin, int echoSensorPin, int actuatorPin) {
  pinMode(ledFailPin, OUTPUT);
  pinMode(ledSuccessPin, OUTPUT);
  pinMode(trigSensorPin, OUTPUT);
  pinMode(echoSensorPin, INPUT);

  this->ledFailPin = ledFailPin;
  this->ledSuccessPin = ledSuccessPin;
  this->trigSensorPin = trigSensorPin;
  this->echoSensorPin = echoSensorPin;

  actuator.attach(actuatorPin);
}

float Barrier::getDistance() {
  long duration;

  digitalWrite(trigSensorPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigSensorPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigSensorPin, LOW);

  duration = pulseIn(echoSensorPin, HIGH);

  float distance;
  // Calculate the distance
  distance = duration * SOUND_VELOCITY / 2;

  return distance;
}

void Barrier::close() {
  turnOffLED(ledSuccessPin);
  actuator.write(INITIAL_POSITION);
}

void Barrier::errorSignal(short blinkCount, short delayInMilliseconds) {
  while (blinkCount > 0) {
    turnOnLED(ledFailPin);
    delay(delayInMilliseconds);
    turnOffLED(ledFailPin);
    delay(delayInMilliseconds);

    blinkCount--;
  } 
}

void Barrier::loadingSignal(short blinkCount, short delayInMilliseconds) {
  while (blinkCount > 0) {
    turnOnLED(ledSuccessPin); turnOnLED(ledFailPin);
    delay(delayInMilliseconds);
    turnOffLED(ledSuccessPin); turnOffLED(ledFailPin);
    delay(delayInMilliseconds);

    blinkCount--;
  }
}

void Barrier::open() {
  turnOnLED(ledSuccessPin);
  actuator.write(FINAL_POSITION);
}

void Barrier::setDistanceLimit(int distanceLimit, int distanceLimitMarginError) {
  this->distanceLimit = distanceLimit;
  this->distanceLimitMarginError = distanceLimitMarginError;
}

void Barrier::turnOffLED(unsigned short pin) {
  digitalWrite(pin, LOW);
}

void Barrier::turnOnLED(unsigned short pin) {
  digitalWrite(pin, HIGH);
}

bool Barrier::vehicleIsNear() {
  float distance = getDistance();

  if (distanceLimit - distanceLimitMarginError <= distance &&
      distance <= distanceLimit + distanceLimitMarginError) {
    return true;
  }

  return false;
}

void Barrier::waitingSignal(short blinkCount, short delayInMilliseconds) {
  while (blinkCount > 0) {
    turnOnLED(ledSuccessPin); turnOffLED(ledFailPin);
    delay(delayInMilliseconds);
    turnOffLED(ledSuccessPin); turnOnLED(ledFailPin);
    delay(delayInMilliseconds);
    turnOffLED(ledFailPin);

    blinkCount--;
  }
}
