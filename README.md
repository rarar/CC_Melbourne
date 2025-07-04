# Capitalist Clock - Arduino Project

A WiFi-connected display that fetches posts from a web server and shows them on an LCD screen while triggering a solenoid mechanism.

## 🚀 Features

- Fetches posts from remote server via WiFi
- Displays text on 16x2 LCD with auto-scrolling
- Fires solenoid mechanism for each new post
- Falls back to pre-programmed tweets if server unavailable
- Auto-reconnects to WiFi if connection lost

## 🔌 Wiring

### LCD Connections
```
LCD Pin → Arduino Pin
RS      → 12
Enable  → 11
D4      → 5
D5      → 4
D6      → 3
D7      → 2
VSS     → Ground
VCC     → 5V
```

### Solenoid Driver Circuit
```
Arduino Pin 7 → 1kΩ Resistor → TIP120 Base
12V+          → Solenoid      → TIP120 Collector
TIP120 Emitter → Ground
Flyback Diode: Cathode to 12V+, Anode to Solenoid Ground
```

## 💻 Setup

### Files
- `CC_Melbourne.ino` - Main sketch
- `config.h` - WiFi and server settings

### Configuration
Edit `config.h` with your WiFi details:
```cpp
const char* ssid = "Your_WiFi_Name";
const char* password = "Your_WiFi_Password";
```

### Required Libraries
- WiFiS3
- ArduinoHttpClient
- ArduinoJson
- LiquidCrystal

## 📡 Server API

Server should return JSON:
```json
[
  {
    "text": "Your post content here",
    "author": "username"
  }
]
```

## 🛠️ Troubleshooting

**WiFi Issues**: Check credentials in `config.h`
**Solenoid Not Firing**: Verify 12V power and TIP120 wiring
**LCD Problems**: Check wiring and contrast adjustment
**Server Issues**: Monitor Serial output for HTTP status codes

Open Serial Monitor (115200 baud) to see status messages and debug info.
