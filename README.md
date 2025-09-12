# Embedded Multi-Protocol Adapter

Reconfigurable multi-protocol communication adapter (STM32F446RE + ESP-01) — FreeRTOS firmware + embedded Lua VM + ESP-01 web dashboard + PC companion tool.

## 📌 Overview

This repository contains the firmware and tools for a prototype multi-protocol adapter that enables real-time routing and conversion between **UART, SPI, I²C, and CAN**. The system is powered by an **STM32F446RE** running **FreeRTOS** with an embedded **Lua VM** for scripting, paired with an **ESP-01 (ESP8266)** module hosting a web dashboard. A PC companion tool (Windows installer or Python GUI) is provided for editing/uploading Lua scripts, serial monitoring, and debugging.

## ✨ Features

- Bidirectional protocol bridging: **UART ↔ SPI ↔ I²C ↔ CAN**
- Runtime reconfiguration via **ESP-01 web dashboard**
- Embedded **Lua** scripting for message filtering, processing, and routing
- Deterministic task scheduling with **FreeRTOS**
- PC companion tool: Windows `.exe` installer or Python GUI for script editing and serial monitoring
- DMA/ISR-driven low-latency data paths for real-time performance

## 🗂 Repository Structure

- `ESP01 Web Dashboard/` — ESP-01 dashboard firmware and static web UI files
- `PC Companion Software/` — Python-based PC tool (script editor and serial monitor), including a Windows `.exe` installer and Python GUI script
- `STM32 Firmware.zip` — Zipped STM32/FreeRTOS firmware project (for STM32CubeIDE)
- `README.md` — This file
- `LICENSE` — MIT License file

## 🔧 Requirements

### Hardware
- **STM32F446RE** (Nucleo board or custom breakout)
- **ESP-01 (ESP8266)** module
- **CAN transceiver** (e.g., MCP2551, SN65HVD230)
- **USB-to-Serial adapter** or **ST-Link** for flashing
- Wiring: STM32 UART3 ↔ ESP-01 UART (TX/RX + GND)
- Power supply: 5V USB or equivalent

### Software
- **STM32CubeIDE** — To build and flash the STM32 firmware
- **Arduino IDE** or **PlatformIO** — For flashing ESP-01 dashboard firmware
- **Python 3.x** — For running the Python GUI (`main.py`)
- PC companion tool includes:
  - Windows `.exe` installer (double-click to run)
  - Python GUI script (`main.py`)

## 🚀 Getting Started

Follow these steps in order: STM32 firmware → ESP-01 dashboard → PC companion tool.

### 1. STM32 Firmware (`STM32 Firmware.zip`)
1. Unzip `STM32 Firmware.zip` to a folder.
2. Open the project in **STM32CubeIDE** (load the `.ioc` or project file).
3. Configure your debug probe (ST-Link, JTAG, etc.) if necessary.
4. Build and flash the firmware to the **STM32F446RE**.
5. Connect hardware peripherals (CAN transceiver, SPI/I²C devices, etc.) and wire **UART3** to ESP-01 UART.

**Notes**:
- The firmware includes FreeRTOS and an embedded Lua VM. If you modify clock or pin configurations, regenerate code via CubeMX or update HAL settings.
- Ensure USART pins for ESP-01 match the wiring in your board and ESP firmware.

### 2. ESP-01 Web Dashboard (`ESP01 Web Dashboard/`)
1. Open the `ESP01 Web Dashboard/` folder in **Arduino IDE** or **PlatformIO**.
2. Install the **ESP8266 board package** and select *Generic ESP8266 Module* or *ESP-01*.
3. Set the correct COM port and upload settings (flash size/mode).
4. Flash the dashboard firmware to the ESP-01.
5. Connect ESP-01 UART pins to STM32 UART3 (TX↔RX, RX↔TX, common GND).
6. After boot, access the ESP-01 IP address in a browser (check serial output for the IP or use captive setup if available).

**Dashboard Features**:
- Real-time serial monitor (Terminal)
- Protocol configuration (baud rates, SPI mode, I²C params, CAN bitrate)
- Routing matrix editor
- Lua script upload interface (if implemented)

### 3. PC Companion Software (`PC Companion Software/`)
Choose one of the following methods:

#### A. Windows Installer (`.exe`)
1. Navigate to `PC Companion Software/` and run the `.exe` installer.
2. Follow the installation steps to create a desktop/start-menu shortcut.
3. Use the GUI to:
   - Edit and save Lua scripts
   - Upload scripts to the STM32 (via serial or dashboard)
   - Monitor serial traffic and logs

**Note**: If Windows flags the installer as unknown, verify the file and allow it to run (common for unsigned test installers).

#### B. Python GUI
1. Ensure **Python 3.x** is installed.
2. Open a terminal and navigate to:
   ```bash
   cd PC Companion Software
   ```
3. (Optional) Create and activate a virtual environment:
   ```bash
   python -m venv venv
   # Windows
   venv\Scripts\activate
   # macOS/Linux
   source venv/bin/activate
   ```
4. Install dependencies (if `requirements.txt` is included):
   ```bash
   pip install -r requirements.txt
   ```
5. Run the Python script:
   ```bash
   python lua_editor.py
   ```
6. Use the GUI to edit/upload Lua scripts and monitor serial logs.

**Serial Connection Tip**:
- Connect the STM32 to the PC via USB (if the board supports USB-to-UART) or a USB-to-TTL adapter. Select the correct COM port in the PC tool.

## 🔁 Typical Workflow
1. Flash STM32 firmware and ESP-01 dashboard.
2. Open the ESP dashboard and configure protocol parameters (baud, SPI mode, CAN speed).
3. Define routing rules using the dashboard or PC tool.
4. Edit a Lua script in the PC companion GUI or a local editor.
5. Upload the Lua script to the STM32 via the dashboard or serial upload.
6. Monitor traffic using the dashboard Terminal or PC companion serial monitor.

## 📝 Example Lua Script
```lua
-- Example: Wrap UART messages with '#' and forward to CAN
function handle_message(protocol, data)
    if protocol == UART then
        local wrapped_data = "#" .. data .. "#"
        send_message(CAN, wrapped_data)
    else
        send_message(CAN, data)
    end
end
```

## ⚠️ Troubleshooting
- **No ESP IP address**: Check ESP-01 serial output at 115200 baud. Verify Wi-Fi credentials (STA mode) or use AP mode if available.
- **Garbled serial data**: Ensure matching baud rates and 3.3V logic levels between STM32 and ESP-01.
- **CAN not visible**: Check CAN transceiver wiring and termination resistors (120Ω at each bus end).
- **Lua scripts not executing**: Verify upload success and check Lua VM logs in the dashboard Terminal. Test with small scripts.
- **Installer blocked**: On Windows, right-click the `.exe` and select "Run as administrator" if needed.

## 📊 Project Status
- ✅ Prototype completed and tested in lab: Low-latency protocol bridging, stable Lua execution.
- 🚧 Planned features:
  - Cloud integration (MQTT/HTTP)
  - Auto protocol detection and heuristics
  - Enhanced routing UI and traffic analytics

## 📜 License
This project is licensed under the **MIT License**. See the `LICENSE` file for details.

## 🙌 Credits
- **Ghaith Jebali** — Firmware and system design, report author
- **M. Yassine Boussa** — Academic supervisor
- **M. Halim KACEM** — Industrial supervisor (CSF)

## 📬 Contact/Support
For assistance with building, flashing, or running the tools, open an issue in this repository and include:
- Your operating system (Windows/macOS/Linux)
- PC tool version used (installer or Python script)
- Serial logs (if applicable) and wiring description
