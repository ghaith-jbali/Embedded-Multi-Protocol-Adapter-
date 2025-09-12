# Embedded Multi-Protocol Adapter

## 📌 Overview
This project implements a **reconfigurable multi-protocol communication adapter** for industrial embedded systems.  
It enables **real-time bidirectional conversion** between **UART, SPI, I²C, and CAN** using an **STM32F446RE** microcontroller running **FreeRTOS**, with an **ESP-01 (ESP8266)** hosting a web dashboard for configuration.  
A **Lua scripting engine** is embedded to allow runtime customization of data routing and processing.

The adapter is designed as a **Swiss-Army knife** for engineers, technicians, and students needing a flexible, open, and low-cost protocol bridge.

---

## ✨ Features
- 🔌 Multi-protocol bridging: **UART ↔ SPI ↔ I²C ↔ CAN**  
- ⚙️ Runtime reconfiguration via **ESP-01 web dashboard**  
- 📜 Embedded **Lua scripting engine** for dynamic message processing  
- ⏱️ Deterministic multitasking with **FreeRTOS**  
- 💻 PC companion software for **script editing** and **serial debugging**  
- ⚡ DMA/ISR-driven data paths for low-latency performance  
- 📦 Compact hardware design (STM32F446RE + ESP-01, 3D-printed enclosure)

---

## 🛠️ Hardware Requirements
- **STM32F446RE** (Nucleo board or bare MCU)  
- **ESP-01 (ESP8266)** module  
- **CAN transceiver** (e.g., MCP2551, SN65HVD230)  
- UART/SPI/I²C peripherals for testing  
- Power supply: 5V (USB or external)

---

## 💻 Software Requirements
- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)  
- [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html) (optional)  
- **FreeRTOS** (included in project)  
- **Lua 5.3 embedded build**  
- [ESP8266 Arduino Core](https://github.com/esp8266/Arduino) (for ESP-01)  
- **Python 3.x** (for PC companion tool)

---

## 📂 Repository Structure
