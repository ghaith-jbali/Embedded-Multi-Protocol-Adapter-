import customtkinter as ctk
import tkinter as tk
from tkinter import scrolledtext, messagebox
import serial
import serial.tools.list_ports
import threading
import time
import re

# Global variables
running = True
ser = None
connection_status = False

# Set modern appearance
ctk.set_appearance_mode("Dark")
ctk.set_default_color_theme("blue")

# Modern color scheme
COLORS = {
    'bg_primary': '#0d1117',
    'bg_secondary': '#161b22',
    'bg_tertiary': '#21262d',
    'accent': '#238636',
    'accent_hover': '#2ea043',
    'text_primary': '#f0f6fc',
    'text_secondary': '#8b949e',
    'border': '#30363d',
    'error': '#da3633',
    'warning': '#d29922'
}

# Lua syntax highlighting colors
SYNTAX_COLORS = {
    'keywords': '#ff7b72',  # Red for keywords
    'functions': '#d2a8ff',  # Purple for functions
    'strings': '#a5d6ff',  # Light blue for strings
    'comments': '#8b949e',  # Gray for comments
    'numbers': '#79c0ff',  # Blue for numbers
    'operators': '#ff7b72',  # Red for operators
    'special': '#ffa657'  # Orange for special characters
}

# Lua keywords and functions
LUA_KEYWORDS = [
    'and', 'break', 'do', 'else', 'elseif', 'end', 'false', 'for', 'function',
    'if', 'in', 'local', 'nil', 'not', 'or', 'repeat', 'return', 'then',
    'true', 'until', 'while', 'goto'
]

LUA_FUNCTIONS = [
    'print', 'type', 'next', 'pairs', 'ipairs', 'tonumber', 'tostring',
    'string', 'table', 'math', 'debug','CAN',
    'gpio', 'tmr', 'wifi', 'net', 'file', 'node',
    'SPI', 'I2C', 'UART', 'bit', 'crypto', 'sjson','COM'
]


def squeeze_code(code):
    """Compress Lua code by removing comments and extra whitespace"""
    lines = [line.split("--")[0].strip() for line in code.splitlines()
             if line.strip() and not line.strip().startswith("--")]
    squeezed = " ".join(lines)
    return squeezed


def check_wrapping_characters(code):
    """Check if code contains script wrapping characters $ that could interfere"""
    if '$' in code:
        return True, "Warning: Script contains '$' characters which are used for wrapping. This may cause issues."
    return False, ""


