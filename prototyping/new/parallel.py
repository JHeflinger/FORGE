from serial import *
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import animation, colors, colormaps
from matplotlib.collections import LineCollection
from scipy.constants import G
from timer import *
import threading
from tqdm import tqdm
import os

# Initialize particles and their states
EPS = 1e-12 # epsilon for numerical stability
g_timer = Timer()
g_config = globals()
g_numworkers = g_config["workers"]
g_numparticles = len(g_config["particles"])
g_default_jobsize = int(g_numparticles/g_numworkers)
g_lock = threading.Lock()
if g_numworkers <= 0:
    print("There must be at least 1 worker to simulate")
    exit()
g_jobs = []
for i in range(g_numworkers):
    joblen = g_default_jobsize
    if i == g_numworkers - 1:
        joblen += len(g_config["particles"]) % g_numworkers
    job = {
        "state" : np.zeros((joblen, 4)),
        "mass" : np.zeros(joblen),
        "vmass" : np.zeros(joblen).reshape(-1, 1)
    }
    for j in range(joblen):
        job["mass"][j] = g_config["particles"][(i*g_default_jobsize) + j]["mass"]
        job["state"][j][0] = g_config["particles"][(i*g_default_jobsize) + j]["position"][0]
        job["state"][j][1] = g_config["particles"][(i*g_default_jobsize) + j]["position"][1]
        job["state"][j][2] = g_config["particles"][(i*g_default_jobsize) + j]["velocity"][0]
        job["state"][j][3] = g_config["particles"][(i*g_default_jobsize) + j]["velocity"][1]
    job["vmass"] = job["mass"].reshape(-1, 1)
    g_jobs.append(job)
g_forcematrix = np.zeros((g_numparticles, g_numparticles, 2))
g_mass = np.zeros(g_numparticles)
for i in range(g_numparticles):
    g_mass[i] = g_config["particles"][i]["mass"]
g_vec_mass = g_mass.reshape(-1, 1)
print(f"[{Yellow}WARN{Reset}] Only using leapfrog for naive distributed method")

def print_forcematrix():
    str = ""
    for y in range(g_numparticles):
        for x in range(g_numparticles):
            str += "┌────────────┐"
        str += "\n"
        for x in range(g_numparticles):
            str += f"│{g_forcematrix[x][y][0]:.3f}, {g_forcematrix[x][y][1]:.3f}│"
        str += "\n"
        for x in range(g_numparticles):
            str += "└────────────┘"
        str += "\n"
    print(str)


def simstep_one(jobid):
    for i in range(len(g_jobs[jobid]["state"])):
        for j in range(g_numparticles):
            g_jobs[jobid]["state"][i][2] += g_forcematrix[jobid*g_default_jobsize + i][j][0] * g_config["timestep"]/2.0
            g_jobs[jobid]["state"][i][3] += g_forcematrix[jobid*g_default_jobsize + i][j][1] * g_config["timestep"]/2.0
        g_jobs[jobid]["state"][i][0] += g_jobs[jobid]["state"][i][2] * g_config["timestep"]
        g_jobs[jobid]["state"][i][1] += g_jobs[jobid]["state"][i][3] * g_config["timestep"]

def simstep_two(jobid):
    for i in range(g_numworkers):
        if i != jobid:
            for x in range(len(g_jobs[i]["state"])):
                for y in range(len(g_jobs[jobid]["state"])):
                    #print(f"{jobid*g_default_jobsize + x}, {i*g_default_jobsize + y}")
                    calculate_acceleration(jobid*g_default_jobsize + x, i*g_default_jobsize + y)
    for x in range(len(g_jobs[jobid]["state"])):
        for y in range(len(g_jobs[jobid]["state"])):
            if x != y:
                #print(f"um {jobid*g_default_jobsize + x}, {jobid*g_default_jobsize + y}")
                calculate_acceleration(jobid*g_default_jobsize + x, jobid*g_default_jobsize + y)
    return

