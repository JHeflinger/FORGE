from serial import *
from timer import *
import os

g_config = tglobals()

def warn(mystr):
    print(f"[{Yellow}WARN{Reset}] {mystr}")

def trace(mystr):
    print(f"[TRACE] {mystr}")

def info(mystr):
    print(f"[{Green}INFO{Reset}] {mystr}")

def error(mystr):
    print(f"[{Red}ERROR{Reset}] {mystr}")

if g_config["timing"]["enabled"]:
    warn("The timing test is enabled. This may take a lot of compute resources and time.")

trace(f"Found {len(g_config["tests"])} test(s) to run:")
for i, test in enumerate(g_config["tests"]):
    print(f"  {i+1}. {test}")

g_accuracy = []
g_energy = []
g_timer = Timer()

for i, test in enumerate(g_config["tests"]):
    info(f"Loading {test} test states...")
    g_timer.end()
    test_simulation = loadstate(os.path.join(g_config["tests"][test]["test"], ".simstate"))
    truth_simulation = loadstate(os.path.join(g_config["tests"][test]["truth"], ".simstate"))
    info(f"Finished loading {test} test in {Green}{g_timer.end()}{Reset} seconds")
    info(f"Verifying {test} test...")
    verified = False
    
    if verified:
        info(f"Verified {test} test in {Green}{g_timer.end()}{Reset} seconds")
    else:
        error(f"{test} was unable to be verified as a valid test.")
