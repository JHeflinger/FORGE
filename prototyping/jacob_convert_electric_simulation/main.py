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
SIM_SPEED = 4  # number of steps to skip in the animation
E_PLOT_N = 200  # number of X and Y values in the electric field plot

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


bound = 500000000

g_minmax_init = False
g_field_min = 0 
g_field_max = 0
def bake_g_fields(X, Y, r2_lut):
    global g_field_min, g_field_max, g_minmax_init
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
        Ex, Ey = G_field(simulation[i], mass, bound, E_PLOT_N, X, Y, r2_lut)
        E_strength = np.log(Ex**2 + Ey**2)
        cmin = np.min(E_strength)
        cmax = np.max(E_strength)
        if cmin < g_field_min or not g_minmax_init:
            g_field_min = cmin
        if cmax > g_field_max or not g_minmax_init:
            g_field_max = cmax
        g_minmax_init = True
        baked_fields[i] = E_strength

    return baked_fields


print("Baking g fields...")
start_time = time()
print("pre-computing look up tables")
X,Y,r2_lut = pre_compute_r2(bound, E_PLOT_N, ignore_lut=True)
# print(r2_lut[0][2] - r2_lut[3][2])
# print(r2_lut[0][0] - r2_lut[3][3])
# exit(0)
baked_g_fields = bake_g_fields(X, Y, r2_lut)
end_time = time()
elapsed = end_time - start_time
print(f"Finished cook'n. in {elapsed:.3f}s.")


# plot initial particles and electric field
fig = plt.figure()
main_ax = fig.add_axes(PLOT_POS)
field_range = 0
# field_range = (g_field_max - g_field_min) * 0.1
norm = colors.PowerNorm(gamma=0.3, vmin=(g_field_min + field_range), vmax=(g_field_max - field_range))
mesh = main_ax.pcolormesh(X, Y, baked_g_fields[0], cmap="inferno", norm=norm)
tmp = np.float64(mass / np.min(mass) + 1)
scatter = main_ax.scatter(
    state[:, 0], state[:, 1], s=np.log(tmp) * 15, c=mass, cmap=cmap, vmin=-3, vmax=3
)


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
