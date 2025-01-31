import tkinter as tk
from tkinter import ttk
from PIL import Image, ImageTk
import serial
import threading
import random
import time


class DataVariable:
    def __init__(self, initial_value=0.0):
        self.current_value = initial_value
        self.memory_value = 0.0

# Global Variables
voltage_var = DataVariable()
battery_var = DataVariable()
tilt_angle_var = DataVariable()
tilt_angle_display_var = DataVariable()
panel_open = DataVariable(initial_value=0.0)
mode = DataVariable(initial_value=0.0)


root = tk.Tk()
root.title("Psyche 2 Power Supply Control")
root.geometry("800x600")
for i in range(9):
    root.grid_rowconfigure(i, weight=1)
root.grid_columnconfigure(0, weight=1)
root.grid_columnconfigure(1, weight=1)
root.grid_columnconfigure(2, weight=1)

ser = serial.Serial('COM3', 9600, timeout=1)


slider_update_timer = None

def send_tilt_angle(val):
    tilt_value = int(float(val))
    message = f"TA:{tilt_value:.2f}\r" 

    ser.write(message.encode()) 


    tilt_angle_var.current_value = tilt_value
    tilt_angle_display_var.current_value = tilt_value 
    tilt_angle_display_label.config(text=f"{tilt_angle_var.current_value:.1f}")

    print(f"Sending: {message}")

def on_slider_change(val):
    global slider_update_timer

    if slider_update_timer:
        root.after_cancel(slider_update_timer)
    slider_update_timer = root.after(500, send_tilt_angle, val)



def receive_data():
    data_buffer = ""  # Initialize an empty buffer to store the incoming data

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
                            print("Updating Tilt Angle")
                            tilt_angle_var.current_value = value
                            tilt_angle_display_var.current_value = value
                            print(id, value)

                        elif id == "PO":
                            panel_open.current_value = value
                        elif id == "CM":
                            mode.current_value = value
                        else:
                            print(f"Unknown ID: {id}")



                    except ValueError:
                        print("Invalid value format in message")
                    except Exception as e:
                        print(f"Error processing message: {e}")

def blow_off_dust():

    print("Blowing off dust with compressed air...")

def toggle_panels():
    panel_open.current_value = 1.0 if panel_open.current_value == 0.0 else 0.0
    unfold_button.config(text="Close Panels" if panel_open.current_value else "Open Panels")
    print("Panels Opened." if panel_open.current_value else "Panels Closed.")

def toggle_mode():
    mode.current_value = 1.0 if mode.current_value == 0.0 else 0.0
    mode_button.config(
        text="Switch to Manual Mode" if mode.current_value else "Switch to Autonomous Mode"
    )
    print("Switched to Autonomous Mode." if mode.current_value else "Switched to Manual Mode.")

def update_gui():
    voltage_display.config(text=f"{voltage_var.current_value:.2f} V")
    battery_display.config(text=f"{battery_var.current_value:.2f} %")
    power_display.config(text=f"{voltage_var.current_value * 2.0:.2f} W")
    root.after(1000, update_gui)

def run_solar_tracking():
    print("Solar tracking activated...")

def generate_window():
    global voltage_display, battery_display, tilt_angle_display_label, unfold_button, mode_button, power_display

    # NASA and Psyche logos
    #nasa_img = Image.open("NASA.PNG").resize((100, 100), Image.Resampling.LANCZOS)
    #nasa_photo = ImageTk.PhotoImage(nasa_img)
    #root.nasa_photo = nasa_photo
    #ttk.Label(root, image=nasa_photo).grid(row=0, column=0, padx=10, pady=10, sticky="nw")

    #psyche_img = Image.open("Psyche.PNG").resize((150, 100), Image.Resampling.LANCZOS)
    #psyche_photo = ImageTk.PhotoImage(psyche_img)
    #root.psyche_photo = psyche_photo
    #ttk.Label(root, image=psyche_photo).grid(row=0, column=2, padx=10, pady=10, sticky="ne")

    ttk.Label(root, text="Psyche 2 Power Supply Control", font=("Arial", 12, "bold")).grid(
        row=0, column=1, padx=10, pady=10, sticky="nsew"
    )

    # Voltage display
    ttk.Label(root, text="Solar Panel Voltage (V):").grid(row=1, column=0, padx=10, pady=10, sticky="nsew")
    voltage_display = ttk.Label(root, text=f"{voltage_var.current_value:.2f} V")
    voltage_display.grid(row=1, column=1, padx=10, pady=10, sticky="nsew")

    # Tilt angle
    ttk.Label(root, text="Tilt Angle (degrees):").grid(row=2, column=0, padx=10, pady=10, sticky="nsew")
    tilt_angle_display_label = ttk.Label(root, text=f"{tilt_angle_display_var.current_value}")
    tilt_angle_display_label.grid(row=2, column=1, padx=10, pady=10, sticky="nsew")

    # Tilt slider (adjust to work with integers)
    tilt_slider = ttk.Scale(
        root,
        from_=-180,
        to_=180,
        orient="horizontal",
        command=on_slider_change,  # Use the new function
    )
    tilt_slider.grid(row=3, column=1, padx=10, pady=10, sticky="nsew")
    ttk.Label(root, text="-180").grid(row=3, column=0, padx=10, pady=10, sticky="w")
    ttk.Label(root, text="180").grid(row=3, column=2, padx=10, pady=10, sticky="e")

    # Buttons
    ttk.Button(root, text="Activate Compressed Air", command=blow_off_dust).grid(
        row=4, column=0, columnspan=3, padx=10, pady=10, sticky="nsew"
    )
    unfold_button = ttk.Button(
        root, text="Open Panels" if not panel_open.current_value else "Close Panels", command=toggle_panels
    )
    unfold_button.grid(row=5, column=0, columnspan=3, padx=10, pady=10, sticky="nsew")
    mode_button = ttk.Button(
        root,
        text="Switch to Manual Mode" if mode.current_value else "Switch to Autonomous Mode",
        command=toggle_mode,
    )
    mode_button.grid(row=6, column=0, columnspan=3, padx=10, pady=10, sticky="nsew")

    # Power generation display
    ttk.Label(root, text="Power Generation (W):").grid(row=7, column=0, padx=10, pady=10, sticky="nsew")
    power_display = ttk.Label(root, text=f"{voltage_var.current_value * 2.0:.2f} W")
    power_display.grid(row=7, column=1, padx=10, pady=10, sticky="nsew")

    # Battery display
    ttk.Label(root, text="Battery Percentage:").grid(row=8, column=0, padx=10, pady=10, sticky="nsew")
    battery_display = ttk.Label(root, text=f"{battery_var.current_value:.2f} %")
    battery_display.grid(row=8, column=1, padx=10, pady=10, sticky="nsew")

    # Solar tracking button
    ttk.Button(root, text="Run Solar Tracking", command=run_solar_tracking).grid(
        row=9, column=0, columnspan=3, padx=10, pady=10, sticky="nsew"
    )

def start_receive_thread():
    receive_thread = threading.Thread(target=receive_data, daemon=True)
    receive_thread.start()



if __name__ == "__main__":
    generate_window()
    start_receive_thread()
    update_gui()
    root.mainloop()
