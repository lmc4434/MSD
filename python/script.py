import tkinter as tk
from tkinter import ttk, messagebox
import serial
import threading
import math
import time

class DataVariable:
    def __init__(self, initial_value = 0.0):
        self.current_value = initial_value
        self.memory_value = 0.0

# Global Variables
voltage_var = DataVariable()
battery_var = DataVariable()
tilt_angle_var = DataVariable()
tilt_angle_display_var = DataVariable()
panel_open = DataVariable(initial_value=0.0)
mode = DataVariable(initial_value=0.0)
power_generation = DataVariable()
tiltfinished = DataVariable()
sent = 0
in_admin_screen = False
data_initialized = False




root = tk.Tk()
root.title("Psyche 2 Power Supply Control")
root.geometry("900x750")
root.configure(bg="#1D1F21")

for i in range(10):
    root.grid_rowconfigure(i, weight=1)
for i in range(3):
    root.grid_columnconfigure(i, weight=1)

ser = serial.Serial('COM3', 9600, timeout=1)

slider_update_timer = None

def send_value(id: str, value: float):
    message = f"{id}:{value:.2f}\r"
    ser.write(message.encode())
    print(f"Sending: {message}")
    update_terminal(f"Sending: {message}")

def on_slider_change(val):
    global slider_update_timer
    if slider_update_timer:
        root.after_cancel(slider_update_timer)
    slider_update_timer = root.after(500, send_value, "TA", float(val))
    tiltfinished.current_value = 1


def receive_data():
    data_buffer = ""
    global sent

    while True:
        if ser.in_waiting > 0:
            data = ser.read()
            data_buffer += data.decode('utf-8', errors='ignore')

            if "\r" in data_buffer:
                complete_message = data_buffer.strip()
                data_buffer = ""

                if ":" in complete_message:
                    try:
                        id, value = complete_message.split(":")
                        value = float(value)

                        if id == "VV":
                            voltage_var.current_value = value
                        elif id == "BP":
                            battery_var.current_value = value
                        elif id == "TA":
                            tilt_angle_var.current_value = value
                            tilt_angle_display_var.current_value = value
                            update_terminal(f"Tilt Angle Updated: {value:.2f}°")
                        global data_initialized
                        if not data_initialized:
                            data_initialized = True
                            root.after(100, lambda: tilt_slider.set(tilt_angle_var.current_value))

                        elif id == "CM":
                            mode.current_value = value
                            update_terminal(f"Mode Changed: {'Autonomous' if mode.current_value else 'Manual'}")
                        elif id == "PG":
                            power_generation.current_value = value
                        elif id == "PO":
                            print(panel_open.current_value)
                        elif id == "ST":
                            print("Solar Tracking Run")
                        elif id == "CD":
                            print("Clearing Dust Run")
                        elif id == "FI":
                            tiltfinished.current_value = 0

                    except ValueError:
                        print("Invalid value format in message")
                    except Exception as e:
                        print(f"Error processing message: {e}")

def blow_off_dust():
    update_terminal("Blowing off dust with compressed air...")
    send_value("CD", 1.0)

def toggle_panels():
    panel_open.current_value = 1.0 if panel_open.current_value == 0.0 else 0.0
    unfold_button.config(text="Close Panels" if panel_open.current_value else "Open Panels")
    update_terminal("Panels Opened." if panel_open.current_value else "Panels Closed.")
    send_value("PO", 180)

def toggle_mode():
    mode.current_value = 1.0 if mode.current_value == 0.0 else 0.0
    mode_button.config(
        text="Switch to Manual Mode" if mode.current_value else "Switch to Autonomous Mode"
    )
    send_value("CM", mode.current_value)
    update_terminal(f"Switched to {'Autonomous' if mode.current_value else 'Manual'} Mode.")

def update_gui():
    if in_admin_screen:
        return
    voltage_display.config(text=f"{voltage_var.current_value:.2f} V")
    battery_display.config(text=f"{battery_var.current_value:.2f} %")
    power_display.config(text=f"{voltage_var.current_value * 2.0:.2f} W")
    tilt_angle_display_label.config(text=f"{tilt_angle_display_var.current_value:.2f}°")
    update_tilt_indicator(tilt_angle_display_var.current_value)


    if tiltfinished.current_value == 1:
        tilt_slider.state(["disabled"])
    else:
        tilt_slider.state(["!disabled"])

    root.after(1000, update_gui)

