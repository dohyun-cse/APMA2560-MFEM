# Install MFEM

MFEM is a free, lightweight, scalable C++ library for finite element methods.
In this tutorial, we will install MFEM on your local machine.

> **NOTE 1** If you are using a Windows machine, you can install the Windows Subsystem for Linux (WSL) and follow the instructions for Linux.

> **NOTE 2** If you are using a Mac, I recommend using the Homebrew package manager to install dependencies. See, [Homebrew](https://brew.sh/).

> **NOTE 3** You can find the official installation instructions on the [MFEM website](https://mfem.org/building).

> **NOTE 4** For code blocks, you can copy the code by clicking `copy` button on the top right corner of the code block.


## Directory Setup and Basic Terminal Usage

- Before installing MFEM, make a clean directory, e.g., `$HOME/apma2560`.
- You can make a build directory and navigate to it using the following commands:
```bash
mkdir $HOME/apma2560
cd $HOME/apma2560
```
The first line [m]a[k]es a [dir]ectory apma2560 in your home ($HOME) directory, and the second line [c]hange [d]irectory to apma2560
- Change `$HOME/apma2560` to the directory where you want to install MFEM.
- You can check your current directory using the following command:
```bash
pwd
```
[P]rint [w]orking [d]irectory
- You can list the files and directories in your current directory using the following command:
```bash
ls
```
[L]i[s]t files/directories.
- `.` represents the current directory, `..` represents the parent directory, and `~` or `$HOME` represents the home directory.
```bash
cd ..
```
[C]hange [d]irectory to `..` (parent directory).

## Build Serial MFEM

### Step 1: Install C/C++ Compiler
Check whether you have a C/C++ compiler installed on your machine:
```bash
gcc --version
make --version
```
If you do not have a C/C++ compiler installed, then you will get an error message.

<details>
    <summary>Linux</summary>

You can install the prerequisites using the following command on your terminal:

```bash
sudo apt update # Update the package list
sudo apt install -y build-essential # Install C/C++ compiler and build tools
```
</details>

<details>
    <summary>MacOS</summary>

You can use Apple's Xcode Command Line Tools. You can install it using the following command:
```bash
xcode-select --install
```
A dialog box will appear, click on Install. This will install necessary compilers and build tools.
</details>

Check your installation by running the following command:
```bash
g++ --version
make --version
```

### Step 2: Download MFEM

You can download MFEM v4.5 from the [MFEM repository](https://github.com/mfem/mfem/releases).
If you have `git` installed, you can clone the repository using the following command:
```bash
git clone https://github.com/mfem/mfem.git
cd mfem
git checkout -b v4.5
git pull origin v4.5
```
Clone means that you download a repository on your machine. Then `git checkout -b v4.5` means that we want to make our local version match with `v4.5` (branch `v4.5`). Finally, `git pull origin v4.5` download contents in version 4.5 if it is not up-to-date (you will see `Already up to date` if your version is up-to-date.

### Step 3: Build MFEM
Now, you 
Then run the following commands to build MFEM:
```bash
make serial -j8
```
This will take some time to build MFEM with 8 parallel processes.

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

MacOS: (Assuming you have `git` installed)
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

<details>
    <summary>Linux</summary>

Navigate to the top level directory (`$HOME/apma2560`) and run the following commands:
```bash
sudo apt-get install -y libfontconfig1-dev libfreetype-dev libsdl2-dev libglew-dev libglm-dev libpng-dev
git clone https://github.com/GLVis/glvis.git # Download GLVis repository
cd glvis
git checkout -b v4.3
make MFEM_DIR=../mfem -j8
```
</details>

<details>
    <summary>MacOS</summary>

    1. If you are using an Apple machine with M-series chips, you can download the app from the [GLVis website](https://glvis.org/).
    Then you can run `GLVis`` like any other app.

    2. If you don't have an M-series chip, you can build `GLVis` from the source code.
    ```bash
brew install fontconfig freetype sdl2 glew glm libpng
git clone https://github.com/GLVis/glvis.git
cd glvis
git checkout -b v4.3
make MFEM_DIR=../mfem -j8
    ```
</details>

You can run `glvis` using the following command:
```bash
./glvis
```
This will launch a `GLVis` server waiting for a connection from MFEM. The server will be closed when you press `Ctrl+c` or close the terminal.

You can also run your `GLVis` server in the background using the following command:
```bash
./glvis > /dev/null 2>&2 &
```
Then, the server will run in the background until you close the terminal.
- Here, `>` redirects output to `/dev/null`, a null device (nothing).
- Then `2>&2` redirects `2 (stderr)` to `2 (stderr)` so that we can still get an error when the server emits an error.
- Finally, the last `&` means the server runs in the background.
- It is a good idea to make an alias by adding the following line
  ```bash
  alias glvis='<GLVIS_DIR>/glvis > /dev/null 2>&2 &'
  ```
  to `$HOME/.bashrc` or `$HOME/.zshrc` depending on your shell environment.

Now, navigate to the MFEM examples directory (`mfem/examples`) and run the following command:
```bash
make ex1
./ex1
```

## Parallel MFEM

Parallel MFEM requires MPI (Message Passing Interface) to run in parallel.
Also, you need to install the `metis` (for graph partitioning) and `hypre` (for parallel data structures) libraries.

### Step 1: Install MPI

There are two popular implementations of MPI: `OpenMPI` and `MPICH`.
Because they two can conflict with each other, you should install only one of them.
In the following, we will use `OpenMPI`.
Please skip this step if you already have MPI installed. You can check your installation by running the following command:
```bash
mpicc --version
```
If you do not have MPI installed, you will get an error message.

You can install MPI using the following command:

<details>
    <summary>Linux</summary>

```bash 
sudo apt install -y openmpi-bin openmpi-common openmpi-doc libopenmpi-dev
```

</details>

<details>
    <summary>MacOS</summary>

```bash
brew install open-mpi
```

</details>

Check your installation by running the following command:
```bash
mpicc --version # Should return the version of MPI
```

### Step 2: Install Metis 5

Change your directory to the top level directory (`$HOME/apma2560`) and run the following commands:
```bash
curl -sL https://github.com/mfem/tpls/raw/refs/heads/gh-pages/metis-5.1.0.tar.gz -o metis-5.1.0.tar.gz
tar -xzf metis-5.1.0.tar.gz
cd metis-5.1.0
make BUILDDIR=lib config
make BUILDDIR=lib -j8
cp lib/libmetis/libmetis.a lib
```
Please make sure that your output does not contain any errors and the output contains `[100%] Built target ...`.
(You can ignore the warnings.)

### Step 3: Install Hypre

Change your directory to the top level directory (`$HOME/apma2560`) and run the following commands:
```bash
curl -sL https://github.com/hypre-space/hypre/archive/refs/tags/v2.26.0.tar.gz -o hypre-2.26.0.tar.gz
tar -xzf hypre-2.26.0.tar.gz
cd hypre-2.26.0/src
./configure --disable-fortran
make -j8
cd ../..
ln -s hypre-2.26.0 hypre
```

### Step 4: Build Parallel MFEM

Change your directory to the top level directory (`$HOME/apma2560`) and run the following commands:
```bash
cd mfem
make parallel -j8 MFEM_USE_MPI=YES MFEM_USE_METIS_5=YES METIS_DIR=@MFEM_DIR@/../metis-5.1.0
```
After the build is complete, you can run the following command to run the first example in parallel:
```bash
cd examples
make ex1p
mpirun -np 8 ./ex1p
```
This will run `ex1p` with 8 processes.

## Debugging

By default, MFEM is built in release mode.
The release mode is optimized for performance and does not include debugging information.
If you want to debug your code, you can build MFEM in debug mode.
Serial:
```bash
make debug -j8
```
Parallel:
```bash
make pdebug -j8
```
