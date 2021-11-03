#include <Arduino.h>
#include <BLEDevice.h>

#define ADDRESS "ff:ff:c2:07ab:16"
#define RELAY_PIN 2
#define SCAN_INTERVAL 1000
#define TARGET_RSSI -100
#define MAX_MISSING_TIME 5000

BLEScan *presenceBLEScan;
uint32_t lastScanTime = 0;
boolean found = false;
uint32_t lastFoundTime = 0;
int rssi = 0;

void setup()
{
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  BLEDevice::init(""); // Metodo estatico ::
  presenceBLEScan = BLEDevice::getScan();
  //presenceBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  presenceBLEScan->setActiveScan(true);
}

void loop()
{
  uint32_t now = millis();
  if (found)
  {
    lastFoundTime = now;
    found = false;
    if (rssi > TARGET_RSSI)
    {
      digitalWrite(RELAY_PIN, HIGH);
    }
    else
    {
      digitalWrite(RELAY_PIN, LOW);
    }
  }
  else if (now - lastFoundTime > MAX_MISSING_TIME)
  {
    digitalWrite(RELAY_PIN, LOW);
  }

  if (now - lastScanTime > SCAN_INTERVAL)
  {
    lastScanTime = now;
    presenceBLEScan->start(1);
  }
}

class MyAdvertiseDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    Serial.print("Device Found: ");
    Serial.println(advertisedDevice.toString().c_str());

    if (advertisedDevice.getAddress().toString() == ADDRESS)
    {
      found = true;
      advertisedDevice.getScan()->stop();
      rssi = advertisedDevice.getRSSI();
      Serial.println("RSSI: " + rssi);
    }
  }
};