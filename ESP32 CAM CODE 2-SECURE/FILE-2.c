#include <WiFi.h>
#include <esp_camera.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>

// Replace with your network credentials
const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

// CA-Signed Certificate for HTTPS (replace with your actual certificate and key)
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

WiFiServerSecure secureServer(443); // HTTPS server
WebServer server(80);               // Fallback HTTP server

// Secure credentials for basic authentication
const char* loginUsername = "admin";
const char* loginPassword = "secure_password_here";

// Camera configuration
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Function declarations
void handleRoot();
void handleCapture();

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi. IP address: ");
  Serial.println(WiFi.localIP());

  // Configure camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera initialization failed!");
    return;
  }

  // Initialize the HTTPS server
  secureServer.setServerCertificate(serverCert);
  secureServer.setPrivateKey(privateKey);
  secureServer.begin();
  Serial.println("HTTPS server started.");

  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/capture", handleCapture);
  server.begin();
  Serial.println("HTTP server started.");
}

void loop() {
  server.handleClient();
}

// Handle root URL
void handleRoot() {
  if (!server.authenticate(loginUsername, loginPassword)) {
    return server.requestAuthentication();
  }

  String html = "<html><body><h1>ESP32 Secure Camera</h1>";
  html += "<img src=\"/capture\" width=\"100%\"><br>";
  html += "<a href=\"/capture\">Refresh</a>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// Handle capture URL
void handleCapture() {
  if (!server.authenticate(loginUsername, loginPassword)) {
    return server.requestAuthentication();
  }

  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }

  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(200, "image/jpeg", (const char*)fb->buf, fb->len);

  esp_camera_fb_return(fb);
}
