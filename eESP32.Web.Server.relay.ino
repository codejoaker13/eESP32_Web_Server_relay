/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

#include <WiFi.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD_FROM_SSID";

WiFiServer server(80);

String header;

// Using an array to store the states of multiple polo pins
String outputStates[] = {"pano", "off", "off", "off", "off", "off", "off", "off"}; // Initial states for polo 26, 27, 25, 33, 32, 14, 12, 2

int outputPins[] = {26, 27, 25, 33, 32, 14, 12, 2}; // rolo pins 26, 27, 25, 33, 32, 14, 12, 2

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < sizeof(outputPins) / sizeof(outputPins[0]); i++) {
    pinMode(outputPins[i], OUTPUT);
    digitalWrite(outputPins[i], LOW);
  }

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");

    String currentLine = "";
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Loop through all defined pins
            for (int i = 0; i < sizeof(outputPins) / sizeof(outputPins[0]); i++) {
              if (header.indexOf("GET /" + String(outputPins[i]) + "/on") >= 0) {
                Serial.println("rolo " + String(outputPins[i]) + " on");
                outputStates[i] = "on";
                digitalWrite(outputPins[i], HIGH);
              } else if (header.indexOf("GET /" + String(outputPins[i]) + "/off") >= 0) {
                Serial.println("rolo " + String(outputPins[i]) + " off");
                outputStates[i] = "off";
                digitalWrite(outputPins[i], LOW);
              }

              // Display the state and buttons for each GPIO pin
              client.println("<p>rolo " + String(outputPins[i]) + " - spiti " + outputStates[i] + "</p>");
              if (outputStates[i] == "off") {
                client.println("<p><a href=\"/" + String(outputPins[i]) + "/on\"><button class=\"button\">ON</button></a></p>");
              } else {
                client.println("<p><a href=\"/" + String(outputPins[i]) + "/off\"><button class=\"button button2\">OFF</button></a></p>");
              }
            }

            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #00FF00; border: none; color: white; padding: 10px 30px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #FF0000;}</style></head>");

            client.println("<body><h1>KNX PARTNER</h1>");

            // ... rest of the HTML code

            client.println("</body></html>");

            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
