#define SEALEVELPRESSURE_HPA 1013.00

// Time to sleep after taking a measurement
#define TIME_TO_SLEEP 10 * 60

// Go back to sleep if no measurement was taken withing this time
#define DEADMAN_SWITCH_MS 30 * 1000

#define MQTT_HOST IPAddress(192, 168, 178, 20)
#define MQTT_PORT 1883

// Hooked up using voltage divider
// In this case using 10K and 22K resistors I had laying around
int batteryLevelPin = A0;
