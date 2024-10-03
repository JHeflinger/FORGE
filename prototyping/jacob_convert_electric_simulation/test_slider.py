import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.widgets import Slider

TWO_PI = 2*np.pi

fig, ax = plt.subplots()

time = np.arange(0.0, TWO_PI, 0.001)
initial_amp = .5
values = initial_amp*np.sin(time)
l, = plt.plot(time, values, lw=2)

ax = plt.axis([0,TWO_PI,-1,1])

slider_axis = plt.axes([0.25, .95, 0.50, 0.02])
# Slider
slider = Slider(slider_axis, 'Amp', 0, 1, valinit=initial_amp)

# Animation controls
is_paused = False # True if user has taken control of the animation
interval = 100 # ms, time between animation frames
loop_len = 5.0 # seconds per loop
scale = interval / 1000 / loop_len

def update_slider(val):
    global is_paused
    is_paused=True
    update(val)

def update(val):
    # update curve
    l.set_ydata(val*np.sin(time))
    # redraw canvas while idle
    fig.canvas.draw_idle()

def update_plot(num):
    global is_paused
    if is_paused:
        return l, # don't change

    val = (slider.val + scale) % slider.valmax
    slider.set_val(val)
    is_paused = False # the above line called update_slider, so we need to reset this
    return l,

def on_click(event):
    # Check where the click happened
    (xm,ym),(xM,yM) = slider.label.clipbox.get_points()
    if xm < event.x < xM and ym < event.y < yM:
        # Event happened within the slider, ignore since it is handled in update_slider
        return
    else:
        # user clicked somewhere else on canvas = unpause
        global is_paused
        is_paused=False

# call update function on slider value change
slider.on_changed(update_slider)

fig.canvas.mpl_connect('button_press_event', on_click)

ani = animation.FuncAnimation(fig, update_plot, interval=interval)

plt.show()