def simstep_three(jobid):
    for i in range(len(g_jobs[jobid]["state"])):
        for j in range(g_numparticles):
            g_jobs[jobid]["state"][i][2] += g_forcematrix[jobid*g_default_jobsize + i][j][0] * g_config["timestep"]/2.0
            g_jobs[jobid]["state"][i][3] += g_forcematrix[jobid*g_default_jobsize + i][j][1] * g_config["timestep"]/2.0
    return

def launch_step1():
    threads = []
    for i in range(g_numworkers):
        threads.append(threading.Thread(target=simstep_one(i)))
        threads[-1].start()
    for thread in threads:
        thread.join()

def launch_step2():
    threads = []
    global g_forcematrix
    g_forcematrix = np.zeros((g_numparticles, g_numparticles, 2))
    for i in range(g_numworkers):
        threads.append(threading.Thread(target=simstep_two(i)))
        threads[-1].start()
    for thread in threads:
        thread.join()

def launch_step3():
    threads = []
    for i in range(g_numworkers):
        threads.append(threading.Thread(target=simstep_three(i)))
        threads[-1].start()
    for thread in threads:
        thread.join()

def leapfrog_step():
    launch_step1()
    launch_step2()
    launch_step3()
    state = np.zeros((g_numparticles, 4))
    for i in range(g_numworkers):
        for j in range(len(g_jobs[i]["state"])):
            state[(i*g_default_jobsize) + j] = g_jobs[i]["state"][j]
    return state

def calculate_acceleration(p1, p2):
    global g_forcematrix
    global g_lock
    with g_lock:
        if p1 == p2 or g_forcematrix[p1][p2][0] != 0 or g_forcematrix[p1][p2][1] != 0:
            return
    if p1 >= g_default_jobsize*g_numworkers:
        p1_pos = g_jobs[g_numworkers - 1]["state"][p1 - g_default_jobsize*g_numworkers]
        p1_mass = g_jobs[g_numworkers - 1]["mass"][p1 - g_default_jobsize*g_numworkers]
    else:
        p1_pos = g_jobs[int(p1/g_default_jobsize)]["state"][p1 % g_default_jobsize]
        p1_mass = g_jobs[int(p1/g_default_jobsize)]["mass"][p1 % g_default_jobsize]
    if p2 >= g_default_jobsize*g_numworkers:
        p2_pos = g_jobs[g_numworkers - 1]["state"][p2 - g_default_jobsize*g_numworkers]
        p2_mass = g_jobs[g_numworkers - 1]["mass"][p2 - g_default_jobsize*g_numworkers]
    else:
        p2_pos = g_jobs[int(p2/g_default_jobsize)]["state"][p2 % g_default_jobsize]
        p2_mass = g_jobs[int(p2/g_default_jobsize)]["mass"][p2 % g_default_jobsize]
    dx = p2_pos[0] - p1_pos[0]
    dy = p2_pos[1] - p1_pos[1]
    inv_r3 = (dx**2 + dy**2 + g_config["softening"]**2)**(-1.5)
    p1_ax = G * (dx * inv_r3) * p2_mass
    p1_ay = G * (dy * inv_r3) * p2_mass
    p2_ax = -1.0 * p1_ax * p1_mass / p2_mass
    p2_ay = -1.0 * p1_ay * p1_mass / p2_mass
    with g_lock:
        g_forcematrix[p1][p2][0] = p1_ax
        g_forcematrix[p1][p2][1] = p1_ay
        g_forcematrix[p2][p1][0] = p2_ax
        g_forcematrix[p2][p1][1] = p2_ay

def simulate_leapfrog():
    # set initial state
    state = np.zeros((g_numparticles, 4))
    for i in range(g_numworkers):
        for j in range(len(g_jobs[i]["state"])):
            state[(i*g_default_jobsize) + j] = g_jobs[i]["state"][j]
    simulation = [state]
    
    # calculate initial force grid
    launch_step2()

    # simulate
    steps = int(g_config["duration"] / g_config["timestep"])
    progress = tqdm(total = steps)
    for _ in range(steps):
        state = leapfrog_step()
        simulation.append(np.copy(state))
        progress.update(1)
    progress.close()
    return np.array(simulation)

