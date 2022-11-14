# Feormgast 
*[neo-Olde English Farm Spirit]* 

ESP32 project to monitor and automate various small agriculture projects

The first use case will be opening and closing the chicken coop door. It should automatically open and close around sun rise/set and be monitored and manually controllable remotely.

My intention is to develop this into a modular framework to 
support a variety of home / garden / farm projects.

## Deployment Instructions

### secrets.h

Create text file /src/secrets.h containing the password for the 
hosted wifi network.

  > const char *AP_WIFI_PASSWORD = "*****";

### Filesystem Image for SPIFFS

### Compile and Upload Firmware

### Over The Air (OTA) Update

## First Prototype BOM

* 1x Unreliable automatic chicken door with dc motor
* 3x AAA Batteries
* 1x ESP-WROOM-32 DEVKIT V1 Development Board
* 1x KeeYees ESP32 Breakout Board
* 1x Aideepen Dual H-Bridge Motor Driver
* 1x USB Micro Cord
* 1x USB Battery Pack
* Wire & Solder

## Future Development Thoughts

* Clean Code
 * Unit Testing Framework
 * **Refactor Into Objects**
  * ~~or maybe Modules~~
 * Integration Tests
 * Diagram
* Usability 
 * Interface Design
  * Add basic front end framework?
  * React?
 * Async Webserver
 * Dashboard and/or Notifications
* **Power Management**
  * Unify Battery
  * Battery Monitor 
   * MAX17043 Board
  * **Low Power / Sleep Mode** 
  * **Connect To Network Instead of Hosting**
   * **ESP-NOW Protocol**
    * **Separate board to bridge to Wifi**
  * Solar Panel
* Functionality
 * **Endstops**
 * Temperature Control
  * Built In Temp Sensor
 * Humidity Control
 * Irrigation Control
 * Camera
 * Remote Access

### Prototype Two

* Switch Boards (to FireBeetle?)
 