import numpy as np
import matplotlib.pyplot as plt
from matplotlib import animation, colors, colormaps
from matplotlib.collections import LineCollection
from simulation import *

DT = 3000 # timestep
N = 2 # number of particles
SIM_LEN = 500 # number of steps to simulate
SIM_SPEED = 1 # number of steps to skip in the animation
E_PLOT_N = 200 # number of X and Y values in the electric field plot

# N particles each have a state consisting of four values:
# x position, y position, x velocity, y velocity
state = np.zeros((N,4))
mass = np.array( [
                    [5972000000000000000000000],
                    [73476730900000000000000]
                 ])


state = np.array([[0,0,0,-12.5865197104],
                  [384000000,0,0,1023]])

simulation = simulate_steps(state, mass, DT, SIM_LEN)
print("Done with simulation.")

# custom colormap for positive and negative charges
seismic = colormaps['seismic'].resampled(255)
newcolors = seismic(np.linspace(0, 1, 255))
for i in range(3):
    newcolors[:127,i] -= np.linspace(0,80,127)/256
    newcolors[127:,i] -= np.linspace(80,0,128)/256 
newcolors[np.where(newcolors < 0)] = 0
cmap = colors.ListedColormap(newcolors)

# calculate initial electric field
bound = 500000000
x = np.linspace(-bound, bound, E_PLOT_N)
y = np.linspace(-bound, bound, E_PLOT_N)
X, Y = np.meshgrid(x, y)
Ex, Ey = G_field(state, mass, bound, E_PLOT_N)
G_strength = np.log(Ex**2 + Ey**2 + EPS)
# print(G_strength)
print(np.max(G_strength))
print(np.min(G_strength))
# print(G_strength)

# plot initial particles and electric field
fig = plt.figure()
norm = colors.PowerNorm(gamma=0.3, vmin=-136, vmax=-110)
mesh = plt.pcolormesh(X, Y, G_strength, cmap='inferno', norm=norm)
tmp = np.float64(mass/np.min(mass)+1)
scatter = plt.scatter(state[:,0], state[:,1], s=np.log(tmp)*15,
                      c=mass, cmap=cmap, vmin=-3, vmax=3)
axs = fig.get_axes()

def animate_func(i):
    # simulation[i*SIM_SPEED] represents the new state in frame i of the animation
    # recalculate electric field for the new state
    Ex, Ey = G_field(simulation[i*SIM_SPEED], mass, bound, E_PLOT_N)
    E_strength = np.log(Ex**2 + Ey**2)
    # print(set(E_strength.flatten().tolist()))

    mesh.set_array(E_strength) # update electric field plot
    scatter.set_offsets(simulation[i*SIM_SPEED]) # update particles scatter plot
    return scatter, mesh

anim = animation.FuncAnimation(
    fig, animate_func, frames=range(SIM_LEN//SIM_SPEED), interval=40)

axs[0].set_xlim(-bound, bound)
axs[0].set_ylim(-bound, bound)
fig.set_size_inches(6, 6)
fig.subplots_adjust(left=0, bottom=0, right=1, top=1, wspace=None, hspace=None) # remove white border
plt.gca().set_aspect('equal')
plt.axis('off')

plt.show()
anim.save('./animation.gif', dpi=50) # dpi=50 for lower quality to reduce file size
