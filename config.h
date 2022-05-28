#define SEALEVELPRESSURE_HPA 1013.00
#define TIME_TO_SLEEP  10 * 60

IPAddress broker(192, 168, 178, 20); // IP address of your MQTT broker eg. 192.168.1.50

// Hooked up using voltage divider
// In this case using 10K and 22K resistors I had laying around
int batteryLevelPin = A0;
