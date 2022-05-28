#define SEALEVELPRESSURE_HPA 1013.00
#define TIME_TO_SLEEP  10 * 60

#define MQTT_HOST IPAddress(192, 168, 178, 20)
#define MQTT_PORT 1883

// Hooked up using voltage divider
// In this case using 10K and 22K resistors I had laying around
int batteryLevelPin = A0;
