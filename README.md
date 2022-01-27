# CS5625 Starter code

First, make sure you have cloned the project's submodules with `git submodule update --init --recursive`.

To try the demo:

1. To get Embree, Intel's ray tracing library working, you first need to install TBB. The easiest way to do this is to use your system's package manager. In absence of one, follow [this guide](https://www.intel.com/content/www/us/en/developer/articles/guide/get-started-with-tbb.html), dowloading TBB from [here](https://github.com/oneapi-src/oneTBB/releases).

2. Now we can get to actually building. It's a good idea to create a new folder in which to build things, so let's do that first.
```sh
mkdir build
cd build
```

3. Now tell `cmake` that you want it to create a Makefile for you using the project root's `CMakeLists.txt` configuration file. Then you can actually `make`. This is supposed to take a while, but it'll be constantly printing things.
```sh
cmake ..
make
```
You can also use `make -j<n>`, with `n` the number of threads you want to build with. It'll be pretty slow with the default of 1. Conventional wisdom says you should make this the number of cores on your system + 1. But don't expect to be able to do anything else while compiling. Once you've compiled once and are just prototyping, you might want to build with 1 thread, so that the `make` output is more sequential and thus more legible. 

4. Now run the actual demo.
```sh
./Demo              # or `./Demo Tetra` or `./Demo Sky` to try out others
```

To add your own binary to the project, create a new directory at the project root, and append `createExecutable("<dir_name>")` to `CMakeLists.txt`.
