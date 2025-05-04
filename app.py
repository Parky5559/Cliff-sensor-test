import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import socket
import keyboard
import threading
import math

# Connection Info
HOST = '192.168.1.1'
PORT = 288

# Socket Setup
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((HOST, PORT))

# Data Lists
degree = []
rarr = []
sizearr = []
min_degrees = []
max_degrees = []

# Motion path
path_angles = []
path_dists = []

# Send keys to robot
def on_key_press(event):
    if event.name == 'esc':
        sock.close()
        print('Program ended.')
        exit()
    elif event.name == 'm':
        degree.clear()
        rarr.clear()
        sizearr.clear()
        min_degrees.clear()
        max_degrees.clear()
        path_angles.clear()
        path_dists.clear()
        ax.clear()
        sock.send(b'm')
    else:
        sock.send(event.name.encode())

keyboard.on_press(on_key_press)

# Receive data
def receive_data():
    while True:
        data = sock.recv(2048)
        if not data:
            break
        lines = data.decode(errors="ignore").strip().split('\n')
        for line in lines:
            if line.startswith('Min Degree:'):
                print("Received:", line)  # Debug print
                try:
                    parts = line.split(',')

                    min_part = parts[0].split()
                    min_deg = int(min_part[2])
                    max_deg = int(min_part[4])
                    ir_cm = float(parts[1].split(':')[1].strip())
                    midpoint = int(parts[2].split(':')[1].strip())
                    width = float(parts[3].split(':')[1].strip())

                    if width < 5:
                        size = 'small'
                    elif width < 15:
                        size = 'medium'
                    else:
                        size = 'large'

                    degree.append(math.radians(midpoint))
                    rarr.append(int(ir_cm))
                    sizearr.append(size)

                    min_degrees.append(math.radians(min_deg))
                    max_degrees.append(math.radians(max_deg))

                    path_angles.append(math.radians(midpoint))
                    path_dists.append(ir_cm)
                except Exception as e:
                    print("\u274c Parse error:", line)
                    print("\u2757 Exception:", e)
            else:
                print(line)

receive_thread = threading.Thread(target=receive_data)
receive_thread.start()

# Plot setup
fig = plt.figure()
ax = fig.add_subplot(projection='polar')

def get_color(size):
    if size.lower() == 'small':
        return 'green'
    elif size.lower() == 'medium':
        return 'orange'
    else:
        return 'red'

def animate(i):
    ax.clear()
    ax.set_rlabel_position(-22.5)
    ax.set_title("R2D2 Radar System", va='bottom')
    ax.set_thetamin(0)
    ax.set_thetamax(180)
    ax.set_rmax(70)

    # Draw object angular outlines2
    for angle_min, angle_max, dist in zip(min_degrees, max_degrees, rarr):
        arc_angles = np.linspace(angle_min, angle_max, 50)
        arc_radii = [dist] * len(arc_angles)
        ax.plot(arc_angles, arc_radii, color='gray', linewidth=1.5, alpha=0.6)

    # Plot midpoints with size color
    for angle, dist, size in zip(degree, rarr, sizearr):
        ax.scatter(angle, dist, color=get_color(size), s=80, alpha=0.7)
        angle_deg = round(math.degrees(angle))
        ax.text(angle, dist + 10, f"{angle_deg}\u00b0\n{dist}cm", fontsize=6, ha='center')

    # Plot motion path
    if len(path_angles) > 1:
        ax.plot(path_angles, path_dists, linestyle=':', color='blue', linewidth=1, alpha=0.4)

ani = animation.FuncAnimation(fig, animate, interval=500)

plt.show()
keyboard.wait()
receive_thread.join()
