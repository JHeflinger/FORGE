# from py_n_body import _rk45_adaptive_step
import numpy as np
from scipy.integrate import RK45
import math
import matplotlib.pyplot as plt

EPSILON = 0.001
INIT_T = 0
FINAL_T = 1.4
INIT_DT = 0.2


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


def test_deriv_gravity(
    time: float, arg_vector: np.ndarray, state_vector: np.ndarray
) -> np.ndarray:
    x, x_vel = state_vector
    # print(f"inside diy, x_vel = {x_vel}")
    return np.array([x_vel, -9.81])


def _rk4_step(first_deriv_func, state_vector, arg_vector, time_step, time):
    k1 = first_deriv_func(time, arg_vector, state_vector)
    k2 = first_deriv_func(time, arg_vector, state_vector + (time_step / 2) * k1)
    k3 = first_deriv_func(time, arg_vector, state_vector + (time_step / 2) * k2)
    k4 = first_deriv_func(time, arg_vector, state_vector + time_step * k3)

    return state_vector + ((time_step / 6) * (k1 + 2 * k2 + 2 * k3 + k4))


def test_deriv_sp_gravity(time: float, state_vector: np.ndarray) -> np.ndarray:
    x, [x_vel] = state_vector
    # print(f"inside sp, x_vel = {x_vel}")
    return np.array([x_vel, -9.81])

def test_deriv_tan(time: float, arg_vector: np.ndarray, state_vector: np.ndarray) -> np.ndarray:
    [y] = state_vector.tolist()
    return np.array([1 + y**2])

def test_deriv_tan_sp(time: float, state_vector: np.ndarray) -> np.ndarray:
    [[y]] = state_vector.tolist()
    return np.array([1 + y**2])




def main() -> None:
    state_vector_gravity = np.array([0.0, 10.0])
    arg_vector_gravity = np.array([0.0])

    state_vector_tan = np.array([0.0])
    arg_vector_tan = np.array([0.0])

    solution = RK45(
        test_deriv_tan_sp,
        INIT_T,
        state_vector_tan,
        FINAL_T,
        rtol=EPSILON,
        first_step=INIT_DT,
        vectorized=True,
    )
    sp_t_list = list()
    sp_x_list = list()
    curr_step = 0
    while True:
        solution.step()
        sp_t_list.append(solution.t)
        x = solution.y.tolist()[0]
        sp_x_list.append(x)
        print(f"scipy step size {solution.step_size}")
        if solution.status == "finished":
            print(f"Finished on step {curr_step}")
            break
        if solution.status == "failed":
            print(f"Failed on step {curr_step}")
            break
        curr_step += 1

    print("Done with scipy RK45")
    # return

    curr_t = INIT_T
    diy_t_list = list()
    diy_x_list = list()
    time_step = INIT_DT
    curr_step = 0
    while curr_t < FINAL_T:
        new_time_step = time_step
        # new_time_step, [x, x_vel] = _rk45_adaptive_step(
        #     curr_t, test_deriv, state_vector, arg_vector, time_step, EPSILON
        # )
        used_step, new_time_step, [x] = _rk45_adaptive_step(
            curr_t, test_deriv_tan, state_vector_tan, arg_vector_tan, time_step, EPSILON
        )

        # [x, x_vel] =_rk4_step(test_deriv_gravity, state_vector_gravity, arg_vector_gravity, time_step, curr_t)
        # [x] =_rk4_step(test_deriv_tan, state_vector_tan, arg_vector_tan, time_step, curr_t).tolist()

        diy_x_list.append(x)
        curr_t +=  used_step
        diy_t_list.append(curr_t)
        curr_step += 1
        time_step = new_time_step
        # state_vector_gravity = np.array([x, x_vel])
        state_vector_tan = np.array([x])
    print(f"Done with diy RK45 int {curr_step} steps")


    state_vector_tan = np.array([0.0])
    arg_vector_tan = np.array([0.0])

    curr_t = INIT_T
    forg_t_list = list()
    forg_x_list = list()
    time_step = 0.01
    curr_step = 0
    while curr_t < FINAL_T:
        [x] = _leap_frog_step(
            test_deriv_tan, state_vector_tan, arg_vector_tan, time_step, curr_t
        )

        # [x, x_vel] =_rk4_step(test_deriv_gravity, state_vector_gravity, arg_vector_gravity, time_step, curr_t)
        # [x] =_rk4_step(test_deriv_tan, state_vector_tan, arg_vector_tan, time_step, curr_t).tolist()

        forg_x_list.append(x)
        curr_t += time_step
        forg_t_list.append(curr_t)
        curr_step += 1
        # state_vector_gravity = np.array([x, x_vel])
        state_vector_tan = np.array([x])
    print(f"Done with diy leap_forg in {curr_step} steps!")


    # analytical_x_grav = [10 * t - 9.81 * (t**2) / 2 for t in diy_t_list]
    analytical_x_grav = [math.tan(t) for t in diy_t_list]

    error_list = [x_val - analytical_x_grav[idx] for idx, x_val in enumerate(diy_x_list)]
    abs_error_list = [abs(x) for x in error_list]

    avg_abs_error = sum(abs_error_list) / len(abs_error_list)
    avg_error = sum(error_list) / len(error_list)
    
    print(f"Avg Absolute Error RK45 = {avg_abs_error}")
    print(f"Avg Error RK45 = {avg_error}")


    analytical_x_grav = [math.tan(t) for t in forg_t_list]

    error_list = [x_val - analytical_x_grav[idx] for idx, x_val in enumerate(forg_x_list)]
    abs_error_list = [abs(x) for x in error_list]

    avg_abs_error = sum(abs_error_list) / len(abs_error_list)
    avg_error = sum(error_list) / len(error_list)

    print(f"Avg Absolute Error forg = {avg_abs_error}")
    print(f"Avg Error forg = {avg_error}")

    fig = plt.figure()
    ax1 = fig.add_subplot(111)
    ax1.scatter(sp_t_list, sp_x_list, c="b", marker="s", label="Scipy")
    ax1.scatter(diy_t_list, diy_x_list, c="r", marker="o", label="DIY")
    ax1.scatter(forg_t_list, forg_x_list, c="orange", marker="o", label="forg")
    ax1.scatter(forg_t_list, analytical_x_grav, c="g", marker="^", label="Actual")
    plt.legend(loc="upper left")
    plt.show()


if __name__ == "__main__":
    main()
