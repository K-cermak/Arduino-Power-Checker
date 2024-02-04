#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>;

// OVERALL
int state = 1;

// ETHERNET
byte mac[] = {0xA6, 0x2A, 0xFA, 0xC7, 0x78, 0xC9}; // make it random
IPAddress ip(10, 0, 0, 60);
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
unsigned long longOn = 2000;
unsigned long longOff = 400;
unsigned long shortOn = 200;
unsigned long shortOff = 75;

// ALARMS
const byte redLight = 32;
const byte buzzer = 33;
boolean buzzerState = true;
unsigned long previousMillisBuzzer = 0;

// STOP BUTTON
const byte stopButton = 34;
byte pauseType = 0; // 0 - no pause, 1 - pause alarm, 2 - pause light and alarm
unsigned long pauseTill = 0;


void(* resetFunc) (void) = 0;

void setup() {
    pinMode(checkMain, INPUT);
    pinMode(checkBackup, INPUT);

    pinMode(ledBlue, OUTPUT);
    pinMode(ledOrange, OUTPUT);
    pinMode(ledRed, OUTPUT);
    pinMode(redLight, OUTPUT);
    pinMode(buzzer, OUTPUT);

    pinMode(stopButton, INPUT_PULLUP);


    Ethernet.begin(mac, ip);
    server.begin();
    dht.begin();


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
    data_process();
    temp_process();
    internet_works();
    server_process();
    blinkLeds();
    checkReset();
    checkStopButton();
}


void data_process() {
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

void temp_process() {
    hum = dht.readHumidity();
    temp = dht.readTemperature();

    if ((isnan(hum) || isnan(temp)) && state == 1) {
        state = 2;
    } else if (!isnan(hum) && !isnan(temp) && state == 2) {
        state = 1;
    }
}

void internet_works() {
    if ((Ethernet.hardwareStatus() == EthernetNoHardware || Ethernet.linkStatus() == LinkOFF) && state <= 3) {
        state = 3;
    } else if (state == 3) {
        state = 1;
    }
}

void server_process() {
    EthernetClient client = server.available();
    if (client && client.connected() && client.available()) {
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json");
        client.println("Connection: close");
        client.println();
        client.println("{");
        client.println("\"state\":\"" + stateToString(state) + "\",");
        client.println("\"time\":" + String(millis()) + ",");
        client.println("\"humidity\":" + String(hum) + ",");
        client.println("\"temperature\":" + String(temp) + ",");
        client.println("\"mainPower\":" + String(mainPower) + ",");
        client.println("\"backupPower\":" + String(backupPower) + "}");
        client.println("}");

        // give the web browser time to receive the data
        delay(1);
        client.stop();
    }
}

String stateToString(int state) {
    switch (state) {
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
            if ((calcBlue >= longOn && !ledStateBlue) || (calcBlue >= longOff && ledStateBlue)) {
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
            if ((calcBlue >= shortOn && !ledStateBlue) || (calcBlue >= shortOff && ledStateBlue)) {
                previousMillisBlue = millis();
                ledStateBlue = !ledStateBlue;
                digitalWrite(ledBlue, getState(ledStateBlue));
            }

            //orange
            if ((calcOrange >= longOn && !ledStateOrange) || (calcOrange >= longOff && ledStateOrange)) {
                previousMillisOrange = millis();
                ledStateOrange = !ledStateOrange;
                digitalWrite(ledOrange, getState(ledStateOrange));
                digitalWrite(ledRed, LOW);
                digitalWrite(buzzer, LOW);
            }
        break;


        case 3:
            //blue
            if ((calcBlue >= shortOn && !ledStateBlue) || (calcBlue >= shortOff && ledStateBlue)) {
                previousMillisBlue = millis();
                ledStateBlue = !ledStateBlue;
                digitalWrite(ledBlue, getState(ledStateBlue));
            }

            //red
            if ((calcRed >= longOn && !ledStateRed) || (calcRed >= longOff && ledStateRed)) {
                previousMillisRed = millis();
                ledStateRed = !ledStateRed;
                digitalWrite(ledRed, getState(ledStateRed));
                digitalWrite(ledOrange, LOW);
                digitalWrite(buzzer, LOW);
            }
        break;


        case 4:
            //orange
            if ((calcOrange >= shortOn && !ledStateOrange) || (calcOrange >= shortOff && ledStateOrange)) {
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
            if ((calcRed >= shortOn && !ledStateRed) || (calcRed >= shortOff && ledStateRed)) {
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

int getState(boolean state) {
    if (state) {
        return 1;
    }
    return 0;
}

void checkStopButton() {
    if (pauseTill < millis()) {
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
        pauseTill = millis() + (12 * 60 * 60 * 1000L);
        digitalWrite(buzzer, LOW);
    } else if (millis() - buttonStart > 500 && pauseType == 0) {
        pauseType = 1;
        pauseTill = millis() + (30 * 60 * 1000L);
        digitalWrite(buzzer, LOW);
    }
}


void checkReset() {
    // reset after 45 days of running
    if (millis() > 4000000000L) {
        resetFunc();
    }
}