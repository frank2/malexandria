# Malexandria

*Malexandria* is a cross-platform single-user malware database and analysis tool. Its design purpose is intended
to create a clean, relatively safe analysis environment that can be transported easily and altered across various systems
(for example, between analysis VMs and their host systems). It is also intended to make sharing analyses and their associated
samples a more streamlined task. Samples and malware data are all stored in a traditional fashion (encrypted zip files) for ease
of transport and manipulation outside the database, and can be bundled together and exported for further analysis without
the use of the Malexandria tool.

Currently, it's only ready for beta uses by people who are curious and want to try using the tool to integrate into their
malware analysis workflow. If you'd like to learn more about the tool, see the [using section](#using).

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
to compile this. The minimum C++ standard this project uses is **C++17**, so this project has been tested against Visual Studio 2019,
GCC 12 and Clang 13.

On both platforms, OpenSSL needs to be installed. I couldn't work any CMake voodoo with the locally-added libraries to get that to work.
On Windows, I've built the project with [these binaries](https://slproweb.com/products/Win32OpenSSL.html) (1.1.1t, not 1.1.1t light).
Standard system packages for OpenSSL (like Debian) should be fine. On Linux, libuuid and libncurses need to be installed. All other
dependencies are handled within the repository.

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

The general usecase of Malexandria is a reverse-engineer attempting to manage a variety of samples, be they malicious or benign,
and organize the analysis of the related samples into a format which can be easily transmitted between analysis systems. If the user
so wishes, they can also very easily transport the analyses and samples together outside of the Malexandria database to interested
individuals. Samples are stored safely within encrypted zip archives within a user-specified directory, and at the time of
analysis, can be extracted for modification and viewing.

Traditionally, malware samples arrive as a single file encrypted in a zip archive with a common password, such as samples
from [Malware Bazaar](bazaar.abuse.ch). You can import these archives with relative ease:

```
$ malexandria transport import -p infected ./c13d10d9eb5505c91c1982dd55019f8d5e5121952be774dc80eff344453c50ea.zip
>> detecting the type of archive file...singleton
c13d10d9eb5505c91c1982dd55019f8d5e5121952be774dc80eff344453c50ea
```

