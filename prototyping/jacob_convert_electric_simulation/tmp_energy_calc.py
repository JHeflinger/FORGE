import numpy as np
from scipy.constants import G

def getEnergy(state, mass):
	"""
	Get kinetic energy (KE) and potential energy (PE) of simulation
	pos is N x 2 matrix of positions
	vel is N x 2 matrix of velocities
	mass is an N x 1 vector of masses
	G is Newton's Gravitational constant
	KE is the kinetic energy of the system
	PE is the potential energy of the system
	"""
	
	pos = state[:,0:2]
	vel = state[:,2:4]

	# Kinetic Energy:
	KE = 0.5 * np.sum(np.sum( mass * vel**2 ))

	# Potential Energy:
	# positions r = [x,y,z] for all particles
	x = pos[:,0:1]
	y = pos[:,1:2]
	# matrix that stores all pairwise particle separations: r_j - r_i
	dx = x.T - x
	dy = y.T - y
	# matrix that stores 1/r for all particle pairwise particle separations 
	inv_r = np.sqrt(dx**2 + dy**2 )
	inv_r[inv_r>0] = 1.0/inv_r[inv_r>0]
	# sum over upper triangle, to count each interaction only once
	PE = G * np.sum(np.sum(np.triu(-(mass*mass.T)*inv_r,1)))
	
	return KE, PE;

state = np.array(
	[
		[-1, 0, 0.3471128135672417, 0.532726851767674],
		[1, 0, 0.3471128135672417, 0.532726851767674],
		[0, 0, -0.69422562713, -1.06545370354],
	]
)

target_energy = -1.287144

mass = np.array([[0.1], [0.1], [0.1]], dtype=np.float64)
KE, PE = getEnergy(state, mass)
best_energy = KE + PE
best_error = abs(best_energy - target_energy)
best_mass = 0.1

for x in range(100):
	mass = np.array([[0.1 * x], [0.1 * x], [0.1 * x]], dtype=np.float64)
	KE, PE = getEnergy(state, mass)
	total_energy = KE + PE
	print(total_energy)
	error = abs(total_energy - target_energy)
	if error < best_error:
		best_error = error
		best_energy = total_energy
		best_mass = 0.1 * x

print(f"Target: {target_energy}J, best: {best_energy}J, mass at best: {best_mass}kg")
	# print(f"Total_energy: {total_energy}J")