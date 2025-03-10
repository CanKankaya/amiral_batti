### Contact

<a href="mailto:can.kankaya@outlook.com">
  <img src="https://img.shields.io/badge/Microsoft_Outlook-0078D4?style=for-the-badge&logo=microsoft-outlook&logoColor=white" alt="Outlook" />
	
</a>
<a href="https://www.linkedin.com/in/can-kankaya-738518158/">
  <img src="https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white" alt="LinkedIn" />
</a>

<a href="https://t.me/cankankaya">
  <img src="https://img.shields.io/badge/Telegram-26A5E4?style=for-the-badge&logo=telegram&logoColor=white" alt="Telegram" />
</a>


# Amiral Batti (Battleship) Game

This project implements a Battleship game using ESP32 and an OLED display. The game consists of a client and a server, where the client sends hit coordinates to the server, and the server responds with hit or miss messages.

## Project Structure

- `amiral_batti_client.ino`: The client code that sends hit coordinates to the server.
- `amiral_batti_server.ino`: The server code that receives hit coordinates from the client and responds with hit or miss messages.
- `amiral_batti.ino`: The main game logic for the Battleship game.

## Hardware Requirements

- ESP32 board
- OLED display (SSD1306)
- Push buttons

## Wiring

### OLED Display

| OLED Pin | ESP32 Pin |
|----------|-----------|
| VCC      | 3.3V      |
| GND      | GND       |
| SCL      | GPIO 22   |
| SDA      | GPIO 21   |

### Push Buttons

| Button       | ESP32 Pin |
|--------------|-----------|
| Left Button  | GPIO 23   |
| Right Button | GPIO 19   |
| Up Button    | GPIO 18   |
| Down Button  | GPIO 5    |
| Hit Button   | GPIO 15   |

## Libraries

The following libraries are required for this project:

- `U8g2`: For controlling the OLED display.
- `WiFi`: For connecting to WiFi.
- `ArduinoWebsockets`: For WebSocket communication between the client and server.

You can install these libraries via the Arduino Library Manager.

## Setup

1. Connect the OLED display and push buttons to the ESP32 as described in the wiring section.
2. Open the `amiral_batti_client.ino` file in the Arduino IDE and upload it to the ESP32 board.
3. Open the `amiral_batti_server.ino` file in the Arduino IDE and upload it to another ESP32 board.
4. Open the `amiral_batti.ino` file in the Arduino IDE and upload it to the ESP32 board.

## Usage

1. Power on both the client and server ESP32 boards.
2. The server will connect to the specified WiFi network and start a WebSocket server.
3. The client will connect to the server and send hit coordinates.
4. Use the push buttons on the client to move the highlight and press the hit button to send hit coordinates to the server.
5. The server will respond with hit or miss messages, which will be displayed on the client's OLED screen.





