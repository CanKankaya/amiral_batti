#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <ArduinoWebsockets.h>

// Define custom I2C pins
#define I2C_SCL 22
#define I2C_SDA 21

#define BUTTON_PIN_LEFT 23
#define BUTTON_PIN_RIGHT 19
#define BUTTON_PIN_UP 18
#define BUTTON_PIN_DOWN 5
#define BUTTON_PIN_HIT 15

using namespace websockets;

const char* ssid = "Can Kankaya";
const char* password = "wifisifre";

WebsocketsServer server;
WebsocketsClient client;

// Initialize the U8g2 library for the SSD1106 display with custom I2C pins
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ I2C_SCL, /* data=*/ I2C_SDA);

int counter = 0;

struct Boat {
  int x;
  int y;
  int length;
  bool horizontal;
  bool hit[4]; // Maximum length is 4
};

Boat boats[4];

int highlightX = 0;
int highlightY = 0;

bool opponentTurn = true;

void setup() {
  Serial.begin(115200);
  u8g2.begin();
  u8g2.clearBuffer();          // Clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr); // Choose a suitable font
  u8g2.drawStr(0, 10, "Hello, world!");  // Write something to the internal memory
  u8g2.sendBuffer();           // Transfer internal memory to the display
  delay(1000);                 // Wait for 1 second

  // Initialize boats
  boats[0] = {2, 2, 1, true, {false}};
  boats[1] = {4, 4, 2, false, {false, false}};
  boats[2] = {6, 6, 3, true, {false, false, false}};
  boats[3] = {7, 1, 4, false, {false, false, false, false}};

  pinMode(BUTTON_PIN_LEFT, INPUT_PULLUP);  // Initialize the left button pin as input with pull-up resistor
  pinMode(BUTTON_PIN_RIGHT, INPUT_PULLUP); // Initialize the right button pin as input with pull-up resistor
  pinMode(BUTTON_PIN_UP, INPUT_PULLUP); // Initialize the up button pin as input with pull-up resistor
  pinMode(BUTTON_PIN_DOWN, INPUT_PULLUP); // Initialize the down button pin as input with pull-up resistor
  pinMode(BUTTON_PIN_HIT, INPUT_PULLUP); // Initialize the hit button pin as input with pull-up resistor

  // Connect to WiFi
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // Print the IP address

  // Start WebSocket server
  server.listen(80);
  Serial.println("WebSocket server started");
}

void drawBattleMap() {
  u8g2.clearBuffer();          // Clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr); // Choose a suitable font

  // Draw the battle map grid
  int gridSize = 16;           // 16 cells horizontally and vertically
  int cellSize = 8;            // Each cell is 8x8 pixels
  for (int i = 0; i <= gridSize; i++) {
    u8g2.drawLine(i * cellSize, 0, i * cellSize, gridSize * cellSize); // Vertical lines
    u8g2.drawLine(0, i * cellSize, gridSize * cellSize, i * cellSize); // Horizontal lines
  }
}

void drawBoat(Boat boat) {
  int cellSize = 8; // Each cell is 8x8 pixels
  for (int i = 0; i < boat.length; i++) {
    int x = boat.x * cellSize;
    int y = boat.y * cellSize;
    if (boat.horizontal) {
      x += i * cellSize;
    } else {
      y += i * cellSize;
    }
    if (boat.hit[i]) {
      // Draw demolished boat part
      u8g2.drawBox(x, y, cellSize, cellSize);
      u8g2.setDrawColor(0); // Set color to black
      u8g2.drawLine(x, y, x + cellSize, y + cellSize);
      u8g2.drawLine(x + cellSize, y, x, y + cellSize);
      u8g2.setDrawColor(1); // Set color back to white
    } else {
      // Draw intact boat part
      u8g2.drawBox(x, y, cellSize, cellSize);
    }
  }
  u8g2.sendBuffer();           // Transfer internal memory to the display

}

void handleOpponentHit(int x, int y) {
  Serial.print("Opponent hit: ");
  Serial.print(x);
  Serial.print(", ");
  Serial.println(y);
  for (int i = 0; i < 4; i++) {
    Boat &boat = boats[i];
    for (int j = 0; j < boat.length; j++) {
      int bx = boat.x + (boat.horizontal ? j : 0);
      int by = boat.y + (boat.horizontal ? 0 : j);
      if (bx == x && by == y) {
        boat.hit[j] = true;
        drawBoat(boat);        // Redraw the boat to show the hit
        return;
      }
    }
  }
  u8g2.sendBuffer();           // Transfer internal memory to the display
}

