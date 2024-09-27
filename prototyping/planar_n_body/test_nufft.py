import math
import time
import finufft as fp
import numpy as np
import matplotlib.pyplot as plt

np.random.seed(42)


def fourier_series_coeff_numpy(f, T, N):
    f_sample = 2 * N + 1
    t, dt = np.linspace(0, T, f_sample + 2, endpoint=False, retstep=True)
    y = np.fft.rfft(f(t)) / t.size
    return y


def test_func(t):
    # return math.cos(1/((t+0.75)/100))
    if t < 1:
        return -t
    return -1/((t+0.1)**3)

apply_test = np.vectorize(test_func)

N = int(1e4) # Num points.
period = 3
order = 1000

t = np.linspace(0, period, num=N) # Time points.
x = apply_test(t) 

start_time = time.time()

complex_coefs = fourier_series_coeff_numpy(apply_test, period, order)


#timing
print("Finished DFT in {0:.2g} seconds."
      .format(time.time()-start_time))


def get_approx_func(complex_coefs:np.ndarray, period):
    series_order = complex_coefs.size
    cos_coefs = np.real(complex_coefs) * 2
    sin_coefs = np.imag(complex_coefs) * 2
    def func_approx(t):
        ret_val = 0
        for i in range(series_order):
            n = i + 1
            ret_val += cos_coefs[i] * math.cos(n * math.pi * t / period) + sin_coefs[i] * math.sin(n * math.pi * t / period)
        return (cos_coefs[0]/2) * ret_val
    return func_approx

def series_complex_coeff(c, t, T):
    """calculates the Fourier series with period T at times t,
       from the complex coeff. c"""
    tmp = np.zeros((t.size), dtype=np.complex64)
    for k, ck in enumerate(c):
        # sum from 0 to +N
        tmp += ck * np.exp(2j * np.pi * k * t / T)
        # sum from -N to -1
        if k != 0:
            tmp += ck.conjugate() * np.exp(-2j * np.pi * k * t / T)
    return tmp.real
    
start_time = time.time()

four_func = get_approx_func(complex_coefs, period)
# print(four_func(2))
# apply_four_func = np.vectorize(four_func)
# y = apply_four_func(t)
y = series_complex_coeff(complex_coefs, t, period)


print("Finished value recovery in {0:.2g} seconds."
      .format(time.time()-start_time))



# t = t[400:-400]
# x = x[400:-400]
# y = y[400:-400]

mse = (np.square(x - y)).mean(axis=None)
print(f'MSE: {mse:.5f}')

plt.plot(t,x)
plt.plot(t,y)
plt.show()