import time

Black = "\033[30m"
Red = "\033[31m"
Green = "\033[32m"
Yellow = "\033[33m"
Blue = "\033[34m"
Magenta = "\033[35m"
Cyan = "\033[36m"
White = "\033[37m"
Reset = "\033[0m"

class Timer:
    def __init__(self):
        self.curr = time.time_ns()
    
    def end(self, str=""):
        old = self.curr
        self.curr = time.time_ns()
        tmp = float((self.curr - old) // 1_000_000) / 1000
        if str != "":
            print(f"{str} in {Green}{tmp}{Reset} seconds")
        return tmp

    def start(self, str):
        self.end()
        print(str)
