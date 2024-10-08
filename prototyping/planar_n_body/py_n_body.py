from typing import Dict, List
import copy
import matplotlib.pyplot as plt
import json
import math
import time
# from scipy.constants import G
G = 1
import numpy as np

SIM_PARAM_PATH = "./bodies.json"

sim_dim_x_m: int
sim_dim_y_m: int
time_step_s: float
sim_duration_s: int


def _init() -> Dict[str, Dict]:
    global sim_dim_x_m, sim_dim_y_m, time_step_s, sim_duration_s
    json_rslt = dict()
    with open(SIM_PARAM_PATH, "r") as infile:
        json_string = infile.read()
        json_rslt = json.loads(json_string)

    body_info = json_rslt["bodies"]
    global_info = json_rslt["globals"]

    sim_dim_x_m = global_info["sim_dim_x_m"]
    sim_dim_y_m = global_info["sim_dim_y_m"]
    time_step_s = global_info["time_step_s"]
    sim_duration_s = global_info["sim_duration_s"]

    ret_bodies = dict()
    for body in body_info:
        name = body["name"]
        mass_kg = body["mass_kg"]
        diam_m = body["diam_m"]
        init_vel_x_ms = body["init_vel_x_ms"]
        init_vel_y_ms = body["init_vel_y_ms"]
        init_pos_x_m = body["init_pos_x_m"]
        init_pos_y_m = body["init_pos_y_m"]

        local_dict = {
            "pos_x": init_pos_x_m,
            "pos_y": init_pos_y_m,
            "vel_x": init_vel_x_ms,
            "vel_y": init_vel_y_ms,
            "mass_kg": mass_kg,
            "diam_m": diam_m,
        }
        ret_bodies[name] = local_dict

    return ret_bodies



def get_cmap(n, name='hsv'):
    '''Returns a function that maps each index in 0, 1, ..., n-1 to a distinct 
    RGB color; the keyword argument name must be a standard mpl colormap name.'''
    return plt.cm.get_cmap(name, n)

def display_result(
    pos_frames: List,
    onion_skin: int = 10,
    fps: int = 10,
    show_forces: bool = False,
    body_scale: int = 5,
    vector_scale: int = 2,
    jump_frames: int = 1,
) -> None:
        fig, ax = plt.subplots()
        ax.set_aspect("equal")
        ax.set_ylim([0, sim_dim_y_m])
        ax.set_xlim([0, sim_dim_x_m])
        og_frames = copy.deepcopy(pos_frames)
        pos_frames = pos_frames[::jump_frames]
        total_frames = len(pos_frames)

        onion_dict = {name: [list(), list()] for name in og_frames[0]} # 0 is x 1 is y.

        cmap = get_cmap(len(og_frames[0].keys()))
        body_colors = {name:cmap(idx) for idx, name in enumerate(og_frames[0].keys())}


        print("Precomputing position traces...")
        for frame in og_frames:
            for body_name in frame:
                onion_dict[body_name][0].append(frame[body_name]["pos_x"])
                onion_dict[body_name][1].append(frame[body_name]["pos_y"])
        print("Done.\n")

        print(f"Showing frame n/a of n/a")
        
        for idx, frame in enumerate(pos_frames):
            print("\033[F\033[K", end="")
            print(f"Showing frame {idx} of {total_frames}")
            ax.set_ylim([0, sim_dim_y_m])
            ax.set_xlim([0, sim_dim_x_m])
            for body_name in frame:
                color = body_colors[body_name]
                z_order = 2
                if body_name == "b_center":
                    z_order = 100
                    # color = "r"
                body_state = frame[body_name]
                pos_x = body_state["pos_x"]
                pos_y = body_state["pos_y"]
                radius = body_state["diam_m"] * body_scale / 2
                ax.add_patch(
                    plt.Circle((pos_x, pos_y), radius, color=color, zorder=z_order)
                )
                if onion_skin > 0:
                    x_points = onion_dict[body_name][0][max(0,(idx * jump_frames) - onion_skin*jump_frames):(idx * jump_frames)]
                    y_points = onion_dict[body_name][1][max(0,(idx * jump_frames) - onion_skin*jump_frames):(idx * jump_frames)]

                    ax.scatter(x_points,y_points, color=color, zorder = z_order-1, alpha=0.75, s=0.021)


            fig.canvas.draw()
            plt.pause(1 / fps)
            ax.clear()
        plt.show()
        print("Done.")


def _vector_len(x: float, y: float) -> float:
    return math.sqrt(x**2 + y**2)


def _dist(x1: float, y1: float, x2: float, y2: float) -> float:
    return _vector_len(x2 - x1, y2 - y1)


def _g_accel(m1_kg: float, m2_kg: float, dist_m: float):
    return m2_kg / ((dist_m**2))


