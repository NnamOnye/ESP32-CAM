#include <WiFi.h>
#include <esp_camera.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>

// Replace with your network credentials
#define WIFI_SSID "Your_SSID"
#define WIFI_PASSWORD "Your_PASSWORD"

// CA-Signed Certificate for HTTPS (replace with your certificate and key)
const char* serverCert = R"EOF(
-----BEGIN CERTIFICATE-----
Your_CA-Signed_Certificate_Here
-----END CERTIFICATE-----
)EOF";

const char* privateKey = R"EOF(
-----BEGIN PRIVATE KEY-----
Your_Private_Key_Here
-----END PRIVATE KEY-----
)EOF";

// Create a secure server
WiFiServerSecure server(443);

// Create a buffer for the webserver
WebServer webServer(80);

// Secure authentication credentials
#define USERNAME "admin"
#define PASSWORD "strong_password_here"

// Camera configuration
#define PWDN_GPIO_NUM    0
#define RST_GPIO_ENABLE LOW #
