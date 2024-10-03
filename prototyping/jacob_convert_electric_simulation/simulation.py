import numpy as np
from scipy.integrate import RK45
from scipy.constants import G

EPS = 1e-12 # epsilon for numerical stability
softening = 3

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

def G_field(state, mass, bound, n):
    # returns x- and y-direction Gravity fields at each point given a state
    x = np.linspace(-bound, bound, n)
    y = np.linspace(-bound, bound, n)
    X, Y = np.meshgrid(x, y)
    X = X.astype(np.float128)
    Y = Y.astype(np.float128)
    u = np.zeros_like(X, dtype=np.float128)
    v = np.zeros_like(Y, dtype=np.float128)
    for i in range(state.shape[0]):
        xi = state[i,0]
        yi = state[i,1]
        r2 = np.square(X-xi) + np.square(Y-yi)
        r2 = r2.astype(np.float128)
        u += G*mass[i][0]*(X-xi) / (r2**3/2 + EPS)
        v += G*mass[i][0]*(Y-yi) / (r2**3/2 + EPS)
    return u, v