def _normalize(x, y):
    length = _vector_len(x, y)
    return (x / length, y / length)


def _sum_of_accel(
    m1_kg: float,
    pos_x_1: float,
    pos_y_1: float,
    target_key: str,
    bodies: Dict[str, Dict],
):
    ret_x = 0
    ret_y = 0
    for effector_key in bodies:
        if effector_key == target_key:
            continue
        e_body = bodies[effector_key]
        mass_kg: float = e_body["mass_kg"]
        pos_x_2: float = e_body["pos_x"]
        pos_y_2: float = e_body["pos_y"]

        dist = _dist(pos_x_1, pos_y_1, pos_x_2, pos_y_2)
        accel = _g_accel(m1_kg, mass_kg, dist)


        r_x = pos_x_2 - pos_x_1
        r_y = pos_y_2 - pos_y_1

        inv_r3 = (r_x ** 2 + r_y ** 2) ** -1.5 # Delete the 4 and do softening properly.

        ret_x += (r_x * inv_r3) * mass_kg
        ret_y += (r_y * inv_r3) * mass_kg

        #n_x, n_y = _normalize(r_x, r_y)
        #ret_x += accel * n_x
        #ret_y += accel * n_y

        # r_theta = math.atan2(r_y, r_x)
        # ret_x += accel * math.cos(r_theta)
        # ret_y += accel * math.sin(r_theta)

        # print(f"Distance from {target_key} to {effector_key} is {dist}m")

    return G * ret_x, G * ret_y


# All variables dealt with must be numpy arrays.
def _rk4_step(first_deriv_func, state_vector, arg_vector, time_step, time):
    k1 = first_deriv_func(time, arg_vector, state_vector)
    k2 = first_deriv_func(time, arg_vector, state_vector + (time_step / 2) * k1)
    k3 = first_deriv_func(time, arg_vector, state_vector + (time_step / 2) * k2)
    k4 = first_deriv_func(time, arg_vector, state_vector + time_step * k3)

    return state_vector + ((time_step / 6) * (k1 + 2 * k2 + 2 * k3 + k4))

def _rk45_adaptive_step(
    time, first_deriv_func, state_vector, arg_vector, time_step, epsilon
):
    k1 = time_step * first_deriv_func(time, arg_vector, state_vector)
    k2 = time_step * first_deriv_func(
        time + time_step / 4, arg_vector, state_vector + (k1 / 4)
    )
    k3 = time_step * first_deriv_func(
        time + time_step * 3 / 8,
        arg_vector,
        state_vector + (k1 * 3 / 32) + (k2 * 9 / 32),
    )
    k4 = time_step * first_deriv_func(
        time + time_step * 12 / 13,
        arg_vector,
        state_vector + (k1 * 1932 / 2197) - (k2 * 7200 / 2197) + (k3 * 7296 / 2197),
    )
    k5 = time_step * first_deriv_func(
        time + time_step,
        arg_vector,
        state_vector
        + (k1 * 439 / 216)
        - (8 * k2)
        + (k3 * 3680 / 513)
        - (k4 * 845 / 4104),
    )
    k6 = time_step * first_deriv_func(
        time + time_step / 2,
        arg_vector,
        state_vector
        - (8 * k1 / 27)
        + (2 * k2)
        - (3544 * k3 / 2565)
        + (1859 * k4 / 4104)
        - (11 * k5 / 40),
    )

    new_state_1 = (
        state_vector + 25 * k1 / 216 + 1408 * k3 / 2565 + 2197 * k4 / 4104 - k5 / 5
    )
    new_state_2 = (
        state_vector
        + 16 * k1 / 135
        + 6656 * k3 / 12825
        + 28561 * k4 / 56430
        - 9 * k5 / 50
        + 2 * k6 / 55
    )

    truncation_error_est = (1 / time_step) * np.linalg.norm(new_state_2 - new_state_1)
    new_step = time_step
    other_new_time_step = time_step
    if truncation_error_est > 0.0:
        step_scalar = 0.84 * (epsilon / truncation_error_est) ** 0.25
        new_step = step_scalar * time_step

        other_new_time_step = (
            epsilon * time_step / (2 * np.linalg.norm(new_state_2 - new_state_1))
        ) ** 0.25

        new_step = min(other_new_time_step, new_step)
        # print(truncation_error_est)

    if truncation_error_est <= epsilon:
        # if new_step != time_step:
        #     print(f"Changing timestep to: {new_step:.3f}")
        return time_step, new_step, new_state_1
    
    # print(f"Recursing with time step: {new_step:.3f}")
    return _rk45_adaptive_step(
        time, first_deriv_func, state_vector, arg_vector, new_step, epsilon
    )