def apply_syntax_highlighting():
    """Apply syntax highlighting to the code editor"""
    try:
        # Clear existing tags
        for tag in code_editor.tag_names():
            if tag not in ['sel', 'hyper']:
                code_editor.tag_delete(tag)

        content = code_editor.get("1.0", tk.END)

        # Highlight comments (-- to end of line)
        for match in re.finditer(r'--.*$', content, re.MULTILINE):
            start = f"1.0+{match.start()}c"
            end = f"1.0+{match.end()}c"
            code_editor.tag_add("comment", start, end)
            code_editor.tag_config("comment", foreground=SYNTAX_COLORS['comments'])

        # Highlight strings (both single and double quotes)
        for match in re.finditer(r'(["\'])(?:(?=(\\?))\2.)*?\1', content):
            start = f"1.0+{match.start()}c"
            end = f"1.0+{match.end()}c"
            code_editor.tag_add("string", start, end)
            code_editor.tag_config("string", foreground=SYNTAX_COLORS['strings'])

        # Highlight numbers
        for match in re.finditer(r'\b\d+\.?\d*\b', content):
            start = f"1.0+{match.start()}c"
            end = f"1.0+{match.end()}c"
            code_editor.tag_add("number", start, end)
            code_editor.tag_config("number", foreground=SYNTAX_COLORS['numbers'])

        # Highlight keywords
        for keyword in LUA_KEYWORDS:
            pattern = r'\b' + re.escape(keyword) + r'\b'
            for match in re.finditer(pattern, content):
                start = f"1.0+{match.start()}c"
                end = f"1.0+{match.end()}c"
                code_editor.tag_add("keyword", start, end)
                code_editor.tag_config("keyword", foreground=SYNTAX_COLORS['keywords'],
                                       font=("JetBrains Mono", 11, "bold"))

        # Highlight functions
        for function in LUA_FUNCTIONS:
            pattern = r'\b' + re.escape(function) + r'\b'
            for match in re.finditer(pattern, content):
                start = f"1.0+{match.start()}c"
                end = f"1.0+{match.end()}c"
                code_editor.tag_add("function", start, end)
                code_editor.tag_config("function", foreground=SYNTAX_COLORS['functions'])

        # Highlight operators and special characters
        operators = ['+', '-', '*', '/', '%', '^', '=', '~', '<', '>', '(', ')', '{', '}', '[', ']', ',', ';', ':', '.']
        for op in operators:
            pattern = re.escape(op)
            for match in re.finditer(pattern, content):
                start = f"1.0+{match.start()}c"
                end = f"1.0+{match.end()}c"
                code_editor.tag_add("operator", start, end)
                code_editor.tag_config("operator", foreground=SYNTAX_COLORS['operators'])

        # Highlight $ characters in red as they're problematic
        for match in re.finditer(r'\$', content):
            start = f"1.0+{match.start()}c"
            end = f"1.0+{match.end()}c"
            code_editor.tag_add("warning_char", start, end)
            code_editor.tag_config("warning_char", foreground=COLORS['error'], background="#4a1a1a")

    except Exception as e:
        print(f"Syntax highlighting error: {e}")


def on_code_change(event=None):
    """Handle code editor changes for real-time syntax highlighting"""
    # Schedule syntax highlighting after a short delay to avoid lag
    root.after_idle(apply_syntax_highlighting)


def update_status(message, color="#238636"):
    """Update status bar with modern styling"""
    status_bar.configure(text=f"● {message}", text_color=color)
    root.after(3000, lambda: status_bar.configure(text="● Ready", text_color=COLORS['text_secondary']))


def connect_uart():
    """Establish UART connection"""
    global ser, connection_status
    try:
        selected_port = com_port_combobox.get()
        if not selected_port:
            update_status("No COM port selected", COLORS['error'])
            return False

        if ser and ser.is_open:
            ser.close()

        ser = serial.Serial(selected_port, baudrate=115200, timeout=0.1)
        connection_status = True
        connect_button.configure(text="Disconnect", fg_color=COLORS['error'])
        update_status(f"Connected to {selected_port}")
        return True

    except Exception as e:
        connection_status = False
        connect_button.configure(text="Connect", fg_color=COLORS['accent'])
        update_status(f"Connection failed: {str(e)}", COLORS['error'])
        return False


def disconnect_uart():
    """Disconnect UART connection"""
    global ser, connection_status
    try:
        if ser and ser.is_open:
            ser.close()
        connection_status = False
        connect_button.configure(text="Connect", fg_color=COLORS['accent'])
        update_status("Disconnected")
    except Exception as e:
        update_status(f"Disconnect error: {str(e)}", COLORS['error'])


def toggle_connection():
    """Toggle UART connection"""
    if connection_status:
        disconnect_uart()
    else:
        connect_uart()


