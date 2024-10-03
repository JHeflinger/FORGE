import numpy as np
import matplotlib.pyplot as plt
from matplotlib import animation, colors, colormaps
from matplotlib.collections import LineCollection
from simulation import *
from time import time
from AnimationWrapper import InteractiveAnimation, PLOT_POS
DT = 3000  # timestep
N = 2  # number of particles
SIM_LEN = 1000  # number of steps to simulate
SIM_SPEED = 2  # number of steps to skip in the animation
E_PLOT_N = 100  # number of X and Y values in the electric field plot

# N particles each have a state consisting of four values:
# x position, y position, x velocity, y velocity
state = np.zeros((N, 4))
mass = np.array([[5972000000000000000000000], [73476730900000000000000]])


state = np.array([[0, 0, 0, -12.5865197104], [384000000, 0, 0, 1023]])

print("Starting simulation...")
start_time = time()
simulation = simulate_steps(state, mass, DT, SIM_LEN)
end_time = time()
elapsed = end_time - start_time
print(f"Done with simulation in {elapsed:.3f}s")

# custom colormap for positive and negative charges
seismic = colormaps["seismic"].resampled(255)
newcolors = seismic(np.linspace(0, 1, 255))
for i in range(3):
    newcolors[:127, i] -= np.linspace(0, 80, 127) / 256
    newcolors[127:, i] -= np.linspace(80, 0, 128) / 256
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
# print(np.max(G_strength))
# print(np.min(G_strength))
# print(G_strength)

# plot initial particles and electric field
fig = plt.figure()
main_ax = fig.add_axes(PLOT_POS)
norm = colors.PowerNorm(gamma=0.3, vmin=-136, vmax=-110)
mesh = main_ax.pcolormesh(X, Y, G_strength, cmap="inferno", norm=norm)
tmp = np.float64(mass / np.min(mass) + 1)
scatter = main_ax.scatter(
    state[:, 0], state[:, 1], s=np.log(tmp) * 15, c=mass, cmap=cmap, vmin=-3, vmax=3
)


def bake_g_fields():
    baked_fields = np.zeros([SIM_LEN, E_PLOT_N, E_PLOT_N])
    print(
        f"Baking frame {1} of ~{SIM_LEN} ({0:.2f}% done.)"
    )

    for i in range(SIM_LEN):
        print("\033[F\033[K", end="")
        percent = 100 * (i+1) / SIM_LEN
        print(
            f"Baking frame {i + 1} of ~{SIM_LEN} ({percent:.2f}% done.)"
        )
        Ex, Ey = G_field(simulation[i], mass, bound, E_PLOT_N)
        E_strength = np.log(Ex**2 + Ey**2)
        baked_fields[i] = E_strength

    return baked_fields


print("Baking g fields...")
start_time = time()
baked_g_fields = bake_g_fields()
end_time = time()
elapsed = end_time - start_time
print(f"Finished cook'n. in {elapsed:.3f}s.")


def animate_func(i):
    # simulation[i*SIM_SPEED] represents the new state in frame i of the animation
    # recalculate electric field for the new state
    E_strength = baked_g_fields[i * SIM_SPEED]
    # print(set(E_strength.flatten().tolist()))

    mesh.set_array(E_strength)  # update electric field plot
    scatter.set_offsets(simulation[i * SIM_SPEED])  # update particles scatter plot
    return scatter, mesh


anim = InteractiveAnimation(
    0, SIM_LEN//SIM_SPEED, fig, animate_func, interval=100
)

main_ax.set_xlim(-bound, bound)
main_ax.set_ylim(-bound, bound)
# fig.set_size_inches(6, 6)
# fig.subplots_adjust(
#     left=0, bottom=0, right=1, top=1, wspace=None, hspace=None
# )  # remove white border
fig.get_axes()[0].set_aspect("equal")
# plt.axis("off")

plt.show()
# anim.save('./animation.gif', dpi=50) # dpi=50 for lower quality to reduce file size