def update_tilt_indicator(tilt_value):
    tilt_angle_display_canvas.delete("all")
    line_length = 200
    center_x = 200
    center_y = 200
    angle_radians = math.radians(tilt_value)

    x1 = center_x - line_length / 2
    y1 = center_y
    x2 = center_x + line_length / 2
    y2 = center_y

    cos_angle = math.cos(angle_radians)
    sin_angle = math.sin(angle_radians)

    x1_rot = center_x + (x1 - center_x) * cos_angle - (y1 - center_y) * sin_angle
    y1_rot = center_y + (x1 - center_x) * sin_angle + (y1 - center_y) * cos_angle
    x2_rot = center_x + (x2 - center_x) * cos_angle - (y2 - center_y) * sin_angle
    y2_rot = center_y + (x2 - center_x) * sin_angle + (y2 - center_y) * cos_angle

    tilt_angle_display_canvas.create_line(x1_rot, y1_rot, x2_rot, y2_rot, fill="cyan", width=3)
    rover_x = 200
    rover_y = 300
    tilt_angle_display_canvas.create_line(rover_x, rover_y, center_x, center_y, fill="white", width=2)
    tilt_angle_display_canvas.create_line(150, rover_y, 250, rover_y, fill="white", width=3)
    tilt_angle_display_canvas.create_line(170, rover_y, 170, rover_y + 20, fill="white", width=3)
    tilt_angle_display_canvas.create_line(230, rover_y, 230, rover_y + 20, fill="white", width=3)
    tilt_angle_display_canvas.create_oval(160, rover_y + 10, 180, rover_y + 30, fill="white")
    tilt_angle_display_canvas.create_oval(220, rover_y + 10, 240, rover_y + 30, fill="white")

def run_solar_tracking():
    update_terminal("Solar tracking activated...")
    send_value("ST", 1.0)

def update_terminal(text):
    try:
        terminal_output.insert(tk.END, text + "\n")
        terminal_output.yview(tk.END)
    except:
        print(f"[Admin Log] {text}")

def show_admin_login():
    def validate_password():
        if password_entry.get() == "password":
            login_window.destroy()
            show_admin_screen()
        else:
            messagebox.showerror("Error", "Incorrect Password")

    login_window = tk.Toplevel(root)
    login_window.title("Admin Login")
    login_window.geometry("300x150")
    login_window.configure(bg="#1D1F21")

    tk.Label(login_window, text="Enter Admin Password:", fg="white", bg="#1D1F21", font=("Arial", 12)).pack(pady=10)
    password_entry = tk.Entry(login_window, show="*", width=25)
    password_entry.pack(pady=5)
    tk.Button(login_window, text="Submit", command=validate_password).pack(pady=10)


def show_admin_screen():
    global in_admin_screen
    in_admin_screen = True

    for widget in root.winfo_children():
        widget.destroy()

    tk.Label(root, text="ADMIN PANEL", font=("Arial", 20, "bold"), fg="red", bg="#1D1F21").pack(pady=20)

    def graceful_shutdown():
        tilt_angle_display_var.current_value = 0
        tilt_angle_var.current_value = 0
        panel_open.current_value = 0

        send_value("PO", 0)
        root.after(100, lambda: send_value("TA", 0))
        update_terminal("Graceful shutdown initiated...")

    def manual_tilt_left():
        new_angle = max(-22, tilt_angle_var.current_value - 1)
        tilt_angle_var.current_value = new_angle
        tilt_angle_display_var.current_value = new_angle
        send_value("TA", new_angle)

    def manual_tilt_right():
        new_angle = min(22, tilt_angle_var.current_value + 1)
        tilt_angle_var.current_value = new_angle
        tilt_angle_display_var.current_value = new_angle
        send_value("TA", new_angle)

    def set_panel_open(open_state):
        panel_open.current_value = 1.0 if open_state else 0.0
        send_value("PO", 180 if open_state else 0)
        update_terminal(f"Panels {'Opened' if open_state else 'Closed'} via Jog")

    buttons = [
        ("Graceful Shutdown", graceful_shutdown),
        ("Tilt Left ⬅️", manual_tilt_left),
        ("Tilt Right ➡️", manual_tilt_right),
        ("Jog Panels Open", lambda: [set_panel_open(True)]),
        ("Jog Panels Closed", lambda: [set_panel_open(False)]),
        ("Return to Main Screen", lambda: [generate_window(), update_gui()]),
    ]


    for text, action in buttons:
        tk.Button(root, text=text, width=25, height=2, font=("Arial", 14), command=action).pack(pady=10)

