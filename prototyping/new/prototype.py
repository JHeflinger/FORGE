from serial import *
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import animation, colors, colormaps
from matplotlib.collections import LineCollection
from scipy.constants import G
from scipy.integrate import RK45
from timer import *

# Initialize particles and their states
EPS = 1e-12 # epsilon for numerical stability
g_timer = Timer()
g_config = globals()
g_state = np.zeros((len(g_config["particles"]), 4))
g_mass = np.zeros(len(g_config["particles"]))
g_softening = g_config["softening"]
for i, particle in enumerate(g_config["particles"]):
    g_mass[i] = particle["mass"]
    g_state[i][0] = particle["position"][0]
    g_state[i][1] = particle["position"][1]
    g_state[i][2] = particle["velocity"][0]
    g_state[i][3] = particle["velocity"][1]

# Solvers
g_solvers = dict()
steps = int(g_config["duration"] / g_config["timestep"])
def rk45_get_derivative(vec):
    state = np.reshape(vec, g_state.shape)
    x_pos = state[:,0:1]
    y_pos = state[:,1:2]
    x_vel = state[:,2:3]
    y_vel = state[:,3:4]
    dx = x_pos.T - x_pos
    dy = y_pos.T - y_pos
    inv_r3 = (dx**2 + dy**2 + g_config["softening"]**2)**(-1.5)
    ax = G * (dx * inv_r3) @ g_mass.reshape(-1, 1)
    ay = G * (dy * inv_r3) @ g_mass.reshape(-1, 1)
    d = np.hstack((x_vel, y_vel, ax,ay))
    d = np.reshape(d, g_state.shape)
    d = d.flatten()
    return d
g_solvers.update({
    "RK45":
    RK45(lambda t,y: rk45_get_derivative(y), 0, g_state.flatten(), t_bound=g_config["timestep"]*(steps+1), max_step=g_config["timestep"])
})

# Simulation function
def simulate_steps():
    simulation = [g_state]
    steps = int(g_config["duration"] / g_config["timestep"])
    solver = g_solvers[g_config["solver"]]
    y = g_state.flatten()
    for _ in range(steps):
        solver.step()
        y = solver.y
        state = np.reshape(y, g_state.shape)
        simulation.append(np.copy(state))
    return np.array(simulation)

# Field calculation
def g_field(cstate):
    x = np.linspace(-g_config["bounds"], g_config["bounds"], g_config["density"])
    y = np.linspace(-g_config["bounds"], g_config["bounds"], g_config["density"])
    X, Y = np.meshgrid(x, y)
    X = X.astype(np.float128)
    Y = Y.astype(np.float128)
    u = np.zeros_like(X, dtype=np.float128)
    v = np.zeros_like(Y, dtype=np.float128)
    for i in range(cstate.shape[0]):
        xi = cstate[i, 0]
        yi = cstate[i, 1]
        r2 = np.square(X-xi) + np.square(Y-yi)
        r2 = r2.astype(np.float128)
        u += (G*g_mass[i]*(X-xi) / (r2**3/2 + EPS))
        v += (G*g_mass[i]*(Y-yi) / (r2**3/2 + EPS))
    return u, v

# Do simulation
g_timer.start("Starting simulation")
simulation = simulate_steps()
g_timer.end("Finished simulation")

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
    return baked_fields
g_timer.start("Baking fields")
g_baked = bake_g_fields()
g_timer.end("Baked fields")

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
gx, gy = g_field(g_state)
gstr = np.log(gx**2 + gy**2)
X, Y = np.meshgrid(x, y)
mesh = plt.pcolormesh(X, Y, gstr, cmap='inferno', norm=colors.PowerNorm(gamma=0.3, vmin=int(g_field_min), vmax=int(g_field_max)))
g_scatter = plt.scatter(g_state[:,0], g_state[:,1], s=np.log(np.float64(g_mass/np.min(g_mass)+1))*15)
axs = g_fig.get_axes()
axs[0].set_xlim(-g_config["bounds"], g_config["bounds"])
axs[0].set_ylim(-g_config["bounds"], g_config["bounds"])
plt.gca().set_aspect('equal')

# Set up animation
g_anim = animation.FuncAnimation(g_fig, animate_func, frames=range((g_config["duration"])//(g_config["speed"])), interval=40)

# Show animation
plt.show()