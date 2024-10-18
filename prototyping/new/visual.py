import matplotlib.pyplot as plt
import numpy as np
from serial import *
import sys

def visualize_test(path):
    results = loadstate(path)
    fig, axs = plt.subplots(2, 2, figsize=(10, 8))

    def get_color(value):
        value = max(0, min(1, value))
        cmap = plt.get_cmap('hsv')
        return cmap(value)[:3]

    # accuracy
    for i, dataset in enumerate(results["accuracy"]):
        yvals = np.array(dataset[1])
        xvals = np.arange(len(yvals))
        axs[0, 0].plot(xvals, yvals, label=dataset[0], color=get_color(i/len(results["accuracy"])))
    axs[0, 0].set_title('Accuracy')
    axs[0, 0].set_xlabel('time')
    axs[0, 0].set_ylabel('error')
    axs[0, 0].legend()

    # total energy
    for i, dataset in enumerate(results["total energy"]):
        yvals = np.array(dataset[1])
        xvals = np.arange(len(yvals))
        axs[0, 1].plot(xvals, yvals, label=dataset[0], color=get_color(i/len(results["total energy"])))
    axs[0, 1].set_title('Total Energy')
    axs[0, 1].set_xlabel('time')
    axs[0, 1].set_ylabel('error')
    axs[0, 1].legend()

    # kinetic energy
    for i, dataset in enumerate(results["kinetic energy"]):
        yvals = np.array(dataset[1])
        xvals = np.arange(len(yvals))
        axs[1, 0].plot(xvals, yvals, label=dataset[0], color=get_color(i/len(results["kinetic energy"])))
    axs[1, 0].set_title('Kinetic Energy')
    axs[1, 0].set_xlabel('time')
    axs[1, 0].set_ylabel('error')
    axs[1, 0].legend()

    # potential energy
    for i, dataset in enumerate(results["potential energy"]):
        yvals = np.array(dataset[1])
        xvals = np.arange(len(yvals))
        axs[1, 1].plot(xvals, yvals, label=dataset[0], color=get_color(i/len(results["potential energy"])))
    axs[1, 1].set_title('Potential Energy')
    axs[1, 1].set_xlabel('time')
    axs[1, 1].set_ylabel('error')
    axs[1, 1].legend()

    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    if len(sys.argv) != 3 or (sys.argv[1] != '-t' and sys.argv[1] != '-v'):
        print("Invalid usage. First argument should be either -t or -s to specify test or simulation, second argument should be the path")
        exit()
    if sys.argv[1] == '-t':
        visualize_test(sys.argv[2])