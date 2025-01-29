# Install MFEM

MFEM is a free, lightweight, scalable C++ library for finite element methods.
In this tutorial, we will install MFEM on your local machine.

> **NOTE 1** If you are using a Windows machine, you can install the Windows Subsystem for Linux (WSL) and follow the instructions for Linux.

> **NOTE 2** If you are using a Mac, I recommend using the Homebrew package manager to install dependencies. See, [Homebrew](https://brew.sh/).

> **NOTE 3** You can find the official installation instructions on the [MFEM website](https://mfem.org/install.html).


## Directory Setup and Basic Terminal Usage

- Before installing MFEM, make a clean directory, e.g., `$HOME/apma2560`.
- You can make a build directory and navigate to it using the following commands:
```bash
mkdir $HOME/apma2560 # [M]a[k]e a [dir]ectory apma2560 in your home ($HOME) directory
cd $HOME/apma2560    # [C]hange [D]irectory to apma2560
```
- Change `$HOME/apma2560` to the directory where you want to install MFEM.
- You can check your current directory using the following command:
```bash
pwd # [P]rint [w]orking [d]irectory
```
- You can list the files and directories in your current directory using the following command:
```bash
ls # [L]i[s]t files and directories
```
- `.` represents the current directory, `..` represents the parent directory, and `~` or `$HOME` represents the home directory.
```bash
cd .. # Change to the parent directory
```

## Build Serial MFEM

### Step 1: Install C/C++ Compiler
Check if you have a C/C++ compiler installed on your machine:
```bash
gcc --version
make --version
```
If you do not have a C/C++ compiler installed, then you will get an error message.

For Linux, you can install the prerequisites using the following command on your terminal:
```bash
sudo apt update # Update the package list
sudo apt install -y build-essential # Install C/C++ compiler and build tools
```
Check your installation by running the following command:
```bash
g++ --version # Check C++ compiler installation
make --version # Check make installation
cmake --version # Check cmake installation
```

For MacOS, you can use Apple's Xcode Command Line Tools. You can install it using the following command:
```bash
xcode-select --install # A dialog box will appear, click on Install. This will install necessary compilers and build tools
```
Check your installation by running the following command:
```bash
g++ --version
make --version
cmake --version
```

### Step 2: Download MFEM

You can download MFEM v4.5 from the [MFEM repository](https://github.com/mfem/mfem/releases).
If you have `git` installed, you can clone the repository using the following command:
```bash
git clone https://github.com/mfem/mfem.git
cd mfem
git checkout -b v4.5 # You can change the version number
```

### Step 3: Build MFEM

Now, change your directory to the MFEM source directory:
```bash
cd mfem
```
Then run the following commands to build MFEM:
```bash
make serial -j8
```
This will take some time to build MFEM.

After the build is complete, you can run the following command to run the first example:
```bash
cd examples
make ex0
./ex0
```

### All in One

Linux:
```bash
sudo apt update
sudo apt install -y build-essential
git clone https://github.com/mfem/mfem.git
cd mfem
git checkout -b v4.5
make serial -j8
cd examples
make ex0
./ex0
```
MacOS:
```bash
xcode-select --install
git clone https://github.com/mfem/mfem.git
cd mfem
git checkout -b v4.5
make serial -j8
cd examples
make ex0
./ex0
```

## Build GLVis

> For higher quality visualization, I recommend using `ParaView`. You may refer to `examples/ex5.cpp` for more information.

For interactive visualization, you can use `GLVis`.
If you are using an Apple machine with M-series chips, you can download the app from the [GLVis website](https://glvis.org/)

For Linux, you can build `GLVis` from the source code.
Firstly, navigate to the top level directory (`$HOME/apma2560`) and run the following commands:
```bash
apt-get install -y libfontconfig1-dev libfreetype-dev libsdl2-dev libglew-dev libglm-dev libpng-dev
git clone https://github.com/GLVis/glvis.git # Download GLVis repository
cd glvis # Change directory to the GLVis source directory
git checkout -b v4.3 # Get a specific version of GLVis
make MFEM_DIR=../mfem -j8 # Assuming mfem is in the parent directory
```
You can run `glvis` using the following command:
```bash
./glvis
```
This will launch a `GLVis` server waiting for a connection from MFEM. The server will be closed when you press `Ctrl+c` or close the terminal.

You can also run your `GLVis` server in the background using the following command:
```bash
./glvis &
```
Then, the server will be running in the background until you close the terminal.

Now, navigate to the MFEM examples directory (`mfem/examples`) and run the following command:
```bash
make ex1
./ex1
```

## Parallel MFEM

Parallel MFEM requires MPI (Message Passing Interface) to run in parallel.
Also, you need to install the `metis` (for graph partitioning) and `hypre` (for parallel data structures) libraries.

### Step 1: Install MPI
MFEM is designed to run in parallel using MPI (Message Passing Interface).
You can install MPI using the following command:

Linux:
```bash 
sudo apt install -y openmpi-bin openmpi-common openmpi-doc libopenmpi-dev
```
MacOS:
If you do not have `brew` installed, follow the instructions on the [Homebrew website](https://brew.sh/) and then restart the terminal.
After installing `brew`, you can run the following command:
```bash
brew install open-mpi
```
Check your installation by running the following command:
```bash
mpicc --version # Should return the version of MPI
```

### Step 2: Install Metis 5

Change your directory to the top level directory (`$HOME/apma2560`) and run the following commands:
```bash
curl -sL https://github.com/mfem/tpls/raw/refs/heads/gh-pages/metis-5.1.0.tar.gz -o metis-5.1.0.tar.gz # Download Metis 5
tar -xzf metis-5.1.0.tar.gz # Extract Metis 5
cd metis-5.1.0 # Change directory to Metis 5
make BUILDDIR=lib config # Configure Metis 5
make BUILDDIR=lib -j8 # Build Metis 5
cp lib/libmetis/libmetis.a lib # Copy the Metis 5 library so that MFEM can find it
```
Please make sure that your output does not contain any errors and the output contains `[100%] Built target ...`.
(You can ignore the warnings.)

### Step 3: Install Hypre

Change your directory to the top level directory (`$HOME/apma2560`) and run the following commands:
```bash
curl -sL https://github.com/hypre-space/hypre/archive/refs/tags/v2.26.0.tar.gz -o hypre-2.26.0.tar.gz # Download Hypre 2.26.0
tar -xzf hypre-2.26.0.tar.gz # Extract Hypre 2.26.0
cd hypre-2.26.0/src # Change directory to Hypre 2.26.0 source directory
./configure --disable-fortran # Configure Hypre 2.26.0
make -j8 # Build Hypre 2.26.0
cd ../.. # Change directory to the top level directory
ln -s hypre-2.26.0 hypre # Create a symbolic link (shortcut) to the Hypre 2.26.0 directory called hypre
```

### Step 4: Build Parallel MFEM

Change your directory to the top level directory (`$HOME/apma2560`) and run the following commands:
```bash
cd mfem # Change directory to the MFEM source directory
make parallel -j8 MFEM_USE_MPI=YES MFEM_USE_METIS_5=YES METIS_DIR=$MFEM_DIR/../metis-5.1.0
```
After the build is complete, you can run the following command to run the first example in parallel:
```bash
cd examples
make ex1p
mpirun -np 8 ./ex1p # Run ex1p with 8 MPI processes
```

