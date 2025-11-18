# CAN-Sniffer

A **CAN frame sniffer system for the NUCLEO-H755ZI-Q board**, implemented as a reusable **C library**.

Features include:

* Automatic CAN baud‑rate detection
* Manual CAN baud‑rate configuration
* CAN ID message filtering

The setup has been successfully tested on a vehicle’s OBD-II port, capturing live CAN data.

This README provides the essential steps needed to **reconstruct the project** inside STM32CubeIDE. For a full technical explanation, Hardware and Software overview, and detailed methodology, refer to the `CAN_Sniffer.pdf` report included in the repository, along with the documentation provided in the code files.

---

## Project Structure

The repository contains:
* `Report/`
  * `CAN_Sniffer.pdf` – Full project documentation

* `Src/`
  * `CAN_Sniffer.ioc` – STM32CubeMX configuration file for project reconstruction
  * `main.c` – Main application source file for the CM7 core

* `My_Modules/` – Custom drivers and feature modules
  * `Drivers/`
    * `can/` – CAN handling
      * `my_can.c` 
      * `my_can.h`
    * `debug/` - Debug support
      * `my_debug.c`
      * `my_debug.h`
    * `stdio/` - Lightweight I/O over UART
      * `my_stdio.c`
      * `my_stdio.h`
    * `uart/` - UART support
      * `my_uart.c`
      * `my_uart.h`
  * `Features/`
    * `settings/` - CAN Sniffer settings menu interface
      * `settings_menu.c`
      * `settings_menu.h`

* `Speedometer/` - Python code for Data Visualization
  * `analog_speedometer.py`
  * `digital_speedometer.py`
---

## How to Reconstruct the Project in STM32CubeIDE

### 1. Clone the repository

```
git clone https://github.com/ZiakasSt/CAN-Sniffer.git
```

Place it anywhere you want on your system.

---

### 2. Create the STM32 Project

1. Open **STM32CubeIDE** and choose a workspace.
2. Go to: **File → New → STM32 Project from an Existing STM32CubeMX Configuration File (.ioc)**
3. Select `Src/CAN_Sniffer.ioc` from the repository.
4. Finish wizard and let the IDE generate the base project.

---

## 3. Replace the Generated `main.c`

Navigate to:

```
CAN_Sniffer/CAN_Sniffer_CM7/Core/Src/
```

and **replace the generated** `main.c` with the `main.c` from the repository.

---

## 4. Add the Custom Modules

1. In the Project Explorer, right‑click `CAN_Sniffer_CM7` → **New → Folder**.
2. Name it: `My_Modules`.
3. Drag and drop into the new folder the modules from the repository:

   * `Drivers`
   * `Features`

---

## 5. Include Paths

Right‑click `CAN_Sniffer_CM7` → **Properties**.

### Navigate to:

**C/C++ Build → Settings → MCU/MPU GCC Compiler → Include Paths → Add directory path → Workspace**

Then choose the following paths:

* `My_Modules/Drivers/can`
* `My_Modules/Drivers/debug`
* `My_Modules/Drivers/stdio`
* `My_Modules/Drivers/uart`
* `My_Modules/Features/settings`

Click **Apply** (bottom right).

---

## 6. Add Source Locations

Still inside Properties:

### Go to: 

**C/C++ General → Paths and Symbols → Source Location → Add Folder**

Select the entire folder:

* `My_Modules`

Click **Apply and Close**.

---

## 7. Build and Run

1. Right‑click `CAN_Sniffer_CM7` → **Build Project**
2. After a successful build: **Run As → STM32 C/C++ Application**

---

## Data Visualization

In case you want to test the Sniffer on your own car, once you identify the CAN frame that carries the vehicle speed, you can use the attached Python script as a speedometer. Adjust anything necessary in the script, such as the serial port.