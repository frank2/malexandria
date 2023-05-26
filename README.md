# Malexandria

*Malexandria* is a cross-platform single-user malware database and analysis tool. Its design purpose is intended
to create a clean, relatively safe analysis environment that can be transported easily and altered across various systems
(for example, between analysis VMs and their host systems). It is also intended to make sharing analyses and their associated
samples a more streamlined task. Samples and malware data are all stored in a traditional fashion (encrypted zip files) for ease
of transport and manipulation outside the database, and can be bundled together and exported for further analysis without
the use of the Malexandria tool.

Currently, it's only ready for beta uses by people who are curious and want to try using the tool to integrate into their
malware analysis workflow. If you'd like to learn more about the tool, read on!

## Acquiring

This repository makes great use of Git submodules to organize necessary libraries for building the code, so cloning isn't as
straightforward as usual. Acquire the repository like this:

```
$ git clone --recursive https://github.com/frank2/malexandria.git
```

This will essentially clone all the git submodules into the `lib` directory and prepare you for building.

## Building

This project makes great use of CMake to accomplish its build tasks. First, download and install [CMake](https://cmake.org) for
your target platform. Currently, Windows 10 and above and Linux are supported. I do not have an Apple machine to test development
on, so if you care enough to try to build for Apple systems, go right ahead!

Compilers are another thing too. You will need a copy of GCC, Clang or [Microsoft Visual Studio](https://visualstudio.microsoft.com)
to compile this. The minimum C++ standard this project uses is **C++17**, so this project has been tested against Visual Studio 2019
and GCC 12. Clang hasn't been tested but should hopefully work fine.

On both platforms, OpenSSL needs to be installed. I couldn't work any CMake voodoo with the locally-added libraries to get that to work.
On Windows, I've built the project with [these binaries](https://slproweb.com/products/Win32OpenSSL.html) (1.1.1t, not 1.1.1t light).
Standard system packages for OpenSSL (like Debian) should be fine.

On Linux, libuuid needs to be installed. This is a prerequisite of stduuid.

With all this in mind, once everything is in place, building is simple. For example, on Windows, you could build the project like this:

```
$ cd malexandria
$ mkdir build
$ cd build
$ cmake -DOPENSSL_ROOT_DIR="C:/Program Files/OpenSSL-Win64" ../
$ cmake --build ./ --config Release
$ cmake --install ./
```

Linux is slightly different:

```
$ cd malexandria
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ../
$ cmake --build ./
$ cmake --install ./
```

## Using

I am writing this README at 2:00AM on a workday and I do not have more time, come back tomorrow. If you're lucky and manage to get it to build,
navigate the system for now with reading the various modules' --help commands.
