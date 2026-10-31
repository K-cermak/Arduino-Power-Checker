/*
    Power Checker by Karel Cermak | Karlosoft.com
    Licensed under the MIT License
*/


#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>

// OVERALL
int state = 1;

// ETHERNET
byte mac[] = {0xA6, 0x2A, 0xFA, 0xC7, 0x70, 0xC0}; // replace with your own random MAC address
IPAddress ip(10, 0, 0, 60); // set to a free IP address on your network
EthernetServer server(80);


// DHT 22
#define DHTPIN 28
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

float hum;
float temp;

// CURRENT INPUT
const int checkMain = A8;
const int checkBackup = A9;
bool mainPower;
bool backupPower;

// LEDS
const byte ledBlue = 29;
const byte ledOrange = 30;
const byte ledRed = 31;

unsigned long previousMillisBlue = 0;
unsigned long previousMillisOrange = 0;
unsigned long previousMillisRed = 0;

boolean ledStateBlue = true;
boolean ledStateOrange = true;
boolean ledStateRed = true;

// TIMES
unsigned long longOn = 400;
unsigned long longOff = 2000;
unsigned long shortOn = 75;
unsigned long shortOff = 200;

// ALARMS
const byte redLight = 32;
const byte buzzer = 33;
boolean buzzerState = true;
unsigned long previousMillisBuzzer = 0;

// STOP BUTTON
const byte stopButton = 34;
byte pauseType = 0; // 0 - no pause, 1 - pause alarm, 2 - pause light and alarm
unsigned long pauseStart = 0;
unsigned long pauseDuration = 0;


void setup() {
    pinMode(checkMain, INPUT);
    pinMode(checkBackup, INPUT);

    pinMode(ledBlue, OUTPUT);
    pinMode(ledOrange, OUTPUT);
    pinMode(ledRed, OUTPUT);
    pinMode(redLight, OUTPUT);
    pinMode(buzzer, OUTPUT);

    pinMode(stopButton, INPUT_PULLUP);

    //dht
    dht.begin();

    //ethernet
    Ethernet.begin(mac, ip);
    server.begin();


    // blink leds and buzz
    digitalWrite(ledBlue, HIGH);
    digitalWrite(ledOrange, HIGH);
    digitalWrite(ledRed, HIGH);
    delay(300);

    for (int i = 0; i < 10; i++) {
        digitalWrite(buzzer, HIGH);
        digitalWrite(ledBlue, HIGH);
        digitalWrite(ledOrange, HIGH);
        digitalWrite(ledRed, HIGH);
        delay(150);
        digitalWrite(ledBlue, LOW);
        digitalWrite(ledOrange, LOW);
        digitalWrite(ledRed, LOW);
        digitalWrite(buzzer, LOW);
        delay(75);
    }
}


void loop() {
    dataProcess();
    tempProcess();
    internetWorks();
    serverProcess();
    blinkLeds();
    checkStopButton();
}


void dataProcess() {
    mainPower = digitalRead(checkMain);
    backupPower = digitalRead(checkBackup);

    if (backupPower == LOW) {
        state = 5;
    } else if (mainPower == LOW) {
        state = 4;
    } else if (state != 2 && state != 3) {
        state = 1;
    }
}

void tempProcess() {
    hum = dht.readHumidity();
    temp = dht.readTemperature();

    if ((isnan(hum) || isnan(temp)) && state == 1) {
        state = 2;
    } else if (!isnan(hum) && !isnan(temp) && state == 2) {
        state = 1;
    }
}

void internetWorks() {
    if ((Ethernet.hardwareStatus() == EthernetNoHardware || Ethernet.linkStatus() == LinkOFF) && state <= 3) {
        state = 3;
    } else if (state == 3) {
        state = 1;
    }
}

void serverProcess() {
    EthernetClient client = server.available();
    if (!client) {
        return;
    }

    // read the HTTP request until the blank line that ends the headers
    boolean currentLineIsBlank = true;
    unsigned long requestStart = millis();
    while (client.connected() && millis() - requestStart < 1000) {
        if (client.available()) {
            char c = client.read();
            if (c == '\n' && currentLineIsBlank) {
                sendResponse(client);
                break;
            }
            if (c == '\n') {
                currentLineIsBlank = true;
            } else if (c != '\r') {
                currentLineIsBlank = false;
            }
        }
    }

    // give the web browser time to receive the data
    delay(1);
    client.stop();
}

void sendResponse(EthernetClient &client) {
    client.println(F("HTTP/1.1 200 OK"));
    client.println(F("Content-Type: application/json"));
    client.println(F("Connection: close"));
    client.println();
    client.println(F("{"));
    client.print(F("\"state\":\""));
    client.print(stateToString(state));
    client.println(F("\","));
    client.print(F("\"time\":"));
    client.print(millis());
    client.println(F(","));
    client.print(F("\"humidity\":"));
    printJsonFloat(client, hum);
    client.println(F(","));
    client.print(F("\"temperature\":"));
    printJsonFloat(client, temp);
    client.println(F(","));
    client.print(F("\"mainPower\":"));
    client.print(mainPower ? 1 : 0);
    client.println(F(","));
    client.print(F("\"backupPower\":"));
    client.println(backupPower ? 1 : 0);
    client.println(F("}"));
}