void handleUserHit() {
  // Simulate sending data with a delay
  u8g2.clearBuffer();          // Clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr); // Choose a suitable font
  u8g2.drawStr(32, 32, "Waiting..."); // Display waiting message
  u8g2.sendBuffer();           // Transfer internal memory to the display
  delay(200);                 // Wait for 0.2 seconds
  
  // Send hit coordinates to the client
  String message = String(highlightX) + ", " + String(highlightY);
  client.send(message);

  // Wait for opponent's response
  unsigned long startTime = millis();
  bool responseReceived = false;
  String response = "Missed...";
  while (millis() - startTime < 1000) { // Wait for up to 5 seconds
    if (client.available()) {
      auto msg = client.readBlocking();
      response = msg.data();
      responseReceived = true;
      break;
    }
    delay(100); // Small delay to avoid busy-waiting
  }

  // Display hit or miss message
  u8g2.clearBuffer();          // Clear the internal memory
  if (responseReceived) {
    u8g2.drawStr(32, 32, response.c_str());
  } else {
    u8g2.drawStr(32, 32, "Missed...");
  }
  u8g2.sendBuffer();           // Transfer internal memory to the display
  delay(2000);                 // Wait for 2 seconds
}

void resetMap() {
  drawBattleMap();             // Draw the battle map
  u8g2.sendBuffer();           // Transfer internal memory to the display
}

void highlightSquare(int x, int y, bool on) {
  int cellSize = 8; // Each cell is 8x8 pixels
  if (on) {
    u8g2.drawBox(x * cellSize, y * cellSize, cellSize, cellSize);
  } else {
    // Redraw the grid cell to clear the highlight
    u8g2.setDrawColor(0); // Set color to black to clear the square
    u8g2.drawBox(x * cellSize, y * cellSize, cellSize, cellSize);
    u8g2.setDrawColor(1); // Set color back to white
    u8g2.drawLine(x * cellSize, y * cellSize, x * cellSize, (y + 1) * cellSize); // Vertical line
    u8g2.drawLine(x * cellSize, y * cellSize, (x + 1) * cellSize, y * cellSize); // Horizontal line
  }
  u8g2.sendBuffer();           // Transfer internal memory to the display
}

void moveHighlightUp() {
  highlightSquare(highlightX, highlightY, false);
  highlightY = max(0, highlightY - 1);
  highlightSquare(highlightX, highlightY, true);
}

void moveHighlightDown() {
  highlightSquare(highlightX, highlightY, false);
  highlightY = min(15, highlightY + 1);
  highlightSquare(highlightX, highlightY, true);
}

void moveHighlightLeft() {
  highlightSquare(highlightX, highlightY, false);
  highlightX = max(0, highlightX - 1);
  highlightSquare(highlightX, highlightY, true);
}

void moveHighlightRight() {
  highlightSquare(highlightX, highlightY, false);
  highlightX = min(15, highlightX + 1);
  highlightSquare(highlightX, highlightY, true);
}

void loop() {
    Serial.println(WiFi.localIP()); // Print the IP address
  if (opponentTurn) {
    drawBattleMap();           // Draw the battle map
    
    for (int i = 0; i < 4; i++) {
      drawBoat(boats[i]);        // Draw boats dynamically
    }
    u8g2.sendBuffer();           // Transfer internal memory to the display

    
    if (!client.available()) {
      client = server.accept();
    }
    if (client.available()) {
      while (client.available()) {
        auto msg = client.readBlocking();

        // Log received message
        Serial.print("Got Message: ");
        Serial.println(msg.data());

        // Parse the message as coordinates
        int commaIndex = msg.data().indexOf(',');
        int x = msg.data().substring(0, commaIndex).toInt();
        int y = msg.data().substring(commaIndex + 1).toInt();

        // Handle opponent hit if coordinates are not (0, 0)
        if (x != 0 || y != 0) {
          handleOpponentHit(x, y);
          delay(2000);           // Wait for 2 seconds
          opponentTurn = false; // Switch to player's turn
          break; // Exit the loop after registering a hit
        }
      }
    }
  } else {
    resetMap();                  // Reset map for our turn
    delay(500);                  // Wait for 0.5 second

    // Highlight the top-left square
    highlightSquare(highlightX, highlightY, true);

    // Wait for button presses to move the highlight
    while (true) {
      if (digitalRead(BUTTON_PIN_LEFT) == LOW) {
        moveHighlightLeft();
        delay(500); // Debounce delay
      }  
      if (digitalRead(BUTTON_PIN_RIGHT) == LOW) {
        moveHighlightRight();
        delay(500); // Debounce delay
      }
      if (digitalRead(BUTTON_PIN_UP) == LOW) {
        moveHighlightUp();
        delay(500); // Debounce delay
      }
      if (digitalRead(BUTTON_PIN_DOWN) == LOW) {
        moveHighlightDown();
        delay(500); // Debounce delay
      }
      if (digitalRead(BUTTON_PIN_HIT) == LOW) {
        Serial.println("Hit button pressed");
        handleUserHit(); // Simulate a hit
        delay(500); // Debounce delay
        break; // Exit the loop after registering a hit
      }
    }
    opponentTurn = true;         // Switch to opponent's turn
  }
}