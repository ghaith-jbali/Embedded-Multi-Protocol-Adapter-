# Embedded-Multi-Protocol-Adapter-
Overview

This project implements a reconfigurable multi-protocol communication adapter for industrial embedded systems. It enables real-time bidirectional conversion between UART, SPI, I²C, and CAN using an STM32F446RE microcontroller running FreeRTOS, with an ESP-01 (ESP8266) hosting a web dashboard for configuration. A Lua scripting engine is embedded to allow runtime customization of data routing and processing.

The adapter acts as a Swiss-Army knife for engineers, technicians, and students needing a flexible, open, and low-cost protocol bridge.

Features

Multi-protocol bridging: UART ↔ SPI ↔ I²C ↔ CAN

Runtime reconfiguration via web dashboard (ESP-01 hosted)

Embedded Lua scripting engine for dynamic message processing

Deterministic multitasking with FreeRTOS

PC companion software for script editing and serial debugging

Modular firmware with DMA/ISR-driven data paths

Compact hardware design (STM32F446RE + ESP-01, 3D-printed enclosure)

Hardware Requirements

STM32F446RE (Nucleo or bare MCU)

ESP-01 (ESP8266) module

CAN transceiver (e.g., MCP2551, SN65HVD230)

UART/SPI/I²C peripherals for testing

Power supply: 5V (USB or external)

Software Requirements

STM32CubeIDE (for building STM32 firmware)

STM32CubeMX (optional, for code generation)

FreeRTOS (included in project)

Lua 5.3 embedded build

ESP8266 Arduino Core (for ESP-01 dashboard firmware)

Python 3.x (for PC companion tool)

Repository Structure
├── firmware/
│   ├── stm32/           # STM32 FreeRTOS firmware (C source)
│   ├── esp01/           # ESP-01 Web Dashboard firmware (Arduino/ESP8266)
│   └── lua/             # Embedded Lua VM + example scripts
├── pc-tool/             # Python companion software (GUI for Lua + serial)
├── docs/                # Schematics, diagrams, PFE report extracts
└── README.md

Getting Started
1. Build STM32 Firmware

Open firmware/stm32 in STM32CubeIDE.

Compile and flash to your STM32F446RE.

Connect UART3 ↔ ESP-01, CAN transceiver, and other peripherals.

2. Flash ESP-01 Web Dashboard

Open firmware/esp01 in Arduino IDE or PlatformIO.

Flash the code to ESP-01.

Connect ESP-01 UART to STM32 (UART3).

3. Run PC Companion Tool

Navigate to pc-tool/.

Install requirements:

pip install -r requirements.txt


Launch the GUI:

python main.py

4. Example Workflow

Open the Web Dashboard in your browser (ESP-01 IP).

Configure protocol parameters (baud rate, SPI mode, etc.).

Define routing rules (UART→CAN, SPI→I²C, etc.).

Upload Lua scripts for message filtering.

Monitor traffic via the Terminal tab or the PC tool.

Example Lua Script
-- Simple Lua script to forward CAN messages to UART
function on_can_rx(msg)
    if msg.id == 0x123 then
        uart1.write("Filtered: "..msg.data)
    end
end

Project Status

✅ Prototype completed and tested (low-latency bridging, stable Lua execution).
🚀 Future improvements: cloud integration (MQTT/HTTP), auto protocol detection, advanced routing.

License

This project is released under the MIT License.

Credits

Ghaith Jebali – Firmware & system design

M. Yassine Boussa – Academic supervisor

M. Halim KACEM – Industrial supervisor (CSF)