def _leap_frog_step(first_deriv_func, state_vector, arg_vector, time_step, time):
    forg = first_deriv_func(time + (time_step / 2), arg_vector, state_vector)
    return state_vector + time_step * forg

def _update_body_pos(time: float, arg_vector: np.ndarray, state_vector: np.ndarray):
    pos_x, pos_y, vel_x, vel_y = state_vector.tolist()
    accel_x, accel_y = arg_vector.tolist()
    return np.array([vel_x, vel_y, accel_x, accel_y], dtype=float)

def _get_energies(bodies: Dict[str, Dict]) -> Dict[str, Dict]:
    # Returns dict mapping body name to KE and PE.

    ret_dict = dict()

    for target_key in bodies:
            target_body = bodies[target_key]
            t_mass_kg = target_body["mass_kg"]
            t_pos_x = target_body["pos_x"]
            t_pos_y = target_body["pos_y"]
            t_vel_x = target_body["vel_x"]
            t_vel_y = target_body["vel_y"]

            _energy_kinetic = 0.5 * t_mass_kg * _vector_len(t_vel_x, t_vel_y)

            _energy_potential = 0

            for paired_body_key in bodies:
                if target_key == paired_body_key:
                    continue
                paired_body = bodies[paired_body_key]
                paired_body_mass_kg = paired_body["mass_kg"]
                paired_body_pos_x = paired_body["pos_x"]
                paired_body_pos_y = paired_body["pos_y"]

                pair_pe = -G * (t_mass_kg * paired_body_mass_kg) / _dist(t_pos_x, t_pos_y, paired_body_pos_x, paired_body_pos_y)

                _energy_potential += pair_pe
            
                ret_dict[target_key] = {
                    "Kinetic_Energy": _energy_kinetic,
                    "Potential_Energy":_energy_potential
                }
            return ret_dict

def _get_energies_total(bodies: Dict[str, Dict]):
    total_ke = 0
    total_pe = 0

    for target_key in bodies:
            if target_key == "b_center":
                continue
            target_body = bodies[target_key]
            t_mass_kg = target_body["mass_kg"]
            t_pos_x = target_body["pos_x"]
            t_pos_y = target_body["pos_y"]
            t_vel_x = target_body["vel_x"]
            t_vel_y = target_body["vel_y"]

            v_mag = _vector_len(t_vel_x, t_vel_y)
            total_ke = 0.5 * t_mass_kg * (v_mag**2)


            for paired_body_key in bodies:
                if target_key == paired_body_key or paired_body_key == "b_center":
                    continue
                
                paired_body = bodies[paired_body_key]

                paired_body_mass_kg = paired_body["mass_kg"]
                paired_body_pos_x = paired_body["pos_x"]
                paired_body_pos_y = paired_body["pos_y"]

                pair_pe = (t_mass_kg * paired_body_mass_kg) / _dist(t_pos_x, t_pos_y, paired_body_pos_x, paired_body_pos_y)

                total_pe += pair_pe
            
            total_pe *= -G/2
    return total_ke, total_pe, (total_ke + total_pe)

