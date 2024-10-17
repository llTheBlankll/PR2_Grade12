#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "fingerprint_control.h"

#define FINGERPRINT_RX 17
#define FINGERPRINT_TX 16
#define WIFI_SSID "PLDT WIFI"
#define WIFI_PASSWORD "@PLDTWIFIkr39h"

HardwareSerial fingerprintSerial(1);
FingerprintControl fingerprintControl(&fingerprintSerial, FINGERPRINT_RX, FINGERPRINT_TX);

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        ; // wait for serial port to connect. Needed for native USB port only
    }
    Serial.println("Serial initialized");

    fingerprintControl.begin();
    Serial.println("Fingerprint Serial initialized");

    // Check if we can communicate with the sensor
    if (fingerprintControl.verifyPassword())
    {
        Serial.println("Found fingerprint sensor!");
    }
    else
    {
        Serial.println("Did not find fingerprint sensor :(");
        while (1)
        {
            delay(1);
        } // Halt if sensor not found
    }

    Serial.println("Fingerprint sensor initialized");

    // Get the fingerprint sensor parameters
    uint8_t p = fingerprintControl.getParameters();
    if (p != FINGERPRINT_OK)
    {
        Serial.println("Error getting parameters");
    }
    else
    {
        Serial.println("Fingerprint parameters retrieved successfully");
    }

    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void loop()
{
    Serial.println("Place finger on sensor...");

    uint8_t fingerprintStatus = fingerprintControl.getImage();

    if (fingerprintStatus == FINGERPRINT_OK)
    {
        Serial.println("Image taken");

        // Instead of validateFingerprint(), let's use our new method
        fingerprintStatus = fingerprintControl.downloadFingerprintTemplate(1); // Assuming we want to download template ID 1

        if (fingerprintStatus == FINGERPRINT_OK)
        {
            Serial.println("Fingerprint template downloaded and printed!");
            // Add any actions you want to perform when the fingerprint is downloaded
        }
        else
        {
            Serial.println("Error downloading fingerprint template");
        }
    }
    else if (fingerprintStatus == FINGERPRINT_NOFINGER)
    {
        // No finger detected, do nothing
    }
    else
    {
        Serial.println("Error capturing fingerprint image");
    }

    delay(1000); // Wait a second before trying again
}
