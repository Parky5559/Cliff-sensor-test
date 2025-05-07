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
            print(f"📥 Received: {line}")  # Always print every line to terminal

            if line.startswith('Min Degree:'):
                try:
                    parts = line.split(',')
                    min_deg = int(parts[0].split()[2])
                    max_deg = int(parts[0].split()[4])
                    ir_cm = float(parts[1].split(':')[1].strip())
                    midpoint = int(parts[2].split(':')[1].strip())
                    width = float(parts[3].split(':')[1].strip())

                    size = 'small' if width < 5 else 'medium' if width < 15 else 'large'

                    degree.append(math.radians(midpoint))
                    rarr.append(ir_cm)
                    sizearr.append(size)
                    min_degrees.append(math.radians(min_deg))
                    max_degrees.append(math.radians(max_deg))
                    path_angles.append(math.radians(midpoint))
                    path_dists.append(ir_cm)
                except Exception as e:
                    print("❌ Parse error:", line)
                    print("‼️ Exception:", e)

receive_thread = threading.Thread(target=receive_data)
receive_thread.start()

# Plot setup
fig = plt.figure()
ax = fig.add_subplot(projection='polar')

def get_color(size):
    return {'small': 'green', 'medium': 'orange', 'large': 'red'}.get(size.lower(), 'gray')

def animate(i):
    ax.clear()
    ax.set_rlabel_position(-22.5)
    ax.set_title("Roomba Radar System", va='bottom')
    ax.set_thetamin(0)
    ax.set_thetamax(180)
    ax.set_rmax(100)

    # Draw Roomba at origin
    ax.scatter(0, 0, color='black', s=100, label='Roomba')

    # Roomba width line (35cm across origin, horizontal)
    roomba_half_width = 17.5  # Half of 35cm
    width_line_angles = [math.radians(0), math.radians(180)]
    width_line_radii = [roomba_half_width, roomba_half_width]
    ax.plot(width_line_angles, width_line_radii, color='Red', linewidth=3.5, alpha=1.0, label='Roomba Width')

    max_dist = 0
    max_angle = 0

    # Draw each object
    for angle, dist, size, a_min, a_max in zip(degree, rarr, sizearr, min_degrees, max_degrees):
        color = get_color(size)

        # Line from Roomba to object
        ax.plot([0, angle], [0, dist], linestyle='-', color='purple', linewidth=2.5, alpha=0.6)

        # Object marker
        ax.scatter(angle, dist, color=color, s=80, alpha=0.8)

        # Label angle and distance
        angle_deg = round(math.degrees(angle))
        dist_txt = round(dist, 2)
        ax.text(angle, dist + 5, f"{angle_deg}°\n{dist_txt}cm\n{size}", fontsize=6, ha='center')

        # Width line across min-max angles
        width_arc = np.linspace(a_min, a_max, 30)
        width_radii = [dist] * len(width_arc)
        ax.plot(width_arc, width_radii, color='gray', linewidth=2.0, alpha=0.4)

        # Update max distance
        if dist > max_dist:
            max_dist = dist
            max_angle = angle

    # Max distance line
    if max_dist > 0:
        ax.plot([0, max_angle], [0, max_dist], linestyle='--', color='cyan', linewidth=2, label='Max Distance')

    # Motion path
    if len(path_angles) > 1:
        ax.plot(path_angles, path_dists, linestyle=':', color='blue', linewidth=1, alpha=0.5)

    ax.legend(loc='upper right', fontsize=6)

# Animate and run
ani = animation.FuncAnimation(fig, animate, interval=500)

plt.show()
keyboard.wait()
receive_thread.join()
