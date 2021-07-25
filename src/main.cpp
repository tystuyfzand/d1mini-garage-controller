#include <EspMQTTClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#define DEBUG true

// Our configuration document
DynamicJsonDocument configDoc(512);

// Client
EspMQTTClient client;

struct Door {
  String topic;
  String name;
  uint8_t outputPin;
  uint8_t inputPin;
  int lastState;
  int currentState;

  Door(String topic, String name, uint8_t outputPin, uint8_t inputPin);
};

Door::Door(String topic, String name, uint8_t outputPin, uint8_t inputPin) {
  this->topic = topic;
  this->name = name;
  this->outputPin = outputPin;
  this->inputPin = inputPin;
  this->lastState = this->currentState = 0;
}

int count = 0;

String states[] = {
  "CLOSED",
  "OPEN",
};

Door *doors[4];

void setup() {
  Serial.begin(115200);
  Serial.println();

  Serial.println("Initializing fs...");

  if (LittleFS.begin()) {
    Serial.println("done.");
  } else {
    Serial.println("fail.");
    exit(1);
  }

  File configFile = LittleFS.open("/config.json", "r");

  if (!configFile) {
    Serial.println("Unable to open configuration file");
    exit(1);
  }

  if (DEBUG) {
    client.enableDebuggingMessages();
  }

  deserializeJson(configDoc, configFile);

  configFile.close();

  client.setWifiCredentials(configDoc["ssid"], configDoc["passphrase"]);
  client.setMqttServer(configDoc["mqttHost"], "", "", 1883);

  client.setMqttClientName(configDoc["name"]);

  JsonObject webUpdaterConfig = configDoc["webUpdater"].as<JsonObject>();

  if (webUpdaterConfig && webUpdaterConfig["enabled"].as<bool>()) {
    client.enableHTTPWebUpdater(webUpdaterConfig["username"], webUpdaterConfig["password"]);
  }

  JsonObject doorConfig = configDoc["doors"].as<JsonObject>();

  for (JsonPair p : doorConfig) {
    JsonObject value = p.value();
    const char* key = p.key().c_str();
    const char* name = value["name"];
    uint8_t outputPin = value["outputPin"];
    uint8_t inputPin = value["inputPin"];

    Serial.println("Deserialized door " + String(key));

    doors[count] = new Door(String(key), String(name), outputPin, inputPin);
    count++;
  }


  for (int i = 0; i < count; i++) {
    Door* door = doors[i];
    pinMode(door->inputPin, INPUT_PULLUP);
    pinMode(door->outputPin, OUTPUT);
    digitalWrite(door->outputPin, HIGH);
  }
}

void togglePin(const uint8_t pin) {
  digitalWrite(pin, LOW);
  delay(500);
  digitalWrite(pin, HIGH);
}

void publishConfig(Door* door) {
  DynamicJsonDocument discoveryDoc(256);
  
  discoveryDoc["device_class"] = "garage";
  discoveryDoc["name"] = door->name;
  discoveryDoc["command_topic"] = "homeassistant/cover/" + door->topic + "/set";
  discoveryDoc["position_topic"] = "homeassistant/cover/" + door->topic + "/position";

  String config;
  serializeJson(discoveryDoc, config);

  if (DEBUG) {
    Serial.println("Device " + door->topic + " config:");
    Serial.println(config);
  }

  client.publish("homeassistant/cover/" + door->topic + "/config", config);

  client.subscribe("homeassistant/cover/" + door->topic + "/set", [door] (const String &payload)  {
    Serial.println(payload);
    if ((payload == "OPEN" && door->lastState == LOW) || (payload == "CLOSE" && door->lastState == HIGH)) {
      togglePin(door->outputPin);
    }
  });
}

/*
 * Once connected (both wifi + mqtt), we send HomeAssistant some JSON information about our sensor.
 * This allows the sensor to be used dynamically and not defined in the HA config files.
 * 
 */
void onConnectionEstablished() {
  Serial.println("Connected");

  for (int i = 0; i < count; i++) {
    publishConfig(doors[i]);
  }
}

unsigned long lastPoll;

void loop() {
  client.loop();

  if (!client.isWifiConnected() || !client.isConnected()) {
    return;
  }

  if (millis() - lastPoll < 1000UL) {
    return;
  }

  lastPoll = millis();
  
  for (int i = 0; i < count; i++) {
    Door* door = doors[i];

    if (!door) {
      continue;
    }

    door->currentState = digitalRead(door->inputPin);

    if (door->lastState != door->currentState) {
      Serial.println("Door " + door->name + " state changed to " + states[door->currentState]);
      client.publish("homeassistant/cover/" + door->topic + "/state", states[door->currentState]);
      door->lastState = door->currentState;
    }
  }
}