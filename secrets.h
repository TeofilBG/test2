#ifndef SECRETS_H
#define SECRETS_H

// WiFi Credentials
#define WIFI_SSID "OrthoLab2"
#define WIFI_PASSWORD "lajlica112233"

// MQTT Broker Configuration
// (Add these when you provide your MQTT settings)
#define MQTT_BROKER_IP "192.168.11.112"  // Replace with your MQTT broker IP
#define MQTT_BROKER_PORT 1883            // Replace with your MQTT broker port
#define MQTT_USERNAME "MQTT-USER"   // Replace with your MQTT username (if required)
#define MQTT_PASSWORD "112233"   // Replace with your MQTT password (if required)

// Device Configuration
#define DEVICE_HOSTNAME "SmartController"  // Replace with your preferred hostname

// Home Assistant Configuration (if used)
#define HA_BASE_URL "http://192.168.11.112:8123"  // Replace with your Home Assistant URL
#define HA_ACCESS_TOKEN "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiI4Nzc1ZDMwZTcwYzE0MTg2YTVjNmJmNWFmNmRkYzYxYiIsImlhdCI6MTc1MzQ2NDc3MywiZXhwIjoyMDY4ODI0NzczfQ.LIdg6LxAzqFurDBdjQXAECnfc4jK0g7eIe9vOwbbRv8";    // Replace with your HA long-lived access token

// NTP Configuration
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 3600      // UTC+1 (Belgrade timezone)
#define DAYLIGHT_OFFSET_SEC 3600 // Daylight saving time

#endif