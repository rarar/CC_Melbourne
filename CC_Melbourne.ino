#include <WiFiS3.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal.h>
#include "config.h"  // Include our configuration file

// LCD pin setup
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Solenoid pin
const int sPin = 7;

// Timing variables
unsigned long previousMillis = 0;
unsigned long lastDisplayTime = 0;
unsigned long scrollStartTime = 0;
int scrollPosition = 0;
bool isScrolling = false;

// Array to store posts
#define MAX_POSTS 10
struct Post {
  String text;
  String author;
};

Post posts[MAX_POSTS];
int totalPosts = 0;
int currentPostIndex = 0;

// Fallback tweets (from your original code)
String fallbackTweets[] = {
  "social media is toxic ...it just starts feeling more like a waste of time.",
  "I dont get time off, not now, not while price is this good.",
  "Embrace meaningful action on loss & damage. We have no time to lose.",
  "I stopped experiencing burnout as soon   as I made free",
  "  SAVE TIME WHEN YOU COOK KALE BY USING BUTTER",
  "Iran tired regime lives on borrowed time  ",
  "Time well spent is time well spent",
  "The Advocate that made sure President Zuma go to Jail ..... Its payback time ... President Ramaphosa appoints him to ... JSC",
  "Time is Money. Knowledge is Power",
  "This is why its important to invest time learning yourself and your partner"
};
int fallbackCount = sizeof(fallbackTweets) / sizeof(fallbackTweets[0]);
int fallbackIndex = 0;
bool usingFallback = false;

// Use WiFiSSLClient for HTTPS
WiFiSSLClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(3000);
  
  // Initialize LCD
  lcd.begin(16, 2);
  lcd.clear();
  
  // Initialize solenoid pin
  pinMode(sPin, OUTPUT);
  digitalWrite(sPin, LOW);  // Ensure it starts LOW
  
  // Test fire on startup to make sure it works
  Serial.println("Testing solenoid on startup...");
  digitalWrite(sPin, LOW);
  delay(500);
  digitalWrite(sPin, HIGH);
  delay(500);
  digitalWrite(sPin, LOW);
  Serial.println("Startup solenoid test complete");
  
  Serial.println("\n\n");
  Serial.println("Starting Capitalist Clock");
  Serial.println("------------------------");
  
  // Show startup message on LCD
  lcd.print("Capitalist Clock");
  lcd.setCursor(0, 1);
  lcd.print("Starting up...");
  
  // Connect to WiFi
  connectToWifi();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Debug: Show we're in the loop every 10 seconds
  static unsigned long lastLoopDebug = 0;
  if (currentMillis - lastLoopDebug >= 10000) {
    lastLoopDebug = currentMillis;
    Serial.println("Loop running... millis: " + String(currentMillis));
    Serial.println("Time since last display: " + String(currentMillis - lastDisplayTime));
    Serial.println("Display interval: " + String(displayInterval));
  }
  
  // Check if it's time to fetch new posts
  if (currentMillis - previousMillis >= fetchInterval) {
    previousMillis = currentMillis;
    fetchPosts();
  }
  
  // Handle display updates
  if (currentMillis - lastDisplayTime >= displayInterval) {
    Serial.println("Display timer triggered - calling displayNextPost()");
    lastDisplayTime = currentMillis;
    displayNextPost();
    
    // Add random delay like in your original code
    int randomDelay = random(1000, 3000);
    Serial.println("Random delay: " + String(randomDelay) + "ms");
    delay(randomDelay);
  }
  
  // Check WiFi connection periodically
  if ((currentMillis % 30000) < 50) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected. Reconnecting...");
      connectToWifi();
    }
  }
  
  delay(100);
}

