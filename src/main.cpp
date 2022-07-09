#include <Arduino.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEService.h>

#define HEART_RATE_SERVICE "180D"
#define HEART_RATE_MEASUREMENT_CHARACTERISTIC "2A37"

BLEServer *server = nullptr;
BLECharacteristic *hrCharacteristic = nullptr;

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *server) {
        Serial.print("Client connected count=");
        Serial.println(
            server->getConnectedCount() +
            1);  // Value doesn't get updated until this function exits
        BLEDevice::startAdvertising();
    }

    void onDisconnect(BLEServer *server) {
        Serial.print("Client disconnected count=");
        Serial.println(server->getConnectedCount() - 1);
    }
};

BLEServerCallbacks *callbacks = new ServerCallbacks();

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE server...");

    BLEDevice::init("Fake HRM");
    server = BLEDevice::createServer();
    server->setCallbacks(callbacks);

    BLEService *hrService = server->createService(HEART_RATE_SERVICE);

    // Indicate requires acknowledgement, notify does not
    hrCharacteristic = hrService->createCharacteristic(
        HEART_RATE_MEASUREMENT_CHARACTERISTIC,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_INDICATE);

    // 2902 Is a descriptor that allows notifications to be enabled
    hrCharacteristic->addDescriptor(new BLE2902());
    hrService->start();

    BLEAdvertising *advertising = BLEDevice::getAdvertising();

    advertising->addServiceUUID(HEART_RATE_SERVICE);
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06);
    advertising->setMaxPreferred(0x12);

    BLEDevice::startAdvertising();
}

void loop() {
    if (server->getConnectedCount()) {
        uint8_t currentHr = random(120, 180) & 0xFF;
        uint8_t data[2] = {0b00000000, currentHr};
        hrCharacteristic->setValue((uint8_t *)&data, 2);
        hrCharacteristic->notify();

        Serial.print("Sending heartrate=");
        Serial.println(currentHr);

        delay(3000);
    }
}