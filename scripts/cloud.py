import sys
from random import random

cloudsize = 0
cloudradius = 0
minv = 0
maxv = 0
minm = 0
maxm = 0
radius = 0.0
content = ""
new = []

try:
    cloudsize = int(sys.argv[2])
    minv = int(sys.argv[3])
    maxv = int(sys.argv[4])
    minm = int(sys.argv[5])
    maxm = int(sys.argv[6])
    radius = float(sys.argv[7])
    cloudradius = int(sys.argv[8])
    with open(sys.argv[1], 'r') as f:
        content = f.read()
except:
    print("usage:\n\tpython cloud.py <file> <size> <min_velocity> <max_velocity> <min_mass> <max_mass> <radius> <cloudbounds>")

for i in range(cloudsize):
    newp = ""
    newp += "  - ID: " + str(i + 1) + "\n"
    newp += "    Name: Particle\n"
    newp += f"    Position: [{random()*cloudradius*2 - cloudradius}, {random()*cloudradius*2 - cloudradius}, {random()*cloudradius*2 - cloudradius}]\n"
    newp += f"    Velocity: [{random()*(maxv - minv) + minv}, {random()*(maxv - minv) + minv}, {random()*(maxv - minv) + minv}]\n"
    newp += f"    Mass: {random()*(maxm - minm) + minm}\n"
    newp += f"    Radius: {radius}\n"
    new.append(newp)

newcontent = ""
for line in content.split("\n"):
    if "  - ID: " in line or "    " in line:
        continue
    if "Particles:" in line:
        newcontent += f"{line}\n"
        for p in new:
            newcontent += p
    else:
        newcontent += f"{line}\n"
with open(sys.argv[1], 'w') as f:
    f.write(newcontent)
