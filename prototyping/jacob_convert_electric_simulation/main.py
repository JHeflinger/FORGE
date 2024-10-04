import numpy as np
import matplotlib.pyplot as plt
from matplotlib import animation, colors, colormaps
from matplotlib.collections import LineCollection
from simulation import *
from time import time
from AnimationWrapper import InteractiveAnimation, PLOT_POS

# DT = 3000  # timestep
# N = 2  # number of particles
# SIM_LEN = 1000  # number of steps to simulate
# SIM_SPEED = 4  # number of steps to skip in the animation
# E_PLOT_N = 200  # number of X and Y values in the electric field plot
# # N particles each have a state consisting of four values:
# # x position, y position, x velocity, y velocity
# state = np.zeros((N, 4))
# mass = np.array([[5972000000000000000000000], [73476730900000000000000]])
# state = np.array([[0, 0, 0, -12.5865197104], [384000000, 0, 0, 1023]])

DT = 0.01 # timestep
N = 3  # number of particles
SIM_LEN = 2000  # number of steps to simulate
SIM_SPEED = 3  # number of steps to skip in the animation
E_PLOT_N = 120  # number of X and Y values in the electric field plot
# N particles each have a state consisting of four values:
# x position, y position, x velocity, y velocity
state = np.zeros((N, 4))
mass = np.array([[1], [1], [1]], dtype=np.float64)
state = np.array(
    [
        [-1, 0, 0.3471128135672417, 0.532726851767674],
        [1, 0, 0.3471128135672417, 0.532726851767674],
        [0, 0, -0.69422562713, -1.06545370354],
    ]
)

print("Starting simulation...")
start_time = time()
simulation = simulate_steps(state, mass, DT, SIM_LEN)
end_time = time()
elapsed = end_time - start_time
print(f"Done with simulation in {elapsed:.3f}s")

# for x in range(100):
#     simulation = simulate_steps(state, mass, DT, SIM_LEN)


# custom colormap for positive and negative charges
seismic = colormaps["seismic"].resampled(255)
newcolors = seismic(np.linspace(0, 1, 255))
for i in range(3):
    newcolors[:127, i] -= np.linspace(0, 80, 127) / 256
    newcolors[127:, i] -= np.linspace(80, 0, 128) / 256
newcolors[np.where(newcolors < 0)] = 0
cmap = colors.ListedColormap(newcolors)


bound = 4

g_minmax_init = False
g_field_min = 0
g_field_max = 0


def bake_g_fields(X, Y, r2_lut):
    global g_field_min, g_field_max, g_minmax_init
    baked_fields = np.zeros([SIM_LEN, E_PLOT_N, E_PLOT_N])
    print(f"Baking frame {1} of ~{SIM_LEN} ({0:.2f}% done.)")

    for i in range(SIM_LEN):
        print("\033[F\033[K", end="")
        percent = 100 * (i + 1) / SIM_LEN
        print(f"Baking frame {i + 1} of ~{SIM_LEN} ({percent:.2f}% done.)")
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


def calc_energy():
    global g_field_min, g_field_max, g_minmax_init
    baked_fields = np.zeros([SIM_LEN, E_PLOT_N, E_PLOT_N])
    print(f"Processing frame {1} of ~{SIM_LEN} ({0:.2f}% done.)")

    ke_list = list()
    pe_list = list()
    total_list = list()

    for i in range(SIM_LEN):
        print("\033[F\033[K", end="")
        percent = 100 * (i + 1) / SIM_LEN
        print(f"Processing frame {i + 1} of ~{SIM_LEN} ({percent:.2f}% done.)")
        state = simulation[i]
        ke, pe = getEnergy(simulation[i], mass)
        ke_list.append(ke)
        pe_list.append(pe)
        total_list.append(ke + pe)

    return ke_list, pe_list, total_list


print("Baking g fields...")
start_time = time()
print("pre-computing look up tables")
X, Y, r2_lut = pre_compute_r2(bound, E_PLOT_N, ignore_lut=True)
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
norm = colors.PowerNorm(
    gamma=0.3, vmin=(g_field_min + field_range), vmax=(g_field_max - field_range)
)
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


anim = InteractiveAnimation(0, SIM_LEN // SIM_SPEED, fig, animate_func, interval=67)

main_ax.set_xlim(-bound, bound)
main_ax.set_ylim(-bound, bound)
# fig.set_size_inches(6, 6)
# fig.subplots_adjust(
#     left=0, bottom=0, right=1, top=1, wspace=None, hspace=None
# )  # remove white border
fig.get_axes()[0].set_aspect("equal")
# plt.axis("off")

plt.show()

# --------------- PLOT ENERGY ------------------

# fig = plt.figure()
# print("Calculating energy...")
# ke, pe, total_e = calc_energy()
# fake_time = [x for x in range(len(ke))]
# # fake_time = [x*DT for x in range(len(ke))]
# fig, (ax1, ax2, ax3) = plt.subplots(3)
# ax1.plot(fake_time, ke)
# ax1.set_title("Total Kinetic Energy")

# ax2.plot(fake_time, pe)
# ax2.set_title("Total Potential Energy")

# ax3.plot(fake_time, total_e)
# ax3.set_title("Total Energy")
# plt.show()


# --------------- PLOT Paths ------------------
particle_one_x = simulation[:,0,0]
particle_one_y = simulation[:,0,1]

particle_two_x = simulation[:,1,0]
particle_two_y = simulation[:,1,1]

particle_three_x = simulation[:,2,0]
particle_three_y = simulation[:,2,1]

fig,ax = plt.subplots()
ax.scatter(particle_one_x, particle_one_y)
ax.scatter(particle_two_x, particle_two_y)
ax.scatter(particle_three_x, particle_three_y)
ax.set_aspect("equal")
plt.show()


# save_anim = animation.FuncAnimation(fig, animate_func, interval=67)
# save_anim.save("./animation.gif", dpi=40)  # dpi=50 for lower quality to reduce file size
