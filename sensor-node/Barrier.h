#ifndef BARRIER_H_INCLUDED
#define BARRIER_H_INCLUDED
  class Barrier {
    public:
      Barrier(int ledFailPin, int ledSuccessPin, int trigSensorPin, int echoSensorPin);
      void loadingSignal(short blinkCount, short delayInMilliseconds);
      void open();
      bool vehicleIsNear();
      void setDistanceLimit(int distanceLimit, int distanceLimitMarginError);
      void waitingSignal(short blinkCount, short delayInMilliseconds);
    private:
      int ledFailPin;
      int ledSuccessPin;
      int trigSensorPin;
      int echoSensorPin;
      int distanceLimit;
      int distanceLimitMarginError;

      float getDistance();
      void turnOnLED(unsigned short pin);
      void turnOffLED(unsigned short pin);
  };
#endif
