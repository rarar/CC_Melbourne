#ifndef CONFIG_H
#define CONFIG_H

// WiFi credentials
const char* ssid = "Shtinky";
const char* password = "F1b0nacc1";

// Server details
const char* serverAddress = "capitalist-clock-web-server.onrender.com";
int port = 443;
String endpoint = "/api/arduino/posts";

// Timing configuration
const long fetchInterval = 60000;  // Fetch new posts every 60 seconds
const long displayInterval = 5000;  // Display each post for 5 seconds

#endif