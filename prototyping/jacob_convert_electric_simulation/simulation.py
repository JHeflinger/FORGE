import numpy as np
from scipy.integrate import RK45
# from scipy.constants import G
G = 1

from line_profiler import profile

EPS = 1e-12 # epsilon for numerical stability
softening = EPS

def simulate_steps(state0, mass, h, steps):
    # state0 is an array of shape (N,4) for N objects
    # each row contains x, y, vx, and vy
    # m and q are arrays of shape (N) for N objects
    # h is the step size in seconds
    # steps is the number of steps to simulate

    y0 = state0.flatten() # considering the state of all particles as a single vector

    def get_derivative(vec):
        state = np.reshape(vec, state0.shape)
        x_pos = state[:,0:1]
        y_pos = state[:,1:2]
        x_vel = state[:,2:3]
        y_vel = state[:,3:4]

        dx = x_pos.T - x_pos
        dy = y_pos.T - y_pos

        
        inv_r3 = (dx**2 + dy**2 + softening**2)**(-1.5)

        ax = G * (dx * inv_r3) @ mass
        ay = G * (dy * inv_r3) @ mass
        
        d = np.hstack((x_vel, y_vel, ax,ay))
        d = np.reshape(d, state0.shape)

        d = d.flatten()

        return d

    
   
    simulation = [state0]
    solver = RK45(
        lambda t,y: get_derivative(y), 0, y0, t_bound=h*(steps+1), max_step=h
    )
    y = y0
    for _ in range(steps):
        # y += h * get_derivative(y)
        solver.step()
        y = solver.y
        state = np.reshape(y, state0.shape)
        simulation.append(np.copy(state))
    return np.array(simulation)

def pre_compute_r2(bound, n, ignore_lut=False):
    x_lin = np.linspace(-bound, bound, n)
    y_lin = np.linspace(-bound, bound, n)

    X, Y = np.meshgrid(x_lin, y_lin)
    
    X = X.astype(np.float64)
    Y = Y.astype(np.float64)

    r2_lut = np.zeros([n,n,n,n], dtype=np.float64)
    if not ignore_lut:
        for x in range(n):
            for y in range(n):
                r2_lut[x][y] = np.square(X-x) + np.square(Y-y)
    
    return X,Y,r2_lut


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

@profile
def G_field(state, mass, bound, n, X, Y, r2_lut):
    # returns x- and y-direction Gravity fields at each point given a state
    u = np.zeros_like(X, dtype=np.float64)
    v = np.zeros_like(Y, dtype=np.float64)
    # round_denom = bound
    # half_n = n//2

    for i in range(state.shape[0]):
        xi = state[i,0]
        yi = state[i,1]

        r2 = np.square(X-xi) + np.square(Y-yi)
        r2 = r2.astype(np.float64)

        # rounded_y = int((half_n * yi)//round_denom) + half_n
        # rounded_x = int((half_n * xi)//round_denom) + half_n
        # r2 = r2_lut[rounded_x][rounded_y]

        xDiff = X - xi
        yDiff = Y - yi

        x_num = mass[i][0]*xDiff
        y_num =  mass[i][0]*yDiff

        denom = (r2**3/2 + EPS)

        u +=  x_num / denom
        v += y_num / denom
    return G*u, G*v