def send_code():
    """Send Lua code via UART"""
    try:
        if not connection_status or not ser or not ser.is_open:
            update_status("Not connected to COM port", COLORS['error'])
            return

        lua_code = code_editor.get("1.0", tk.END).strip()
        if not lua_code:
            update_status("No code to send", COLORS['warning'])
            return

        # Check for problematic characters
        has_issue, warning_msg = check_wrapping_characters(lua_code)
        if has_issue:
            result = messagebox.askyesno("Warning", f"{warning_msg}\n\nDo you want to continue anyway?")
            if not result:
                return

        squeezed_code = squeeze_code(lua_code)
        wrapped_code = f"${squeezed_code}$"

        ser.write(wrapped_code.encode())

        # Add to response window with modern formatting
        timestamp = time.strftime("%H:%M:%S")
        serial_monitor.insert(tk.END, f"[{timestamp}] ⬆️ SENT: {wrapped_code}\n")
        serial_monitor.see(tk.END)

        update_status("Code sent successfully")

    except Exception as e:
        update_status(f"Send error: {str(e)}", COLORS['error'])


def send_message():
    """Send message from serial monitor input"""
    try:
        if not connection_status or not ser or not ser.is_open:
            update_status("Not connected to COM port", COLORS['error'])
            return

        message = message_entry.get().strip()
        if not message:
            return

        #ser.write((message + '\n').encode())
        ser.write((message).encode())

        # Add to serial monitor
        timestamp = time.strftime("%H:%M:%S")
        serial_monitor.insert(tk.END, f"[{timestamp}] 💬 MSG: {message}\n")
        serial_monitor.see(tk.END)

        # Clear input
        message_entry.delete(0, tk.END)

        update_status("Message sent")

    except Exception as e:
        update_status(f"Send error: {str(e)}", COLORS['error'])


def on_enter_pressed(event):
    """Handle Enter key in message entry"""
    send_message()
    return "break"  # Prevent default behavior


def read_uart_responses():
    """Continuously read UART responses in background thread"""
    global running
    while running:
        try:
            if ser and ser.is_open and connection_status:
                if ser.in_waiting > 0:
                    response = ser.readline().decode('utf-8', errors='ignore').strip()
                    if response:
                        timestamp = time.strftime("%H:%M:%S")
                        # Safely update GUI from thread
                        root.after(0, lambda r=response, t=timestamp:
                        serial_monitor.insert(tk.END, f"[{t}] ⬇️ RECV: {r}\n"))
                        root.after(0, lambda: serial_monitor.see(tk.END))
            time.sleep(0.01)  # Small delay to prevent high CPU usage
        except Exception as e:
            if running:  # Only show error if not shutting down
                root.after(0, lambda: update_status(f"Read error: {str(e)}", COLORS['error']))
            time.sleep(0.1)


def refresh_com_ports():
    """Refresh available COM ports"""
    try:
        com_ports = [port.device for port in serial.tools.list_ports.comports()]
        com_port_combobox.configure(values=com_ports)
        if com_ports:
            com_port_combobox.set(com_ports[0])
            update_status(f"Found {len(com_ports)} COM ports")
        else:
            update_status("No COM ports found", COLORS['warning'])
    except Exception as e:
        update_status(f"Port refresh error: {str(e)}", COLORS['error'])


def clear_monitor():
    """Clear the serial monitor"""
    serial_monitor.delete("1.0", tk.END)
    update_status("Serial monitor cleared")


def on_closing():
    """Handle application exit"""
    global running
    running = False
    disconnect_uart()
    root.destroy()


# Create main window with modern styling
root = ctk.CTk()
root.title("🚀 Modern Lua Script Editor")
root.geometry("1000x700")
root.minsize(1000, 700)

# Configure grid weights for responsive design
root.grid_columnconfigure(0, weight=1)
root.grid_rowconfigure(1, weight=1)

# Modern header frame
header_frame = ctk.CTkFrame(root, height=60, fg_color=COLORS['bg_secondary'])
header_frame.pack(fill="x", padx=0, pady=0)
header_frame.pack_propagate(False)

# Title with modern typography
title_label = ctk.CTkLabel(
    header_frame,
    text="🚀 Lua Script Editor & Serial Monitor",
    font=ctk.CTkFont(size=18, weight="bold"),
    text_color=COLORS['text_primary']
)
title_label.pack(side="left", padx=20, pady=15)