void printJsonFloat(EthernetClient &client, float value) {
    if (isnan(value)) {
        client.print(F("null"));
    } else {
        client.print(value);
    }
}

const char* stateToString(int currentState) {
    switch (currentState) {
        case 1:
            return "ok";
        case 2:
            return "sensor_error";
        case 3:
            return "internet_error";
        case 4:
            return "main_power_error";
        case 5:
            return "backup_power_error";
        default:
            return "unknown_error";
    }
}

void blinkLeds() {
    unsigned long calcBlue = millis() - previousMillisBlue;
    unsigned long calcOrange = millis() - previousMillisOrange;
    unsigned long calcRed = millis() - previousMillisRed;

    switch (state) {
        case 1:
            //blue
            if ((calcBlue >= longOff && !ledStateBlue) || (calcBlue >= longOn && ledStateBlue)) {
                previousMillisBlue = millis();
                ledStateBlue = !ledStateBlue;
                digitalWrite(ledBlue, getState(ledStateBlue));
                digitalWrite(ledOrange, LOW);
                digitalWrite(ledRed, LOW);
                digitalWrite(buzzer, LOW);
            }
        break;
    
        case 2:
            //blue
            if ((calcBlue >= shortOff && !ledStateBlue) || (calcBlue >= shortOn && ledStateBlue)) {
                previousMillisBlue = millis();
                ledStateBlue = !ledStateBlue;
                digitalWrite(ledBlue, getState(ledStateBlue));
            }

            //orange
            if ((calcOrange >= longOff && !ledStateOrange) || (calcOrange >= longOn && ledStateOrange)) {
                previousMillisOrange = millis();
                ledStateOrange = !ledStateOrange;
                digitalWrite(ledOrange, getState(ledStateOrange));
                digitalWrite(ledRed, LOW);
                digitalWrite(buzzer, LOW);
            }
        break;


        case 3:
            //blue
            if ((calcBlue >= shortOff && !ledStateBlue) || (calcBlue >= shortOn && ledStateBlue)) {
                previousMillisBlue = millis();
                ledStateBlue = !ledStateBlue;
                digitalWrite(ledBlue, getState(ledStateBlue));
            }

            //red
            if ((calcRed >= longOff && !ledStateRed) || (calcRed >= longOn && ledStateRed)) {
                previousMillisRed = millis();
                ledStateRed = !ledStateRed;
                digitalWrite(ledRed, getState(ledStateRed));
                digitalWrite(ledOrange, LOW);
                digitalWrite(buzzer, LOW);
            }
        break;


        case 4:
            //orange
            if ((calcOrange >= shortOff && !ledStateOrange) || (calcOrange >= shortOn && ledStateOrange)) {
                previousMillisOrange = millis();
                ledStateOrange = !ledStateOrange;
                digitalWrite(ledBlue, LOW);
                digitalWrite(ledOrange, getState(ledStateOrange));
                digitalWrite(ledRed, LOW);
                digitalWrite(buzzer, LOW);
            }
        break;

        case 5:
            //red
            if ((calcRed >= shortOff && !ledStateRed) || (calcRed >= shortOn && ledStateRed)) {
                previousMillisRed = millis();
                ledStateRed = !ledStateRed;
                digitalWrite(ledBlue, LOW);
                digitalWrite(ledOrange, LOW);
                digitalWrite(ledRed, getState(ledStateRed));
            }

            // buzzer
            if (((millis() - previousMillisBuzzer >= 500 && buzzerState == true) || (millis() - previousMillisBuzzer >= 200 && buzzerState == false)) && pauseType == 0) {
                previousMillisBuzzer = millis();
                buzzerState = !buzzerState;
                digitalWrite(buzzer, getState(buzzerState));
            }
        break;


        default:
            digitalWrite(ledBlue, LOW);
            digitalWrite(ledOrange, LOW);
            digitalWrite(ledRed, LOW);
        break;
    }

    // RED LIGHT
    if (state == 5 && pauseType != 2) {
        digitalWrite(redLight, HIGH);
    } else {
        digitalWrite(redLight, LOW);
    }

}

int getState(boolean on) {
    if (on) {
        return 1;
    }
    return 0;
}

void checkStopButton() {
    if (pauseType != 0 && millis() - pauseStart >= pauseDuration) {
        pauseType = 0;
    }

    unsigned long buttonStart = millis();
    while (digitalRead(stopButton) == LOW) {
        if (millis() - buttonStart > 5000 && pauseType != 2) {
            digitalWrite(buzzer, HIGH);
        }    
    }

    if (millis() - buttonStart > 5000 && pauseType != 2) {
        pauseType = 2;
        pauseStart = millis();
        pauseDuration = 12UL * 60 * 60 * 1000;
        digitalWrite(buzzer, LOW);
    } else if (millis() - buttonStart > 500 && pauseType == 0) {
        pauseType = 1;
        pauseStart = millis();
        pauseDuration = 30UL * 60 * 1000;
        digitalWrite(buzzer, LOW);
    }
}
