#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// ===== AP Credentials =====
const char* AP_SSID = "Multi-protocol Adapter";
const char* AP_PASS = "12345678";

// ===== Server & WebSocket =====
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// ===== Forward incoming UART bytes to WebSocket =====
void checkUART() {
  while (Serial.available()) {
    uint8_t b = (uint8_t)Serial.read();
    ws.binaryAll(&b, 1);
  }
}
//void checkUART() {
//  while (Serial.available()) {
//    char c = Serial.read();
//    if (c == '\n' || lineIdx >= LINE_BUF_SIZE - 1) {
//      lineBuf[lineIdx] = '\0';
//      // Send full line as text frame
//      ws.textAll(String(lineBuf));
//      lineIdx = 0;
//    } else {
//      lineBuf[lineIdx++] = c;
//    }
//  }
//}
// ===== Handle WebSocket messages and forward to UART =====
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type != WS_EVT_DATA) return;
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (!info->final || info->index != 0) return;

  if (info->opcode == WS_TEXT || info->opcode == WS_BINARY) {
    // Forward raw bytes/frame to UART
    Serial.write(data, len);
    // Append newline for text frames
    if (info->opcode == WS_TEXT) Serial.write('\n');
  }
}

// ===== Embedded HTML Dashboard =====
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Adapter Control Panel</title>
    <style>
        /* Existing styles */
        :root {
            --bg: #0f1115;
            --surface: #1b1e24;
            --primary: #26a69a;
            --primary-dark: #1d8c84;
            --text: #e0e0e0;
            --muted: #999;
            --border: #2a2d34;
            --radius: 8px;
        }
        
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: 'Segoe UI', sans-serif;
        }
        
        body {
            background-color: var(--bg);
            color: var(--text);
            padding: 0;
            margin: 0;
        }
        
        .tabs {
            display: flex;
            justify-content: center;
            background-color: var(--surface);
            border-bottom: 1px solid var(--border);
        }
        
        .tab {
            padding: 1rem;
            cursor: pointer;
            color: var(--muted);
            transition: color 0.2s ease;
        }
        
        .tab.active {
            color: var(--primary);
            border-bottom: 2px solid var(--primary);
            font-weight: 500;
        }
        
        .content {
            max-width: 960px;
            margin: auto;
            padding: 2rem 1rem;
        }
        
        .view {
            display: none;
        }
        
        .view.active {
            display: block;
        }
        
        .card {
            background-color: var(--surface);
            border: 1px solid var(--border);
            border-radius: var(--radius);
            padding: 1.5rem;
            margin-bottom: 2rem;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2);
        }
        
        .card h2 {
            margin-bottom: 1rem;
            font-size: 1.2rem;
            border-left: 4px solid var(--primary);
            padding-left: 0.5rem;
        }
        
        input,
        select,
        button {
            width: 100%;
            padding: 0.75rem;
            margin-top: 0.75rem;
            background-color: #2a2d34;
            border: 1px solid var(--border);
            border-radius: var(--radius);
            color: var(--text);
            font-size: 1rem;
        }
        
        button {
            background-color: var(--primary);
            color: #000;
            font-weight: 600;
            cursor: pointer;
            transition: background-color 0.2s ease;
        }
        
        button:hover {
            background-color: var(--primary-dark);
        }
        
        .terminal {
            background: #000;
            color: #0f0;
            height: 300px;
            overflow-y: auto;
            font-family: monospace;
            font-size: 0.9rem;
            padding: 1rem;
            border-radius: var(--radius);
            margin-top: 1rem;
        }
        
        .form-row {
            display: flex;
            gap: 1rem;
            flex-wrap: wrap;
        }
        
        .form-row>* {
            flex: 1;
        }
        
        @media (max-width: 600px) {
            .form-row {
                flex-direction: column;
            }
        }
        
        .checkbox-group {
            display: grid;
            grid-template-columns: repeat(5, 1fr);
            gap: 1rem;
            margin: 1rem 0;
        }
        
        .checkbox-group label {
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }
        
        /* New styles for hex/text toggle */
        .mode-toggle {
            display: flex;
            margin: 10px 0;
            gap: 10px;
        }
        
        .mode-btn {
            background: #1b1e24;
            border: 1px solid #2a2d34;
            border-radius: 4px;
            padding: 5px 10px;
            color: #e0e0e0;
            cursor: pointer;
        }
        
        .mode-btn.active {
            background: #26a69a;
            color: #000;
        }
        
        .hex-line {
            font-family: monospace;
            display: flex;
            flex-wrap: wrap;
        }
        
        .hex-offset {
            color: #999;
            margin-right: 8px;
        }
        
        .hex-byte {
            margin: 0 2px;
        }
    </style>