# Connection controls in header
connection_frame = ctk.CTkFrame(header_frame, fg_color="transparent")
connection_frame.pack(side="right", padx=0, pady=0)

com_port_combobox = ctk.CTkComboBox(
    connection_frame,
    state="readonly",
    width=150,
    border_color=COLORS['border'],
    button_color=COLORS['accent']
)
com_port_combobox.pack(side="left", padx=5)

refresh_button = ctk.CTkButton(
    connection_frame,
    text="🔄",
    command=refresh_com_ports,
    width=40,
    fg_color=COLORS['bg_tertiary'],
    hover_color=COLORS['border']
)
refresh_button.pack(side="left", padx=5)

connect_button = ctk.CTkButton(
    connection_frame,
    text="Connect",
    command=toggle_connection,
    width=100,
    fg_color=COLORS['accent'],
    hover_color=COLORS['accent_hover']
)
connect_button.pack(side="left", padx=5)

# Main content area with modern layout
main_frame = ctk.CTkFrame(root, fg_color="transparent")
main_frame.pack(fill="both", expand=True, padx=0, pady=(0))

# Left panel - Code Editor
left_panel = ctk.CTkFrame(main_frame, fg_color=COLORS['bg_secondary'])
left_panel.pack(side="left", fill="both", expand=True, padx=(0))

# Code editor header
editor_header = ctk.CTkFrame(left_panel, height=40, fg_color=COLORS['bg_tertiary'])
editor_header.pack(fill="x", padx=10, pady=10)
editor_header.pack_propagate(False)

code_label = ctk.CTkLabel(
    editor_header,
    text="📝 Lua Code Editor",
    font=ctk.CTkFont(size=14, weight="bold"),
    text_color=COLORS['text_primary']
)
code_label.pack(side="left", padx=15, pady=10)

send_button = ctk.CTkButton(
    editor_header,
    text="⚡ Send Code",
    command=send_code,
    width=120,
    height=32,
    fg_color=COLORS['accent'],
    hover_color=COLORS['accent_hover'],
    font=ctk.CTkFont(weight="bold")
)
send_button.pack(side="right", padx=15, pady=4)

# Modern code editor with syntax highlighting support
code_editor = scrolledtext.ScrolledText(
    left_panel,
    wrap=tk.NONE,
    font=("JetBrains Mono", 11),
    bg=COLORS['bg_primary'],
    fg=COLORS['text_primary'],
    insertbackground="#00d8ff",
    selectbackground="#264f78",
    relief="flat",
    borderwidth=0,
    undo=True,
    maxundo=50
)
code_editor.pack(fill="both", expand=True, padx=10, pady=(0, 10))

# Bind events for syntax highlighting
code_editor.bind('<KeyRelease>', on_code_change)
code_editor.bind('<ButtonRelease-1>', on_code_change)

# Right panel - Serial Monitor
right_panel = ctk.CTkFrame(main_frame, fg_color=COLORS['bg_secondary'])
right_panel.pack(side="right", fill="both", expand=True, padx=(1, 0))

# Monitor header
monitor_header = ctk.CTkFrame(right_panel, height=40, fg_color=COLORS['bg_tertiary'])
monitor_header.pack(fill="x", padx=10, pady=10)
monitor_header.pack_propagate(False)

monitor_label = ctk.CTkLabel(
    monitor_header,
    text="📡 Serial Monitor",
    font=ctk.CTkFont(size=14, weight="bold"),
    text_color=COLORS['text_primary']
)
monitor_label.pack(side="left", padx=15, pady=10)

clear_button = ctk.CTkButton(
    monitor_header,
    text="🗑️ Clear",
    command=clear_monitor,
    width=80,
    height=32,
    fg_color=COLORS['bg_primary'],
    hover_color=COLORS['border']
)
clear_button.pack(side="right", padx=15, pady=4)

