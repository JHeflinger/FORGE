# FORGE
We aim to create an abstracted node-based kinetic particle simulator that can be parallelized onto a cluster of devices on a local network, allowing a gentler barrier of entry to simulating particles with a significant amount of computing power. By making a kinetic solver instead of a more typical finite volume or linked equations method, people familiar with the fluid being simulated can properly set up the simulation without simulation specific knowledge. Being able to use a broad range of devices as an ad hoc computer cluster will allow users to essentially throw any amount of computational resources to make their simulations converge within a reasonable time frame. 

## Features

### Planned Features

FORGE will include but is not limited to the following features:
- Intuitive graphical interface
- Node based simulation configurations
- OpenGL rendering support
- Distributed computing options
- Simulation analysis
- Test and tutorial simulations
- Network and efficiency analysis

## How to Use
FORGE can be used in 2 different ways: it can be set up as either a host or client simulator!

### Host Simulation
Host simulations are where the simulations are designed and loaded. If needed, the host simulator can also run the simulation
all on their own! However, the wider use case aims to connecting client simulators to paralleize the simulation across the client
computing resources.

### Client Simulation
Client simulations are when the client device/simulator connects to a host simulator and provides resources for the host to run their simulation!

## Getting Started
To get started, run the following to clone the repository:

```
git clone --recursive https://github.com/JHeflinger/FORGE.git
```

This will initialize and download all the required git submodules for the dependencies FORGE uses. If you've already
cloned FORGE normally, you can also run the following to download submodules:

```
git submodule update --init --recursive
```

Once everything is properly cloned, run the build script for your respective system!

On Windows, this will be

```
./build.bat
```

On Linux systems, this will be

```
./build.sh
```

Note that this uses Bazel for building and executing binaries. If you do not have Bazel, make sure to properly install this beforehand
or else the build scripts won't work!

Once you have verified a successful build, you can just then run the run script to launch the application!

On Windows, this will be

```
./run.bat
```

On Linux systems, this will be

```
./run.sh
```

Happy simulating!

## Docs
This project supports doxygen documentation! However, the direct documentation files have not been included due to the bloat that it generates
in the repository. If you'd like to view the documentation, be sure to install doxygen and use the provided "doxygen_config" doxyfile to generate
the documentation! This doxyfile has been preconfigured to generate the correct documentation, and will output the documentation into the `docs/` folder.