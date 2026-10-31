# Power Checker

By Karel Cermak | [Karlosoft](https://karlosoft.com).

## A simple Arduino project that monitors the state of the mains power supply and the backup power supply.

<img src="https://cdn.karlosoft.com/cdn-data/ks/img/powerchecker/github-project.png" alt="Project" width="700"/>
<img src="https://cdn.karlosoft.com/cdn-data/ks/img/powerchecker/github-model.png" alt="Model" width="700"/>

---

## Features
- Monitors the mains power supply and the backup power supply (via USB).
- Indicates the current state with status LEDs.
- Sounds a buzzer when the backup power supply is disconnected.
- Serves simple JSON statistics through a built-in web server.
- Includes a DHT22 temperature and humidity sensor and a button to mute the buzzer and the ceiling light.

<br>

## System states
- **Everything OK (1)**
    - Blue LED blinking slowly
    - Orange LED off
    - Red LED off

- **DHT sensor error (2)**
    - Blue LED blinking fast
    - Orange LED blinking slowly
    - Red LED off

- **Ethernet error (3)**
    - Blue LED blinking fast
    - Orange LED off
    - Red LED blinking slowly

- **Mains power off (4)**
    - Blue LED off
    - Orange LED blinking fast
    - Red LED off
    - Ceiling light off

- **Backup power off (5)**
    - Blue LED off
    - Orange LED off
    - Red LED blinking fast
    - Ceiling light on
    - Buzzer on

<br>

## How to silence the buzzer and the ceiling light

- **Hold the button for 0.5 seconds**
    - Mutes the buzzer for 30 minutes.

- **Hold the button for 5 seconds**
    - Mutes the buzzer and turns off the ceiling light for 12 hours.

<br>

## Known issues with the W5100 Ethernet shield
- If the web server only starts working after you press the Arduino's reset button, the following may help:
    - Connect a 100 nF capacitor between the RESET and GND pins (a larger capacitor should also work).
    - Connect 100 Ω resistors between Ethernet pins 1–2 and 3–6 (untested).
