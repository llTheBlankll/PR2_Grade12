#include "fingerprint_control.h"
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <base64.h>

FingerprintControl::FingerprintControl(HardwareSerial *serial, int rx, int tx)
    : _serial(serial), _rx(rx), _tx(tx)
{
    _finger = new Adafruit_Fingerprint(serial);
}

void FingerprintControl::begin()
{
    _serial->begin(57600, SERIAL_8N1, _rx, _tx);
    _finger->begin(57600);
}

bool FingerprintControl::verifyPassword()
{
    return _finger->verifyPassword();
}

uint8_t FingerprintControl::getParameters()
{
    return _finger->getParameters();
}

uint8_t FingerprintControl::getImage()
{
    return _finger->getImage();
}

uint8_t FingerprintControl::image2Tz(uint8_t slot)
{
    return _finger->image2Tz(slot);
}

uint8_t FingerprintControl::createModel()
{
    return _finger->createModel();
}

uint8_t FingerprintControl::storeModel(uint16_t id)
{
    return _finger->storeModel(id);
}

uint8_t FingerprintControl::loadModel(uint16_t id)
{
    return _finger->loadModel(id);
}

uint8_t FingerprintControl::getModel()
{
    return _finger->getModel();
}

uint8_t FingerprintControl::deleteModel(uint16_t id)
{
    return _finger->deleteModel(id);
}

uint8_t FingerprintControl::emptyDatabase()
{
    return _finger->emptyDatabase();
}

uint8_t FingerprintControl::fingerFastSearch()
{
    uint8_t result = _finger->fingerFastSearch();
    fingerID = _finger->fingerID;
    confidence = _finger->confidence;
    return result;
}

uint8_t FingerprintControl::readTemplate(uint8_t fingerprintId)
{
    Serial.println("------------------------------------");
    Serial.print("Attempting to load #");
    Serial.println(fingerprintId);
    uint8_t p = loadModel(fingerprintId);
    if (p != FINGERPRINT_OK)
    {
        Serial.print("Error loading template: ");
        Serial.println(p);
        return p;
    }

    p = getModel();
    if (p != FINGERPRINT_OK)
    {
        Serial.print("Error getting model: ");
        Serial.println(p);
        return p;
    }

    Serial.print("Template ");
    Serial.print(fingerprintId);
    Serial.println(" transferring:");

    memset(templateBuffer, 0xff, TEMPLATE_SIZE);
    uint32_t starttime = millis();
    int i = 0;
    while ((i < TEMPLATE_SIZE) && ((millis() - starttime) < 1000))
    {
        if (_serial->available())
        {
            templateBuffer[i++] = _serial->read();
        }
    }
    Serial.print(i);
    Serial.println(" bytes read.");

    return (i == TEMPLATE_SIZE) ? FINGERPRINT_OK : FINGERPRINT_PACKETRECIEVEERR;
}

uint8_t FingerprintControl::uploadTemplate(uint16_t id)
{
    uint8_t p = readTemplate(id);
    if (p != FINGERPRINT_OK)
    {
        return p;
    }

    // Convert template to base64
    String templateBase64 = base64::encode(templateBuffer, TEMPLATE_SIZE);

    // Create JSON payload
    String payload = "{\"id\":" + String(id) + ",\"template\":\"" + templateBase64 + "\"}";

    // Send HTTP POST request to server
    HTTPClient http;
    http.begin(SERVER_URL); // Replace with your server URL
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(payload);
    if (httpResponseCode > 0)
    {
        String response = http.getString();
        Serial.println(httpResponseCode);
        Serial.println(response);
    }
    else
    {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
    }

    http.end();
    return (httpResponseCode == 200) ? FINGERPRINT_OK : FINGERPRINT_PACKETRECIEVEERR;
}

uint8_t FingerprintControl::downloadFingerprintTemplate(uint16_t id)
{
    Serial.println("------------------------------------");
    Serial.print("Attempting to load #");
    Serial.println(id);
    uint8_t p = _finger->loadModel(id);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.print("Template ");
        Serial.print(id);
        Serial.println(" loaded");
        break;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
    default:
        Serial.print("Unknown error ");
        Serial.println(p);
        return p;
    }

    Serial.print("Attempting to get #");
    Serial.println(id);
    p = _finger->getModel();
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.print("Template ");
        Serial.print(id);
        Serial.println(" transferring:");
        break;
    default:
        Serial.print("Unknown error ");
        Serial.println(p);
        return p;
    }

    uint8_t bytesReceived[534]; // 2 data packets
    memset(bytesReceived, 0xff, 534);

    uint32_t starttime = millis();
    int i = 0;
    while (i < 534 && (millis() - starttime) < 20000)
    {
        if (_serial->available())
        {
            bytesReceived[i++] = _serial->read();
        }
    }
    Serial.print(i);
    Serial.println(" bytes read.");
    Serial.println("Decoding packet...");

    uint8_t fingerTemplate[512]; // the real template
    memset(fingerTemplate, 0xff, 512);

    // filtering only the data packets
    int uindx = 9, index = 0;
    memcpy(fingerTemplate + index, bytesReceived + uindx, 256); // first 256 bytes
    uindx += 256;                                               // skip data
    uindx += 2;                                                 // skip checksum
    uindx += 9;                                                 // skip next header
    index += 256;                                               // advance pointer
    memcpy(fingerTemplate + index, bytesReceived + uindx, 256); // second 256 bytes

    for (int i = 0; i < 512; ++i)
    {
        printHex(fingerTemplate[i], 2);
    }
    Serial.println("\ndone.");

    return p;
}

void FingerprintControl::printHex(int num, int precision)
{
    char tmp[16];
    char format[128];
    sprintf(format, "%%.%dX", precision);
    sprintf(tmp, format, num);
    Serial.print(tmp);
}