# Serial monitor display
serial_monitor = scrolledtext.ScrolledText(
    right_panel,
    wrap=tk.WORD,
    font=("JetBrains Mono", 10),
    bg=COLORS['bg_primary'],
    fg=COLORS['text_primary'],
    insertbackground="#00d8ff",
    selectbackground="#264f78",
    relief="flat",
    borderwidth=0
)
serial_monitor.pack(fill="both", expand=True, padx=10, pady=(0, 5))

# Message input frame
message_frame = ctk.CTkFrame(right_panel, height=40, fg_color=COLORS['bg_tertiary'])
message_frame.pack(fill="x", padx=10, pady=(0, 10))
message_frame.pack_propagate(False)

message_entry = ctk.CTkEntry(
    message_frame,
    placeholder_text="Type message to send...",
    font=("JetBrains Mono", 11),
    border_color=COLORS['border']
)
message_entry.pack(side="left", fill="x", expand=True, padx=(10, 5), pady=8)
message_entry.bind('<Return>', on_enter_pressed)

send_msg_button = ctk.CTkButton(
    message_frame,
    text="📤 Send",
    command=send_message,
    width=80,
    height=28,
    fg_color=COLORS['accent'],
    hover_color=COLORS['accent_hover']
)
send_msg_button.pack(side="right", padx=(5, 10), pady=8)

# Modern status bar
status_bar = ctk.CTkLabel(
    root,
    text="● Ready",
    font=ctk.CTkFont(size=11),
    text_color=COLORS['text_secondary'],
    fg_color=COLORS['bg_secondary'],
    height=30,
    anchor="w"
)
status_bar.pack(side="bottom", fill="x", padx=0, pady=0)

# Initialize
refresh_com_ports()

# Start UART reading thread
uart_thread = threading.Thread(target=read_uart_responses, daemon=True)
uart_thread.start()

# Handle window close
root.protocol("WM_DELETE_WINDOW", on_closing)

# Add some example code with syntax highlighting
example_code = """SEG_TABLE = {
  0x3F, 0x06, 0x5B, 0x4F, 0x66,
  0x6D, 0x7D, 0x07, 0x7F, 0x6F
}

local prev_temp = nil
local alpha = 0.2  -- smoothing factor

function send_command(addr, data)
  i2c_send(addr, {data})
end

function display_temperature(temp)
  if temp < 0 then temp = 0 end
  if temp > 9999 then temp = 9999 end  -- Cap to 4-digit limit

  local value = math.floor(temp + 0.5)  -- No decimal, round to int

  local thousands = math.floor(value / 1000)
  local hundreds  = math.floor((value % 1000) / 100)
  local tens      = math.floor((value % 100) / 10)
  local ones      = value % 10

  send_command(0x34, SEG_TABLE[thousands + 1])
  send_command(0x35, SEG_TABLE[hundreds + 1])
  send_command(0x36, SEG_TABLE[tens + 1])
  send_command(0x37, SEG_TABLE[ones + 1])
end

function handle_message(protocol, data)
  if protocol == UART then
    local raw = data:byte(1)
    if raw and raw > 0 and raw < 255 then
      -- === Send raw value over CAN ===
      send_message(CAN, {raw})

      local v = raw / 255 * 5.0
      local r = 100000 * v / (5.0 - v)
      local ln = math.log(r / 100000)
      local invT = ln / 3950 + 1 / 298.15
      local temp_c = (1 / invT) - 273.15

      if prev_temp == nil then
        prev_temp = temp_c
      else
        prev_temp = alpha * temp_c + (1 - alpha) * prev_temp
      end

      print("Raw:", raw, "Smoothed Temp:", prev_temp)
      display_temperature(prev_temp)
    else
      print("Invalid ADC value")
    end
  end
end

-- Init
delay_ms(100)
send_command(0x24, 0x01)
delay_ms(10)
get(UART)
"""

code_editor.insert("1.0", example_code)

# Apply initial syntax highlighting
apply_syntax_highlighting()

# Run the application
root.mainloop()