</head>
<body>
    <div class="tabs">
        <div class="tab active" data-tab="terminal" onclick="switchTab('terminal')">Terminal</div>
        <div class="tab" data-tab="config" onclick="switchTab('config')">Config</div>
        <div class="tab" data-tab="routing" onclick="switchTab('routing')">Routing</div>
    </div>
    <div class="content">
        <div id="terminal" class="view active">
            <div class="card">
                <h2>Serial Console</h2>
                <div class="mode-toggle">
                    <button class="mode-btn active" data-mode="hex" onclick="setDisplayMode('hex')">Hex View</button>
                    <button class="mode-btn" data-mode="text" onclick="setDisplayMode('text')">Text View</button>
                </div>
                <div class="terminal" id="terminalOutput"></div>
                <div class="form-row">
                    <input id="terminalInput" type="text" placeholder="Type command...">
                    <button onclick="sendTerminal()">Send</button>
                    <button onclick="clearTerminal()">Clear</button>
                </div>
            </div>
        </div>
        
<div id="config" class="view">
            <!-- UART Configuration -->
            <div class="card">
                <h2>UART Configuration</h2>
                <label>Baudrate</label>
                <input id="uartBaud" type="number" value="115200">
                <div class="form-row">
                    <div style="flex:1">
                        <label>Word Length</label>
                        <select id="uartWord">
                            <option value="8">8 bits</option>
                            <option value="9">9 bits</option>
                        </select>
                    </div>
                    <div style="flex:1">
                        <label>Stop Bits</label>
                        <select id="uartStop">
                            <option value="1">1 stop</option>
                            <option value="2">2 stops</option>
                        </select>
                    </div>
                    <div style="flex:1">
                        <label>Parity</label>
                        <select id="uartParity">
                            <option value="0">None</option>
                            <option value="1">Even</option>
                            <option value="2">Odd</option>
                        </select>
                    </div>
                </div>
                <div class="form-row">
                    <div style="flex:1">
                        <label>RX Size</label>
                        <input id="uartRx" type="number" value="1" min="1" max="32">
                    </div>
                    
                </div>
                <button onclick="sendUART()">Apply UART</button>
            </div>

            <!-- SPI Configuration -->
            <div class="card">
                <h2>SPI Configuration</h2>
                <div class="form-row">
                    <div style="flex:1">
                        <label>Mode</label>
                        <select id="spiMode">
                            <option value="1">Master (TransmitReceive)</option>
                            <option value="0">Master (Transmit)</option>
                            <option value="2">Master (Slave)</option>
                        </select>
                    </div>
                    <div style="flex:1">
                        <label>Prescaler</label>
                        <select id="spiPrescaler">
                            <option>2</option>
                            <option>4</option>
                            <option>8</option>
                            <option>16</option>
                            <option>32</option>
                            <option>64</option>
                            <option>128</option>
                            <option>256</option>
                        </select>
                    </div>
                </div>
                <div class="form-row">
                    <div style="flex:1">
                        <label>CPOL</label>
                        <select id="spiCpol">
                            <option value="0">Low</option>
                            <option value="1">High</option>
                        </select>
                    </div>
                    <div style="flex:1">
                        <label>CPHA</label>
                        <select id="spiCpha">
                            <option value="0">First edge</option>
                            <option value="1">Second edge</option>
                        </select>
                    </div>
                </div>
                <div class="form-row">
                    <div style="flex:1">
                        <label>Data Size</label>
                        <input id="spiDataSize" type="number" value="8" min="4" max="16">
                    </div>
                    <div style="flex:1">
                        <label>Bit Order</label>
                        <select id="spiBitOrder">
                            <option value="0">MSB First</option>
                            <option value="1">LSB First</option>
                        </select>
                    </div>
                </div>
                <div class="form-row">
                    
                    <div style="flex:1">
                        <label>RX Size</label>
                        <input id="spiRx" type="number" value="1" min="1" max="256">
                    </div>
                </div>
                <button onclick="sendSPI()">Apply SPI</button>
            </div>

            <!-- I2C Configuration -->
            <div class="card">
                <h2>I2C Configuration</h2>
                <label>Speed (Hz)</label>
                <input id="i2cSpeed" type="number" value="100000" min="10000" max="400000">
                <div class="form-row">
                    <div style="flex:1">
                        <label>Mode</label>
                        <select id="i2cMode">
                            <option value="0">Master TX</option>
                            <option value="1">Master RX</option>
                            <option value="2">Memory Read</option>
                            <option value="3">Slave</option>
                        </select>
                    </div>
                    <div style="flex:1">
                        <label>Own Address</label>
                        <input id="i2cOwnAddr" type="text" placeholder="0x00-0x7F" value="0x01">
                    </div>
                </div>
                <div class="form-row">
                    <div style="flex:1">
                        <label>Slave Address</label>
                        <input id="i2cSlaveAddr" type="text" placeholder="0x00-0x7F" value="0x10">
                    </div>
                    <div style="flex:1">
                        <label>Memory Address</label>
                        <input id="i2cMemAddr" type="text" placeholder="0x00-0xFF" value="0x00">
                    </div>
                </div>
                <div class="form-row">
                    <div style="flex:1">
                        <label>RX Size</label>
                        <input id="i2cRx" type="number" value="1" min="1" max="32">
                    </div>
                    
                </div>
                <button onclick="sendI2C()">Apply I2C</button>
            </div>

            <!-- CAN Configuration -->
            <div class="card">
                <h2>CAN Configuration</h2>
                <label>Prescaler</label>
                <input id="canPrescaler" type="number" value="10" min="1" max="1024">
                <div class="form-row">
                    <div style="flex:1">
                        <label>Time Seg1</label>
                        <input id="canSeg1" type="number" value="13" min="1" max="15">
                    </div>
                    <div style="flex:1">
                        <label>Time Seg2</label>
                        <input id="canSeg2" type="number" value="2" min="1" max="8">
                    </div>
                </div>
                <div class="form-row">
                    <div style="flex:1">
                        <label>Message ID</label>
                        <input id="canMsgId" type="text" placeholder="0x000-0x7FF" value="0x123">
                    </div>
                    <div style="flex:1">
                        <label>Filter ID</label>
                        <input id="canFilterId" type="text" placeholder="0x000-0x7FF" value="0x000">
                    </div>
                </div>
                <div class="form-row">
                    <div style="flex:1">
                        <label>Filter Mask</label>
                        <input id="canFilterMask" type="text" placeholder="0x000-0x7FF" value="0x000">
                    </div>
                    
                </div>
                <button onclick="sendCAN()">Apply CAN</button>
            </div>
        </div>

        <!-- Routing Configuration -->
        <div id="routing" class="view">
            <div class="card">
                <h2>Message Routing</h2>
                <div class="form-row">
                    <select id="routeProtocol">
                        <option>UART</option>
                        <option>SPI</option>
                        <option>I2C</option>
                        <option>CAN</option>
                        <option>COM</option>
                    </select>
                </div>
                <div class="checkbox-group">
                    <label><input type="checkbox" id="routeUART">UART</label>
                    <label><input type="checkbox" id="routeSPI">SPI</label>
                    <label><input type="checkbox" id="routeI2C">I2C</label>
                    <label><input type="checkbox" id="routeCAN">CAN</label>
                    <label><input type="checkbox" id="routeCOM">COM</label>
                </div>
                <div class="hint">Check protocols to enable routing from selected source
                <button onclick="setRoute()">Apply Routing</button>
                </div>
                <div class="form-row" style="margin-top: 1rem;">
                    <button onclick="resetRoute()">Reset Routing</button>
                    <button onclick="routeAll()">Route All</button>
                </div>
            </div>
        </div>        
    </div>

    <script>
        let ws;
        let reconnectAttempts = 0;
        let displayMode = "hex"; // "hex" or "text"
        let currentHexLine = null;
        let byteCount = 0;

        function initSocket() {
            ws = new WebSocket(`ws://${location.host}/ws`);
            ws.binaryType = "arraybuffer";
            
            ws.onopen = () => {
                console.log("WebSocket connected");
                reconnectAttempts = 0;
            };
            
            ws.onclose = () => {
                console.log("WebSocket closed, reconnecting...");
                setTimeout(initSocket, Math.min(1000 * Math.pow(2, reconnectAttempts++), 30000));
            };
            
            ws.onerror = (err) => {
                console.error("WebSocket error:", err);
                ws.close();
            };
            
            ws.onmessage = (event) => {
                if (event.data instanceof ArrayBuffer) {
                    const bytes = new Uint8Array(event.data);
                    processBytes(bytes);
                }
                else if (typeof event.data === "string") {
                    appendText(event.data);
                }
            };
        }

        function processBytes(bytes) {
            const output = document.getElementById('terminalOutput');
            
            if (displayMode === "hex") {
                for (let i = 0; i < bytes.length; i++) {
                    const byte = bytes[i];
                    
                    // Create new line every 16 bytes
                    if (byteCount % 16 === 0) {
                        currentHexLine = document.createElement('div');
                        currentHexLine.className = 'hex-line';
                        
                        // Add offset prefix
                        const offset = document.createElement('span');
                        offset.className = 'hex-offset';
                        offset.textContent = byteCount.toString(16).padStart(4, '0') + ': ';
                        currentHexLine.appendChild(offset);
                        
                        output.appendChild(currentHexLine);
                    }
                    
                    // Create byte element
                    const byteSpan = document.createElement('span');
                    byteSpan.className = 'hex-byte';
                    byteSpan.textContent = byte.toString(16).padStart(2, '0');
                    
                    // Color code based on byte value
                    if (byte >= 0x20 && byte <= 0x7E) {
                        byteSpan.style.color = '#0f0'; // Printable ASCII
                    } else if (byte === 0x00) {
                        byteSpan.style.color = '#f00'; // NULL
                    } else if (byte === 0xFF) {
                        byteSpan.style.color = '#ff0'; // 0xFF
                    } else {
                        byteSpan.style.color = '#0af'; // Control chars
                    }
                    
                    currentHexLine.appendChild(byteSpan);
                    byteCount++;
                    
                    // Add space every 4 bytes
                    if ((byteCount % 4 === 0) && (byteCount % 16 !== 0)) {
                        const space = document.createElement('span');
                        space.textContent = ' ';
                        space.style.marginRight = '8px';
                        currentHexLine.appendChild(space);
                    }
                }
            } 
            else { // Text mode
                const decoder = new TextDecoder();
                const text = decoder.decode(bytes);
                appendText(text);
            }
            
            // Auto-scroll to bottom
            output.scrollTop = output.scrollHeight;
        }

        function appendText(text) {
            const output = document.getElementById('terminalOutput');
            const div = document.createElement('div');
            div.textContent = text;
            output.appendChild(div);
        }

        function setDisplayMode(mode) {
            displayMode = mode;
            document.querySelectorAll('.mode-btn').forEach(btn => 
                btn.classList.toggle('active', btn.dataset.mode === mode)
            );
            clearTerminal();
        }

        function clearTerminal() {
            const output = document.getElementById('terminalOutput');
            output.innerHTML = '';
            byteCount = 0;
            currentHexLine = null;
        }

        function sendTerminal() {
            const input = document.getElementById('terminalInput');
            if (ws && ws.readyState === WebSocket.OPEN && input.value) {
                // Echo locally
                appendText("> " + input.value);
                
                // Send to ESP
                ws.send(input.value);
                input.value = '';
            }
        }

        function switchTab(tab) {
            document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
            document.querySelectorAll('.view').forEach(v => v.classList.remove('active'));
            document.querySelector(`.tab[onclick*="${tab}"]`).classList.add('active');
            document.getElementById(tab).classList.add('active');
        }

        function get(id) {
            return document.getElementById(id).value;
        }

        function sendUART() {
            const cmd = `@UART SET ${get('uartBaud')} ${get('uartWord')} ${get('uartStop')} ${get('uartParity')} ${get('uartRx')}`;
            ws.send(cmd);
        }

        function sendSPI() {
            const cmd = `@SPI SET ${get('spiMode')} ${get('spiPrescaler')} ${get('spiCpol')} ${get('spiCpha')} ${get('spiDataSize')} ${get('spiBitOrder')}  ${get('spiRx')}`;
            ws.send(cmd);
        }

        function sendI2C() {
            const cmd = `@I2C SET ${get('i2cSpeed')} ${get('i2cMode')} ${get('i2cOwnAddr')} ${get('i2cSlaveAddr')} ${get('i2cMemAddr')} ${get('i2cRx')} `;
            ws.send(cmd);
        }

        function sendCAN() {
            const cmd = `@CAN SET ${get('canPrescaler')} ${get('canSeg1')} ${get('canSeg2')} ${get('canMsgId')} ${get('canFilterId')} ${get('canFilterMask')} `;
            ws.send(cmd);
        }

        function setRoute() {
            const protocol = get('routeProtocol');
            const bits = [
                document.getElementById('routeUART').checked ? 1 : 0,
                document.getElementById('routeSPI').checked ? 1 : 0,
                document.getElementById('routeI2C').checked ? 1 : 0,
                document.getElementById('routeCAN').checked ? 1 : 0,
                document.getElementById('routeCOM').checked ? 1 : 0
            ].join(' ');
            ws.send(`@ROUTE ${protocol} ${bits}`);
        }

        function resetRoute() {
            ws.send("@ROUTE RESET");
        }

        function routeAll() {
            ws.send("@ROUTE ALL");
        }

        window.onload = initSocket;
    </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);  // Communication with external device

  WiFi.softAP(AP_SSID, AP_PASS);
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* r){ r->send_P(200, "text/html", index_html); });
  server.begin();
}

void loop() {
  checkUART();
  ws.cleanupClients();
  yield();
}
