# Power Checker

By Karel Cermak | [Karlosoft](https://karlosoft.com).

## A simple arduino project to monitor the functionality of the power grid and the backup power supply.

<img src="https://cdn.karlosoft.com/cdn-data/ks/img/powerchecker/github.png" alt="Model" width="700"/>

---

## Features
- Checks the functionality of the power system and the power supply of the backup power supply via USB.
- It controls the control diodes responding to the current status.
- Controls a buzzer that will be triggered if the backup power supply is disconnected.
- Generate simple JSON statistics via the integrated web server.
- DHT temperature sensor and buzzer/ceiling light mute button.

<br>

## States of the system
- **Everything ok (1)**
    - Blue light slowly blinking
    - Orange light off
    - Red light off

- **DHT temperature sensor error (2)**
    - Blue light blinking fast
    - Orange light slowly blinking
    - Red light off

- **Ethernet error (3)**
    - Blue light blinking fast
    - Orange light off
    - Red light slowly blinking

- **Main grid off (4)**
    - Blue light off
    - Orange light blinking fast
    - Red light off
    - Ceiling light turned off

- **Backup power supply off (5)**
    - Blue light off
    - Orange light off
    - Red light blinking fast
    - Ceiling light turned on
    - Buzzer activated

<br>

## How to stop the buzzer and the ceiling light?

- **Hold button for 500 ms**
    - Deactivate the buzzer for 30 minutes

- **Hold button for 5000 ms**
    - Deactivate the buzzer and ceiling light for 12 hours

<br>

## Problem with ethernet shields W5100 
- If it is necessary to restart the arduino via the restart button to get the web server working, this might help:
    - Put a 100 nF capacitor between the RESET and GND pins (a larger capacitor should also work).
    - Place 100 ohm resistors between ethernet pins 1 - 2 and 3 - 6 (not tested).