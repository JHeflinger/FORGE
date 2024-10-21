from pathlib import Path
import sys
import os
import pickle
import json
import numpy as np
sys.path.insert(0,str(Path(__file__).parent.parent))
sys.path.insert(0,os.path.join(str(Path(__file__).parent.parent),"new"))
# from ..new.timer import Timer
from timer import Timer

# sys.path.insert(0,'../new')


def loadstate(path):
    with open(path, 'rb') as f:
        return pickle.load(f)

def main():
    load_folder = sys.argv[1]
    save_folder = sys.argv[2]

    g_timer = Timer()
    g_timer.start("Loading simulation state...")
    simulation = loadstate(os.path.join(load_folder, ".simstate"))
    g_baked = loadstate(os.path.join(load_folder, ".bakedstate"))
    # g_mass = loadstate(os.path.join(load_folder, ".mass"))
    g_timer.end("Loaded simulation state")
    print(f"Num dataframes: {len(g_baked)}")
    print(f"Num particles: {len(simulation[0])}")

    g_timer.start("Saving simulation to JSON...")
    with open(os.path.join(save_folder, "sim_state.csv"), 'w') as folder:
        json.dump(simulation.tolist(), folder)
    with open(os.path.join(save_folder, "baked_state.csv"), 'w') as folder:
        json.dump(g_baked.tolist(), folder)
    g_timer.end("Saved simulation state to JSON.")

if __name__ == "__main__":
    main()