def run_sim(bodies: Dict[str, Dict]) -> List[Dict]:
    global time_step_s
    frame_buffer = list()
    num_epochs = int(sim_duration_s // time_step_s)
    curr_time = 0  # Seconds.
    for epoch in range(num_epochs):
        if epoch % 1000 == 0:
            print("\033[F\033[K", end="")
            percent_done = 100 * epoch / num_epochs
            print(
                f"Running Epoch {epoch + 1} of ~{num_epochs} ({percent_done:.2f}%) with step size {time_step_s}s"
            )
        total_mass = 0
        weighted_sum_x = 0
        weighted_sum_y = 0
        tmp_frame = dict()
        for target_key in bodies:
            target_body = bodies[target_key]
            t_mass_kg = target_body["mass_kg"]
            t_pos_x = target_body["pos_x"]
            t_pos_y = target_body["pos_y"]
            t_vel_x = target_body["vel_x"]
            t_vel_y = target_body["vel_y"]
            accel_x, accel_y = _sum_of_accel(
                t_mass_kg, t_pos_x, t_pos_y, target_key, bodies
            )

            state_vector = np.array([t_pos_x, t_pos_y, t_vel_x, t_vel_y], dtype=float)
            arg_vector = np.array([accel_x, accel_y], dtype=float)

            new_t_vel_x = t_vel_x + time_step_s * accel_x
            new_t_vel_y = t_vel_y + time_step_s * accel_y

            new_t_pos_x = t_pos_x + new_t_vel_x * time_step_s
            new_t_pos_y = t_pos_y + new_t_vel_y * time_step_s

            # Leap Forg.
            # new_t_vel_x = t_vel_x
            # new_t_vel_y = t_vel_y

            
            # new_t_vel_x += time_step_s * accel_x / 2
            # new_t_vel_y += time_step_s * accel_y / 2

            # new_t_pos_x = t_pos_x + new_t_vel_x * time_step_s
            # new_t_pos_y = t_pos_y + new_t_vel_y * time_step_s

            # new_t_vel_x += time_step_s * accel_x / 2
            # new_t_vel_y += time_step_s * accel_y / 2


            # new_t_pos_x, new_t_pos_y, new_t_vel_x, new_t_vel_y = _rk4_step(
            #     _update_body_pos, state_vector, arg_vector, time_step_s, time
            # ).tolist()

            # epsilon = 0.0001  # meters

            # used_step, new_step_size, [new_t_pos_x, new_t_pos_y, new_t_vel_x, new_t_vel_y] = (
            #     _rk45_adaptive_step(
            #         curr_time,
            #         _update_body_pos,
            #         state_vector,
            #         arg_vector,
            #         time_step_s,
            #         epsilon,
            #     )
            # )

            # [new_t_pos_x, new_t_pos_y, new_t_vel_x, new_t_vel_y] = (
            #     _leap_frog_step(
            #         _update_body_pos,
            #         state_vector,
            #         arg_vector,
            #         time_step_s,
            #         curr_time,
            #     )
            # )

            # time_step_s = min(new_step_size, time_step_s)

            local_body = copy.deepcopy(target_body)

            local_body["pos_x"] = new_t_pos_x
            local_body["pos_y"] = new_t_pos_y
            local_body["vel_x"] = new_t_vel_x
            local_body["vel_y"] = new_t_vel_y

            weighted_sum_x += t_mass_kg * new_t_pos_x
            weighted_sum_y += t_mass_kg * new_t_pos_y
            total_mass += t_mass_kg

            tmp_frame[target_key] = local_body

            # if target_key == "Earth":
            # time.sleep(1)
            # print(f"{target_key} accel_x: {accel_x} accel_y: {accel_y} old pos_x: {t_pos_x} new pos_x {new_t_pos_x} old pos_y: {t_pos_y} new_pos_y: {new_t_pos_y}")
            # exit()

        barycenter_x = weighted_sum_x / total_mass
        barycenter_y = weighted_sum_y / total_mass
        tmp_frame_b_center = copy.deepcopy(tmp_frame)
        tmp_frame_b_center["b_center"] = {
            "pos_x": barycenter_x,
            "pos_y": barycenter_y,
            "diam_m": 0.01,
        }
        # print(f"Barycenter x, rel pos {barycenter_x/sim_dim_x_m:.2f}, {barycenter_y/sim_dim_y_m:.2f}")
        frame_buffer.append(tmp_frame_b_center)
        bodies = tmp_frame
        curr_time += time_step_s
    return frame_buffer

def _display_energy_velocity_graphs(frame_list, time_step, show_vel=False) -> None:
    ke_list = list()
    pe_list = list()
    total_energy = list()

    frame_list = copy.deepcopy(frame_list)
    for frame in frame_list:
        del frame["b_center"]


    for frame in frame_list:
        ke, pe, tot = _get_energies_total(frame)
        ke_list.append(ke)
        pe_list.append(pe)
        total_energy.append(tot)
    
    fake_time = [x*time_step for x in range(len(ke_list))]
    fig, (ax1, ax2, ax3) = plt.subplots(3)
    ax1.plot(fake_time,ke_list)
    ax1.set_title("Total Kinetic Energy")

    ax2.plot(fake_time, pe_list)
    ax2.set_title("Total Potential Energy")

    ax3.plot(fake_time, total_energy)
    ax3.set_title("Total Energy")
    plt.show()

    if show_vel:
        pos_dict = {name: [list(), list()] for name in frame_list[0]} # 0 is x 1 is y.
        vel_dict = {name: list() for name in frame_list[0]}

        for frame in frame_list:
            for body_name in frame:
                pos_dict[body_name][0].append(frame[body_name]["pos_x"])
                pos_dict[body_name][1].append(frame[body_name]["pos_y"])

                vel_dict[body_name].append(_vector_len(frame[body_name]["vel_x"],frame[body_name]["vel_y"]))

        _, axes = plt.subplots(len(frame_list[0].keys()))

        for axis, body_name in zip(axes, frame_list[0].keys()):
            axis.plot(fake_time,vel_dict[body_name])
            axis.set_title(f"{body_name} velocity")
        plt.show()



def main() -> None:
    sim_bodies = _init()

    for name in sim_bodies.keys():
        print(name)

    result_frames = run_sim(sim_bodies)

    _display_energy_velocity_graphs(result_frames, time_step=time_step_s,show_vel=True)
    display_result(result_frames, fps=3000, jump_frames=20,onion_skin=100)


if __name__ == "__main__":
    main()