void connectToWifi() {
  Serial.print("Connecting to WiFi SSID: ");
  Serial.println(ssid);
  
  lcd.clear();
  lcd.print("Connecting WiFi");
  lcd.setCursor(0, 1);
  lcd.print(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  int maxAttempts = 20;
  
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(500);
    Serial.print(".");
    attempts++;
    
    // Update LCD with dots
    if (attempts % 4 == 0) {
      lcd.setCursor(0, 1);
      lcd.print("Connecting");
      for (int i = 0; i < (attempts / 4) % 4; i++) {
        lcd.print(".");
      }
      lcd.print("    ");
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    lcd.clear();
    lcd.print("WiFi Connected!");
    lcd.setCursor(0, 1);
    lcd.print("Fetching posts..");
    
    delay(2000);
    fetchPosts();
  } else {
    Serial.println("");
    Serial.println("WiFi connection failed");
    
    lcd.clear();
    lcd.print("WiFi Failed");
    lcd.setCursor(0, 1);
    lcd.print("Using fallback");
    
    // Use fallback tweets if WiFi fails
    usingFallback = true;
    delay(2000);
  }
}

void fetchPosts() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Using fallback tweets.");
    usingFallback = true;
    return;
  }

  Serial.println("Making HTTP request to: " + String(serverAddress) + endpoint);
  client.get(endpoint);
  
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  
  Serial.print("Status code: ");
  Serial.println(statusCode);
  
  if (statusCode == 200) {
    Serial.println("Response received, length: " + String(response.length()));
    
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      usingFallback = true;
      return;
    }
    
    int newTotalPosts = min((int)doc.size(), MAX_POSTS);
    
    if (newTotalPosts == 0) {
      Serial.println("No posts found in response");
      usingFallback = true;
      return;
    }
    
    Serial.print("Received posts: ");
    Serial.println(newTotalPosts);
    
    totalPosts = newTotalPosts;
    currentPostIndex = 0;
    usingFallback = false;
    
    // Store posts in array
    for (int i = 0; i < totalPosts; i++) {
      posts[i].text = doc[i]["text"].as<String>();
      posts[i].author = doc[i]["author"].as<String>();
      
      Serial.print("Post ");
      Serial.print(i);
      Serial.print(": @");
      Serial.print(posts[i].author);
      Serial.print(" - ");
      Serial.println(posts[i].text);
    }
    
    lcd.clear();
    lcd.print("Posts loaded!");
    lcd.setCursor(0, 1);
    lcd.print("Count: " + String(totalPosts));
    delay(1000);
    
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(statusCode);
    usingFallback = true;
  }
}

void displayNextPost() {
  String textToDisplay;
  String authorToDisplay = "";
  
  if (usingFallback || totalPosts == 0) {
    // Use fallback tweets
    textToDisplay = fallbackTweets[fallbackIndex];
    fallbackIndex = (fallbackIndex + 1) % fallbackCount;
    Serial.println("Displaying fallback: " + textToDisplay);
  } else {
    // Use fetched posts
    Post& post = posts[currentPostIndex];
    textToDisplay = post.text;
    authorToDisplay = "@" + post.author;
    currentPostIndex = (currentPostIndex + 1) % totalPosts;
    
    Serial.print("Displaying post ");
    Serial.print(currentPostIndex);
    Serial.print(" of ");
    Serial.print(totalPosts);
    Serial.print(": ");
    Serial.println(textToDisplay);
  }
  
  // Display on LCD first - back to your original working pattern
  lcd.clear();
  lcd.home();
  lcd.leftToRight();
  
  // Fire solenoid at the beginning like your original code
  Serial.println("Firing solenoid for tweet: " + textToDisplay.substring(0, 20) + "...");
  fireSolenoid();
  
  // Display the text
  lcd.print(textToDisplay);
  
  // Handle scrolling for long text (like your original)
  if (textToDisplay.length() > 16) {
    delay(250);
    Serial.println("Scrolling text: " + textToDisplay);
    
    // Scroll the text like in your original code
    for (int positionCounter = 0; positionCounter < textToDisplay.length(); positionCounter++) {
      lcd.scrollDisplayLeft();
      delay(250);
    }
  }
  
  isScrolling = false; // We handle scrolling immediately, not in the loop
}

void fireSolenoid() {
  // Exact copy of your original working pattern
  digitalWrite(sPin, LOW);
  delay(500);
  digitalWrite(sPin, HIGH);
  delay(500);
  digitalWrite(sPin, LOW);
  
  Serial.println("SOLENOID: Fired (original timing)");
}