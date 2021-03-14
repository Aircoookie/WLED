import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import interactive, style
import numpy as np
import random
import serial

# Initialize serial port
ser = serial.Serial()
ser.port = 'dev/cu.SLAB_USBtoUART'  # Conected serial port
ser.baudrate = 115200
ser.timeout = 50
ser.open()
if ser.is_open:
    print("\nAll right, serial port now open. Configuration:\n")
    print(ser, "\n")

# Create figure for plotting
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
xs = []   # Store trials here (n)
ys = []   # Store relative frequency here
rs = []   # For theoretical probablility


def animate(i, xs, ys):
    """ This function is called periodically from FuncAnimation """
    line = ser.readline()   # ascii
    line_as_list = line.split(b',')
    i = int(line_as_list[0])
    relProb = line_as_list[1]
    relProb_as_list = relProb.split(b'\n')
    relProb_float = float(relProb_as_list[0])

    # Add x and y to lists
    xs.append(i)
    ys.append(relProb_float)
    rs.append(0.5)

    # Limit x and y lists to 20 items
    # xs = xs[-20]
    # ys = ys[-20]

    # Draw x and y lists
    ax.clear()
    ax.plot(xs, ys, label="Experimental Probability")
    ax.plot(xs, rs, label="Theoretical Probability")

    # Format plot
    plt.xticks(rotation=45, ha='right')
    plt.subplots_adjust(bottom=0.30)
    plt.title('This is how I roll...')
    plt.ylabel('Relative Frequency')
    plt.legend()
    plt.axis([1, None, 0, 1.1])   # Use for arbitrary number of trials
    # plt.axis([1, 100, 0, 1.1])   # Use for 100 trial demo


# Set up plot to call animate() function periodically
ani = animation.FuncAnimation(fig, animate, fargs=(xs, ys), interval=1000)
plt.show()
