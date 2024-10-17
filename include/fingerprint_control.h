#ifndef FINGERPRINT_CONTROL_H
#define FINGERPRINT_CONTROL_H

#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

#define TEMPLATE_SIZE 512
#define SERVER_URL "http://192.168.1.13:4444/upload-template"

class FingerprintControl {
public:
    FingerprintControl(HardwareSerial* serial, int rx, int tx);
    void begin();
    bool verifyPassword();
    uint8_t getParameters();
    uint8_t getImage();
    uint8_t image2Tz(uint8_t slot = 1);
    uint8_t createModel();
    uint8_t storeModel(uint16_t id);
    uint8_t loadModel(uint16_t id);
    uint8_t getModel();
    uint8_t deleteModel(uint16_t id);
    uint8_t emptyDatabase();
    uint8_t fingerFastSearch();
    uint8_t readTemplate(uint8_t fingerprintId);
    uint8_t uploadTemplate(uint16_t id);
    uint8_t downloadFingerprintTemplate(uint16_t id);

    uint16_t fingerID;
    uint16_t confidence;

private:
    Adafruit_Fingerprint* _finger;
    HardwareSerial* _serial;
    int _rx, _tx;
    void printHex(int num, int precision);
    uint8_t templateBuffer[TEMPLATE_SIZE];
    uint8_t storedTemplate[TEMPLATE_SIZE];
};

#endif // FINGERPRINT_CONTROL_H
