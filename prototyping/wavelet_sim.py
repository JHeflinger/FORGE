import numpy as np
import matplotlib.pyplot as plt
from scipy.ndimage import gaussian_filter
from matplotlib.animation import FuncAnimation

# Parameters
N = 1000  # Number of particles
box_size = 10.0  # Size of the box
grid_size = 64  # Size of the mesh grid
G = 90.0  # Gravitational constant
dt = 0.01  # Time step
num_steps = 100  # Number of simulation steps

# Particle initialization
np.random.seed(0)
positions = np.random.rand(N, 3) * box_size  # Random initial positions
velocities = (np.random.rand(N, 3) - 0.5) * 0.1  # Small initial velocities
masses = np.ones(N)  # Equal mass for all particles

# Initialize density grid
density_grid = np.zeros((grid_size, grid_size, grid_size))

# Function to update density grid
def update_density_grid():
    global density_grid
    density_grid.fill(0)  # Clear the grid
    grid_spacing = box_size / grid_size

    for i in range(N):
        # Determine which grid cell each particle belongs to
        grid_pos = (positions[i] / grid_spacing).astype(int)
        # Add particle mass to the corresponding grid cell
        density_grid[grid_pos[0] % grid_size, grid_pos[1] % grid_size, grid_pos[2] % grid_size] += masses[i]

# Function to compute gravitational potential using FFT
def compute_gravitational_potential():
    # Fourier transform of the density grid
    density_fft = np.fft.fftn(density_grid)
    # Wavenumber grid
    kx = np.fft.fftfreq(grid_size, d=box_size / grid_size) * 2 * np.pi
    ky = np.fft.fftfreq(grid_size, d=box_size / grid_size) * 2 * np.pi
    kz = np.fft.fftfreq(grid_size, d=box_size / grid_size) * 2 * np.pi
    kx, ky, kz = np.meshgrid(kx, ky, kz, indexing='ij')

    k_squared = kx**2 + ky**2 + kz**2
    k_squared[0, 0, 0] = 1  # Avoid division by zero

    # Gravitational potential in Fourier space
    potential_fft = -density_fft / (k_squared * (2 * np.pi)**2)
    potential_fft[0, 0, 0] = 0  # Set mean potential to zero

    # Inverse Fourier transform to get potential in real space
    potential = np.fft.ifftn(potential_fft).real

    return potential

# Function to compute forces on particles
def compute_forces(potential):
    forces = np.zeros_like(positions)
    grid_spacing = box_size / grid_size

    # Compute gradients (forces) using finite differences
    grad_potential_x = np.gradient(potential, axis=0)
    grad_potential_y = np.gradient(potential, axis=1)
    grad_potential_z = np.gradient(potential, axis=2)

    for i in range(N):
        # Determine which grid cell each particle belongs to
        grid_pos = (positions[i] / grid_spacing).astype(int)
        # Forces are negative gradient of potential
        forces[i, 0] = -grad_potential_x[grid_pos[0] % grid_size, grid_pos[1] % grid_size, grid_pos[2] % grid_size]
        forces[i, 1] = -grad_potential_y[grid_pos[0] % grid_size, grid_pos[1] % grid_size, grid_pos[2] % grid_size]
        forces[i, 2] = -grad_potential_z[grid_pos[0] % grid_size, grid_pos[1] % grid_size, grid_pos[2] % grid_size]

    return forces

# Store positions for animation
position_history = []

# Simulation loop
for step in range(num_steps):
    # Update density grid
    update_density_grid()

    # Compute gravitational potential
    potential = compute_gravitational_potential()

    # Compute forces on particles
    forces = compute_forces(potential)

    # Update velocities and positions of particles
    velocities += forces * dt / masses[:, None]
    positions += velocities * dt

    # Apply periodic boundary conditions
    positions = np.mod(positions, box_size)

    # Store positions for animation
    position_history.append(positions.copy())

    print(f"Step {step + 1}/{num_steps} completed.")

# Set up the figure and axis for animation
fig, ax = plt.subplots(figsize=(8, 8))
ax.set_xlim(0, box_size)
ax.set_ylim(0, box_size)
scat = ax.scatter([], [], s=1)

# Animation update function
def update(frame):
    positions = position_history[frame]
    scat.set_offsets(positions[:, :2])  # Update scatter plot data
    return scat,

# Create animation
ani = FuncAnimation(fig, update, frames=num_steps, interval=1, blit=True)

# Show animation
plt.show()