You can find information about the example sample [here](https://bazaar.abuse.ch/sample/c13d10d9eb5505c91c1982dd55019f8d5e5121952be774dc80eff344453c50ea/).
On a successful import of the zip archive, the sha256 hash of the sample is returned. This can be used to later identify the sample
and add various metadata to it, such as a more reasonable, descriptive name to identify it with.

```
$ malexandria sample modify c13d10d9eb5505c91c1982dd55019f8d5e5121952be774dc80eff344453c50ea \
   --alias w32.malicious.agent-tesla.c13d --tag pe --tag dotnet --tag malware \
   --family agent-tesla --filename tesla.exe
>> loading c13d10d9eb5505c91c1982dd55019f8d5e5121952be774dc80eff344453c50ea...got c13d10d9eb5505c91c1982dd55019f8d5e5121952be774dc80eff344453c50ea
>> sample info:
>>    filename: c13d10d9eb5505c91c1982dd55019f8d5e5121952be774dc80eff344453c50ea.exe
>>    md5:      8a4e7633cd4638e903e0d88842b94fd2
>>    sha1:     a8f2d447da5359eb0190422ebe5c957a836f43a8
>>    sha256:   c13d10d9eb5505c91c1982dd55019f8d5e5121952be774dc80eff344453c50ea
>>    sha3-384: fe7ac0eeb7f06e648f4fdb50f912e6d93ee5e0d291c16fac9f56b203982e69294f5bba0187cd01965d7e8c3b7cb555e0
>> setting alias for c13d10d9eb5505c91c1982dd55019f8d5e5121952be774dc80eff344453c50ea...done.
>> adding tag pe to w32.malicious.agent-tesla.c13d
>> adding tag dotnet to w32.malicious.agent-tesla.c13d
>> adding tag malware to w32.malicious.agent-tesla.c13d
>> adding family agent-tesla to w32.malicious.agent-tesla.c13d
>> setting filename to tesla.exe...done.
>> saving changes to database...done.
```

`w32.malicious.agent-tesla.c13d` can now be used to identify this sample, rather than its hash. We can then create an analysis
repository containing this sample.

```
$ malexandria analysis create --alias malicious.agent-tesla --sample w32.malicious.agent-tesla.c13d
C:\Users\frank2\.malexandria\active\analysis\malicious.agent-tesla
```

Let's have a look at what our analysis repo currently looks like:

```
$ cd C:\Users\frank2\.malexandria\active\analysis\malicious.agent-tesla
$ dir
 Volume in drive C is Windows
 Volume Serial Number is 6C01-6C92

 Directory of C:\Users\frank2\.malexandria\active\analysis\malicious.agent-tesla

05/26/2023  10:21 PM    <DIR>          .
05/26/2023  10:21 PM    <DIR>          ..
05/26/2023  10:21 PM    <DIR>          .mlx
05/26/2023  10:21 PM                 0 notes.md
05/26/2023  10:21 PM    <SYMLINK>      tesla.exe [.mlx\samples\c13d10d9eb5505c91c1982dd55019f8d5e5121952be774dc80eff344453c50ea.000]
               2 File(s)              0 bytes
               3 Dir(s)  113,595,899,904 bytes free

```

In analysis repositories, samples are stored in their benign format in the Malexandria metadata directory. In other words, they are stored with a
filename intended to not be directly executable on the target system. This is a precaution that works on Windows, since files are mapped to executable
handlers in the shell by file extension, but does not work on Unix platforms, which merely require the executable bit to be marked. For sake of usefulness,
though, the samples are linked into the main directory of the repository for any type of analysis, be it static or dynamic. This linking has the added
benefit of preventing the sample from being destroyed by self-deletion.

An analysis repository is initialized with a notes file that can be edited (or even removed). The notes filename can be configured within the Malexandria
configuration file.

During analysis, artifacts from the sample (such as shellcode and other executables) may arise. Additionally, you may want to add supplemental scripts
to the analysis process. These files can be tracked and stored within the analysis repository.

```
$ malexandria analysis add ./unpack.py
```

Files can be identified as artifacts of a specific sample within the analysis. This is for future use of searching the binary data of analysis
artifacts. 

```
$ malexandria analysis add --artifact w32.malicious.agent-tesla.c13d ./artifact.bin
```

The analysis can then be saved to disk, with the taintable state of the samples being stored in the analysis repository.

```
$ malexandria analysis save
```

Once the analysis is saved in the database, it can be transported to other Malexandria instances. Samples associated with the analysis
are automatically transferred along with the analysis on transport.

```
$ malexandria transport push --analysis malicious.agent-tesla frank2@remote-system
>> exporting 0 samples and 1 analyses
>> connecting to frank2@remote-system...
>> authenticating...
## Password:
>> successfully connected to remote server.
>> importing into remote malexandria instance...
>> imported!
```

The analysis can then be opened on the remote machine like so:

```
$ malexandria analysis open malicious.agent-tesla
/home/frank2/.malexandria/active/analysis/malicious.agent-tesla
```

And it's restored to its original state:

```
$ cd ~/.malexandria/active/analysis/malicious.agent-tesla
$ ls -lha
total 20K
drwxr-xr-x 3 frank2 frank2 4.0K May 26 23:42 .
drwxr-xr-x 3 frank2 frank2 4.0K May 26 23:42 ..
drwxr-xr-x 3 frank2 frank2 4.0K May 26 23:42 .mlx
-rw-r--r-- 1 frank2 frank2 1.0K May 26 23:42 artifact.bin
-rw-r--r-- 1 frank2 frank2    0 May 26 23:42 notes.org
lrwxrwxrwx 1 frank2 frank2   81 May 26 23:42 tesla.exe -> .mlx/samples/c13d10d9eb5505c91c1982dd55019f8d5e5121952be774dc80eff344453c50ea.000
-rw-r--r-- 1 frank2 frank2    0 May 26 23:42 unpack.py
```

As well as the sample information:

```
$ malexandria sample info w32.malicious.agent-tesla.c13d
[Sample ID: c13d10d9eb5505c91c1982dd55019f8d5e5121952be774dc80eff344453c50ea]
   alias:             w32.malicious.agent-tesla.c13d
   original filename: tesla.exe
   md5:               8a4e7633cd4638e903e0d88842b94fd2
   sha1:              a8f2d447da5359eb0190422ebe5c957a836f43a8
   sha256:            c13d10d9eb5505c91c1982dd55019f8d5e5121952be774dc80eff344453c50ea
   sha3-384:          fe7ac0eeb7f06e648f4fdb50f912e6d93ee5e0d291c16fac9f56b203982e69294f5bba0187cd01965d7e8c3b7cb555e0
   tag:               pe
   tag:               malware
   tag:               dotnet
   family:            agent-tesla
```

This is a really poor attempt at basic documentation of the tool while I continue fleshing it out for a proper release. To see further usage of
the tool, once you get it to compile, refer to the --help section of each module.
