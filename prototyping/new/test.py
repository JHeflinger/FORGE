from serial import *
from timer import *
from math import sqrt
from scipy.constants import G
from tqdm import tqdm
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

numtests = len(g_config["tests"])
trace(f"Found {numtests} test(s) to run:")
for i, test in enumerate(g_config["tests"]):
    print(f"  {i+1}. {test}")

g_accuracy = []
g_kinetic_energy = []
g_potential_energy = []
g_total_energy = []
g_timer = Timer()

def mag(p1, p2):
    return sqrt((p1[0] - p2[0])**2 + (p1[1] - p2[1])**2)

for i, test in enumerate(g_config["tests"]):
    # Load tests
    info(f"Loading {test} test states...")
    g_timer.end()
    test_simulation = loadstate(os.path.join(g_config["tests"][test]["test"], ".simstate"))
    truth_simulation = loadstate(os.path.join(g_config["tests"][test]["truth"], ".simstate"))
    simulation_masses = loadstate(os.path.join(g_config["tests"][test]["truth"], ".mass"))
    test_simulation_masses = loadstate(os.path.join(g_config["tests"][test]["test"], ".mass"))
    info(f"Finished loading {test} test in {Green}{g_timer.end()}{Reset} seconds")

    # Verify tests
    info(f"Verifying {test} test...")
    verified = len(test_simulation) == len(truth_simulation) and len(simulation_masses) == len(test_simulation_masses)
    if verified:
        for i in range(len(simulation_masses)):
            if simulation_masses[i] != test_simulation_masses[i]:
                verified = False
                break
    if verified:
        for i in range(len(truth_simulation)):
            if len(truth_simulation[i]) != len(test_simulation[i]) or len(truth_simulation[i]) != len(simulation_masses):
                verified = False
                break
    if verified:
        info(f"Verified {test} test in {Green}{g_timer.end()}{Reset} seconds")
    else:
        error(f"{test} was unable to be verified as a valid test.")
        continue

    # Test tests
    info(f"Testing {test}...")
    progress = tqdm(total = len(truth_simulation))
    accuracy_log = []
    kinetic_log = []
    potential_log = []
    total_log = []
    total_accuracy_error = 0.0
    total_kinetic_error = 0.0
    total_potential_error = 0.0
    total_energy_error = 0.0
    for i in range(len(truth_simulation)):
        truth_kinetic = 0.0
        truth_potential = 0.0
        test_kinetic = 0.0
        test_potential = 0.0
        for j in range(len(truth_simulation[i])):
            truth_pos = (truth_simulation[i][j][0], truth_simulation[i][j][1])
            test_pos = (test_simulation[i][j][0], test_simulation[i][j][1])
            truth_vel = (truth_simulation[i][j][2], truth_simulation[i][j][3])
            test_vel = (test_simulation[i][j][2], test_simulation[i][j][3])
            total_accuracy_error += mag(truth_pos, test_pos)
            truth_kinetic += 0.5*simulation_masses[j]*mag(truth_vel, (0, 0))
            test_kinetic += 0.5*simulation_masses[j]*mag(test_vel, (0, 0))
            for k in range(len(truth_simulation[i])):
                other_truth_pos = (truth_simulation[i][k][0], truth_simulation[i][k][1])
                other_test_pos = (test_simulation[i][k][0], test_simulation[i][k][1])
                if k == j:
                    continue
                thm = mag(other_truth_pos, truth_pos)
                tsm = mag(other_test_pos, test_pos)
                uc = (G*simulation_masses[j]*simulation_masses[k])
                if thm != 0:
                    truth_potential += uc / thm
                if tsm != 0:
                    test_potential += uc / tsm
        total_kinetic_error += abs(truth_kinetic - test_kinetic)
        total_potential_error += abs(truth_potential - test_potential)
        total_energy_error += abs((test_kinetic + test_potential) - (truth_kinetic + truth_potential))
        kinetic_log.append(total_kinetic_error)
        potential_log.append(total_potential_error)
        accuracy_log.append(total_accuracy_error)
        total_log.append(total_energy_error)
        progress.update(1)
    g_accuracy.append([test, accuracy_log])
    g_kinetic_energy.append([test, kinetic_log])
    g_potential_energy.append([test, potential_log])
    g_total_energy.append([test, total_log])
    progress.close()
    info(f"Finished testing {test} in {Green}{g_timer.end()}{Reset} seconds")

info("Finished running tests, saving results...")
results = { "accuracy": g_accuracy, "kinetic energy": g_kinetic_energy, "potential energy": g_potential_energy, "total energy": g_total_energy}
outpath = g_config["output"]
savestate(results, outpath)
info(f"Saved test results to {Green}{outpath}{Reset}")
