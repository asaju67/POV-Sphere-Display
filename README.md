# POV Sphere Display

A wireless **Persistence-of-Vision (POV) LED display** developed as a University of Illinois Urbana-Champaign Electrical & Computer Engineering Senior Design project. The system displays text and graphics by rapidly rotating a custom LED array while synchronizing image updates with a Hall-effect sensor. Wireless inductive power transfer eliminates slip rings, allowing continuous rotation.

The project combines embedded systems, PCB design, power electronics, wireless power transfer, and web-based control into a complete interactive display platform.

---

## Features

- Wireless inductive power transfer
- ESP32-based embedded controller
- Browser-based control interface
- Hall-effect synchronized image rendering
- Countdown timer mode
- SD card image storage
- Adjustable LED brightness using PWM
- Supports up to 360 display columns per revolution
- Custom PCB designed in KiCad

---

# Project Structure

```
POV-Sphere-Display/
│
├── Firmware/
│   ├── POV_Display.ino
│   ├── Display.cpp
│   ├── WebServer.cpp
│   └── ...
│
├── Hardware/
│   ├── KiCad/
│   │   ├── *.kicad_pro
│   │   ├── *.kicad_pcb
│   │   ├── *.kicad_sch
│   │   └── ...
│   │
│   └── Manufacturing/
│       ├── Gerbers/
│       ├── Drill Files/
│       └── BOM.csv
│
├── Documentation/
│   └── POV Sphere Display Final Report.pdf
│
└── README.md
```

---

# System Overview

The display consists of two primary subsystems.

### Stationary Base

- 24 V power supply
- BLDC motor
- Motor controller
- Inductive transmitter coil

### Rotating Display

- ESP32-WROOM-32E
- Four LED driver ICs
- 64 side-mounted LEDs
- Hall-effect sensor
- Buck converter
- Inductive receiver coil
- MicroSD card interface

Power is transferred wirelessly from the stationary base to the rotating PCB through inductive coupling, eliminating the need for mechanical slip rings.

---

# Hardware Design

The custom electronics were designed in **KiCad 9**.

The hardware includes:

- Complete schematics
- PCB layout
- Custom footprints
- Manufacturing files
- Design rule checks

The KiCad project can be opened by loading the `.kicad_pro` file using KiCad 9 or later.

---

# Manufacturing the PCB

To manufacture the board:

1. Open the KiCad project.
2. Verify the PCB layout.
3. Generate Gerber and drill files if they are not already included.
4. Upload the manufacturing files to your preferred PCB fabrication service (PCBWay, JLCPCB, OSH Park, etc.).

---

# Firmware

The firmware is written using the Arduino framework for the ESP32.

Major functionality includes:

- Wi-Fi access point
- Browser-based configuration interface
- LED driver communication
- Hall sensor synchronization
- SD card image loading
- Countdown timer mode
- PWM brightness control

---

# Web Interface

After programming the ESP32:

1. Power the system.
2. Connect to the ESP32 Wi-Fi network.
3. Open the device IP address (typically `192.168.4.1`).
4. Configure display settings through the web interface.
5. Upload image data or start the countdown timer.

---

# Display Format

Display images are stored as 64-bit hexadecimal values.

Example:

```
0x0000001818000000
```

Each value represents one vertical column of LEDs.

Maximum supported display width:

- **360 columns per revolution**

---

# Hall Sensor Synchronization

A Hall-effect sensor detects a magnet once every revolution.

The firmware uses this information to:

- Measure rotational speed
- Synchronize image rendering
- Maintain image stability
- Compensate for RPM variation

---

# ESP32 Pin Assignments

| Component | GPIO |
|-----------|------|
| Hall Sensor | GPIO34 |
| SD MISO | GPIO19 |
| SD MOSI | GPIO23 |
| SD CLK | GPIO18 |
| SD CS | GPIO5 |
| LED Brightness PWM | GPIO21 |

---

# Software Dependencies

- Arduino IDE
- ESP32 Arduino Core
- WiFi
- WebServer
- SPI
- SD

---

# Documentation

The repository includes a complete project report located in the **Documentation** folder.

The report contains:

- Project motivation
- System architecture
- Circuit design
- PCB layouts
- Hardware photographs
- Firmware implementation
- Testing methodology
- Experimental results
- Lessons learned
- Future improvements

For a detailed explanation of the project design and implementation, refer to the final report.

---

# Current Limitations

- Mechanical balancing limited safe operation at higher rotational speeds.
- Reliable image rendering depends on stable rotational velocity.
- SD card performance is dependent on proper PCB assembly.
- Designed for one Hall sensor pulse per revolution.

---

# Future Improvements

- Improved rotor balancing
- Dynamic brightness compensation
- Higher display resolution
- Multi-color LED support
- Automatic RPM calibration
- OTA firmware updates
- Improved web interface

---

# Author

**Ashley Saju**

Department of Electrical & Computer Engineering

University of Illinois Urbana-Champaign

Senior Design Project

---

# License

This project is provided for educational and research purposes.
