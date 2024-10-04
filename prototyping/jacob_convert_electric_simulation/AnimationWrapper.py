import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.widgets import TextBox, Slider, Button
import numpy as np
import mpl_toolkits.axes_grid1
from time import time
import re


PLOT_POS = [0.05,0.05,0.9,0.80]

class InteractiveAnimation(animation.FuncAnimation):
    def __init__(self, min_i, max_i, *args, **kwargs):
        super().__init__(frames=self.play,*args, **kwargs)
        self.current_interval = self._interval
        self.runs = False
        self.i = 0
        self.min = min_i
        self.max = max_i
        self.old_text = "15"
        self.forwards = True
        self.event_source.stop()
        self.fig=args[0]
        self.last_time = time()
        self.setup((0.125, 0.9))

    def setup(self, pos):
        playerax = self.fig.add_axes([pos[0],pos[1], 0.64, 0.05])
        divider = mpl_toolkits.axes_grid1.make_axes_locatable(playerax)
        back_button_ax = divider.append_axes("right", size="80%", pad=0.05)
        stop_button_ax = divider.append_axes("right", size="80%", pad=0.05)
        forward_button_ax = divider.append_axes("right", size="80%", pad=0.05)
        step_forward_ax = divider.append_axes("right", size="100%", pad=0.05)

        self.fps_field_ax = divider.append_axes("right", size="100%", pad=0.8)
        self.text_box =TextBox(self.fps_field_ax, 'Target FPS', initial=15)
        self.text_box.on_submit(self.submit)
        
        slider_ax = divider.append_axes("right", size="500%", pad=0.07)
        self.button_oneback = Button(playerax, label='$\u29CF$')
        self.button_back = Button(back_button_ax, label='$\u25C0$')
        self.button_stop = Button(stop_button_ax, label='$\u25A0$')
        self.button_forward = Button(forward_button_ax, label='$\u25B6$')
        self.button_oneforward = Button(step_forward_ax, label='$\u29D0$')
        self.button_oneback.on_clicked(self.onebackward)
        self.button_back.on_clicked(self.backward)
        self.button_stop.on_clicked(self.stop)
        self.button_forward.on_clicked(self.forward)
        self.button_oneforward.on_clicked(self.oneforward)
        self.slider = Slider(slider_ax, '', self.min, self.max, valinit=self.i, valstep=1)
        self.slider.on_changed(self.set_pos)
        self.fig.canvas.mpl_connect('key_press_event', self.on_press)

    def on_press(self, event):
        if event.key == 'left':
            self.onebackward()
        elif event.key == 'right':
            self.oneforward()
        elif event.key == ' ':
            self.runs = not self.runs
        elif event.key in '0123456789':
            if event.inaxes in [self.fps_field_ax]:
                return
            target_i = int((self.max-self.min)* (eval(event.key)/10))
            self.i=target_i
            self.update(self.i)
            self._func(self.i)


    def play(self):
        while self.runs:
            curr_time = time()
            elapsed = curr_time - self.last_time
            self.last_time = curr_time
            fps = 1/elapsed
            self.fig.get_axes()[0].set_title(f"FPS: {fps:.0f}")

            self.i = self.i+self.forwards-(not self.forwards)
            if self.i > self.min and self.i < self.max - 1:
                self.update(self.i)
                yield self.i
            else:
                if self.i == self.max - 1:
                    self.i = self.min
                elif self.i == self.min:
                    self.i = self.max-1
                self.update(self.i)
                yield self.i

    def new_interval(self, interval):
        self.current_interval = interval
        self._interval = interval

    def submit(self, text):
        self.old_text
        new_text = re.sub('[^0-9]','', text)
        new_val = eval(new_text)

        self.new_interval((1000/new_val))
        if new_text != text:
            self.text_box.eventson = False
            self.text_box.set_val(new_text)
            self.text_box.eventson = True
        self.old_text = text

    def start(self):
        self.runs=True
        self.event_source.start()

    def stop(self, event=None):
        self.runs = False
        self.pause()

    def forward(self, event=None):
        self.forwards = True
        if self.i == self.max - 1:
            self.i = self.min
        self.start()

    def backward(self, event=None):
        self.forwards = False
        if self.i == self.min:
            self.i = self.max-1
        self.start()
    def oneforward(self, event=None):
        self.forwards = True
        self.onestep()

    def onestep(self):
        if self.i > self.min and self.i < self.max - 1:
            self.i = self.i+self.forwards-(not self.forwards)
        elif self.i <= self.min and not self.forward:
            self.i = self.max-1
        elif self.i <= self.min and self.forwards:
            self.i += 1
        elif self.i >= self.max - 1 and self.forwards:
            self.i = self.min
        elif self.i >= self.max - 1 and not self.forwards:
            self.i -= 1
        self._func(self.i)
        self.slider.set_val(self.i)
        self.fig.canvas.draw_idle()

    def onebackward(self, event=None):
        self.forwards = False
        self.onestep()

    def set_pos(self,i):
        self.i = int(self.slider.val)
        self._func(self.i)

    def update(self,i):
        self.slider.eventson = False
        self.slider.set_val(i)
        self.slider.eventson = True



if __name__ == "__main__":
    fig= plt.figure()
    ax = fig.add_axes([0.05,0.05,0.9,0.80])
    x = np.arange(0, 2*np.pi, 0.01)
    line, = ax.plot(x, np.sin(x))


    def animate(i):
        line.set_ydata(np.sin(x + i / 10.0))
        return line,

    ani = InteractiveAnimation(0, 10, fig, animate, interval=67)
    ani.new_interval(100)

    plt.show()