# Do simulation
g_timer.start(f"Starting simulation on {g_numworkers} worker(s)")
simulation = simulate_leapfrog()
g_timer.end("Finished simulation")

# Field calculation
def g_field(cstate):
    x = np.linspace(-g_config["bounds"], g_config["bounds"], g_config["density"])
    y = np.linspace(-g_config["bounds"], g_config["bounds"], g_config["density"])
    X, Y = np.meshgrid(x, y)
    X = X.astype(np.float64)
    Y = Y.astype(np.float64)
    u = np.zeros_like(X, dtype=np.float64)
    v = np.zeros_like(Y, dtype=np.float64)
    for i in range(cstate.shape[0]):
        xi = cstate[i, 0]
        yi = cstate[i, 1]
        r2 = np.square(X-xi) + np.square(Y-yi)
        r2 = r2.astype(np.float64)
        u += (G*g_mass[i]*(X-xi) / (r2**3/2 + EPS))
        v += (G*g_mass[i]*(Y-yi) / (r2**3/2 + EPS))
    return u, v

# Bake Fields
g_minmax_init = False
g_field_min = 0 
g_field_max = 0
def bake_g_fields():
    global g_field_max
    global g_field_min
    global g_minmax_init
    numframes = int(g_config["duration"]/g_config["timestep"])
    baked_fields = np.zeros([numframes, g_config["density"], g_config["density"]])
    progress = tqdm(total = numframes)
    for i in range(numframes):
        gx, gy = g_field(simulation[i])
        gstrength = np.log(gx**2 + gy**2)
        baked_fields[i] = gstrength
        cmin = np.min(gstrength)
        cmax = np.max(gstrength)
        if cmin < g_field_min or not g_minmax_init:
            g_field_min = cmin
        if cmax > g_field_max or not g_minmax_init:
            g_field_max = cmax
        g_minmax_init = True
        progress.update(1)
    progress.close()
    return baked_fields
g_timer.start("Baking fields")
g_baked = bake_g_fields()
g_timer.end("Baked fields")

# Save final simulation
if g_config["save"]["enabled"] == "true":
    g_timer.start("Saving simulation state...")
    os.makedirs(g_config["save"]["path"], exist_ok=True)
    savestate(simulation, os.path.join(g_config["save"]["path"], ".simstate"))
    savestate(g_baked, os.path.join(g_config["save"]["path"], ".bakedstate"))
    g_timer.end("Saved simulation state")
if g_config["load"]["enabled"] == "true":
    g_timer.start("Loading simulation state...")
    simulation = loadstate(os.path.join(g_config["load"]["path"], ".simstate"))
    g_baked = loadstate(os.path.join(g_config["load"]["path"], ".bakedstate"))
    g_timer.end("Loaded simulation state")

# Animation function
def animate_func(i):
    gstr = g_baked[i*g_config["speed"]]
    mesh.set_array(gstr)
    g_scatter.set_offsets(simulation[i*g_config["speed"]]) # update particles scatter plot
    return g_scatter, mesh

# Set up figure
g_fig = plt.figure()
x = np.linspace(-g_config["bounds"], g_config["bounds"], g_config["density"])
y = np.linspace(-g_config["bounds"], g_config["bounds"], g_config["density"])
gx, gy = g_field(simulation[0])
gstr = np.log(gx**2 + gy**2)
X, Y = np.meshgrid(x, y)
mesh = plt.pcolormesh(X, Y, gstr, cmap='inferno', norm=colors.PowerNorm(gamma=0.3, vmin=int(g_field_min), vmax=int(g_field_max)))
g_scatter = plt.scatter(simulation[0][:,0], simulation[0][:,1], s=np.log(np.float64(g_mass/np.min(g_mass)+1))*15)
axs = g_fig.get_axes()
axs[0].set_xlim(-g_config["bounds"], g_config["bounds"])
axs[0].set_ylim(-g_config["bounds"], g_config["bounds"])
plt.gca().set_aspect('equal')

# Set up animation
g_anim = animation.FuncAnimation(g_fig, animate_func, frames=range(int(g_config["duration"] / (g_config["speed"] * g_config["timestep"]))), interval=40)

# Show animation
plt.show()