def generate_window():
    global in_admin_screen
    in_admin_screen = False
    global voltage_display, battery_display, tilt_angle_display_label, unfold_button, mode_button, power_display
    global tilt_angle_display_canvas, terminal_output, tilt_slider, tiltfinished

    for widget in root.winfo_children():
        widget.destroy()



    ttk.Label(root, text="Solar Panel Voltage (V):", foreground="white", background="#1D1F21").grid(row=0, column=0, padx=20, pady=10, sticky="nsew")
    voltage_display = ttk.Label(root, text=f"{voltage_var.current_value:.2f} V", font=("Arial", 16), foreground="cyan", background="#1D1F21")
    voltage_display.grid(row=0, column=1, padx=10, pady=10, sticky="nsew")

    ttk.Label(root, text="Battery Percentage:", foreground="white", background="#1D1F21").grid(row=1, column=0, padx=20, pady=10, sticky="nsew")
    battery_display = ttk.Label(root, text=f"{battery_var.current_value:.2f} %", font=("Arial", 16), foreground="cyan", background="#1D1F21")
    battery_display.grid(row=1, column=1, padx=10, pady=10, sticky="nsew")

    ttk.Label(root, text="Power Generation (W):", foreground="white", background="#1D1F21").grid(row=2, column=0, padx=20, pady=10, sticky="nsew")
    power_display = ttk.Label(root, text=f"{voltage_var.current_value * 2.0:.2f} W", font=("Arial", 16), foreground="cyan", background="#1D1F21")
    power_display.grid(row=2, column=1, padx=10, pady=10, sticky="nsew")

    ttk.Label(root, text="Tilt Angle (degrees):", foreground="white", background="#1D1F21").grid(row=3, column=0, padx=20, pady=10, sticky="nsew")
    tilt_angle_display_label = ttk.Label(root, text=f"{tilt_angle_display_var.current_value:.2f}", font=("Arial", 16), foreground="cyan", background="#1D1F21")
    tilt_angle_display_label.grid(row=3, column=1, padx=10, pady=10, sticky="nsew")

    global tilt_angle_display_canvas
    tilt_angle_display_canvas = tk.Canvas(root, width=400, height=400, bg="#1D1F21", bd=0, highlightthickness=0)
    tilt_angle_display_canvas.grid(row=0, column=2, rowspan=4, padx=20, pady=20, sticky="nsew")

    button_style = ttk.Style()
    button_style.configure("TButton", font=("Arial", 14, "bold"))

    ttk.Button(root, text="Activate Compressed Air", command=blow_off_dust, style="TButton").grid(row=5, column=0, columnspan=2, padx=20, pady=10, sticky="nsew")
    unfold_button = ttk.Button(root, text="Open Panels" if not panel_open.current_value else "Close Panels", command=toggle_panels)
    unfold_button.grid(row=5, column=2, padx=20, pady=10, sticky="nsew")

    mode_button = ttk.Button(root, text="Switch to Manual Mode" if mode.current_value else "Switch to Autonomous Mode", command=toggle_mode, style="TButton")
    mode_button.grid(row=6, column=0, columnspan=2, padx=20, pady=10, sticky="nsew")

    ttk.Button(root, text="Run Solar Tracking", command=run_solar_tracking, style="TButton").grid(row=6, column=2, padx=20, pady=10, sticky="nsew")

    tilt_slider = ttk.Scale(root, from_=-22, to_=22, orient="horizontal")
    tilt_slider.grid(row=9, column=0, columnspan=3, padx=20, pady=10, sticky="nsew")

    # Set the slider visually to match the tilt angle variable without triggering send
    tilt_slider.set(tilt_angle_var.current_value)

    # Delay binding the command to avoid triggering on startup
    root.after(100, lambda: tilt_slider.configure(command=on_slider_change))


    ttk.Button(root, text="ADMIN", command=show_admin_login, style="TButton").grid(row=0, column=2, padx=10, pady=10, sticky="ne")

    terminal_output = tk.Text(root, height=5, width=90, bg="#1D1F21", fg="cyan", font=("Courier", 10), bd=0)
    terminal_output.grid(row=10, column=0, columnspan=3, padx=20, pady=10, sticky="nsew")
    terminal_output.insert(tk.END, "Terminal Output:\n")

    update_tilt_indicator(tilt_angle_display_var.current_value)

def start_receive_thread():
    receive_thread = threading.Thread(target=receive_data, daemon=True)
    receive_thread.start()

def start_send_thread(id: str, value: float):
    send_thread = threading.Thread(target=send_value, args=(id, value), daemon=True)
    send_thread.start()

if __name__ == "__main__":
    generate_window()
    start_receive_thread()
    update_gui()
    send_value("SU", 0.0)
    root.mainloop()
