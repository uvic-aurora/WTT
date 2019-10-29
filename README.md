Wavelet Transform Toolkit (WTT)
=========================

Wavelet Transform Toolkit (WTT) consists of a C++ header-only library for computing and defining multi-level lifted wavelet transforms on 3-D meshes. In addition to the library, the author has implemented some command-line programs for demonstrating wavelet-based applications (e.g., computing wavelet transforms, wavelet denoising, and wavelet compression).

The author Shengyang Wei can be reached at the following email address:

* shengyangwei@protonmail.com

Dependency
------------

A complier with C++ 17 support is needed to compile WTT. The following versions of compilers and depended libraries have been verified to work with WTT:

* g++ (7.2+) or clang++ (5+)
* CGAL (4.2+)
* Boost (1.58+)

Installation
------------

In what follows, let `$TOP_DIR` denote the top-level directory of WTT software distribution (i.e., the directory containing this README file); let `$BUILD_DIR` denote a new directory to be created for building the software; and let `$INSTALL_DIR` denote the directory in which to install WTT.

To install WTT with CMake, use the command sequence:

```shell
cmake -H$TOP_DIR -B$BUILD_DIR -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR
cmake --build $BUILD_DIR --target install
```

Usage of Command-line Programs
------------------------------

After a successful installation, three programs can be found in directory `$INSTALL_DIR/bin`: `wtl_wavelet_analyze`, `wtl_wavelet_synthesize`, and `wtl_wavelet_filter`. Programs `wtl_wavelet_analyze` and `wtl_wavelet_synthesize` are used to compute the forward and inverse wavelet transform on a triangle mesh, respectively. Program `wtl_wavelet_filter` combines the forward and inverse transform and filters the intermediately produced wavelet coefficients. Users could set the filtering schemes via command-line options to perform wavelet denoising and compression on a triangle mesh. For the three programs, either the Loop or Butterfly scheme can be selected via command-line options.
Recently, the supported file format for a 3-D mesh is OFF.

Some example usages are listed below:

* To compute the 2-level Loop forward wavelet transform on mesh `vase-8.off`, users could run the following command:

```shell
wtl_wavelet_analyze -m Loop -l 2 -i vase-8.off -o vase-fwt.off -c vase-fwt.coefs
```

Mesh `vase-8.off` could be found in `$INSTALL_DIR/data` directory.

* To recover the original `vase.off` by 2-level Loop inverse wavelet transform, users could run the following command:

```shell
wtl_wavelet_synthesize -m Loop -l 2 -i vase-fwt.off -c vase-fwt.coefs -o vase-recover.off
```

* To perform 3-level Butterfly wavelet compression on mesh `vase-8.off`, users could run the following command:

```shell
wtl_wavelet_filter -m Butterfly -l 3 -i vase-8.off -o vase-reconstructed.off -c 5
```

The above command compresses the wavelet coefficients to 5%. That is, only the 5% wavelet coefficients are used to construct the output mesh.


Usage of Library APIs
---------------------

The APIs are composed of primary API and elementary API. Primary provides functions that can be called to compute the multi-level Loop and Butterfly wavelet transforms. The code examples below illustrate how to use the primary API to compute Loop and Butterfly wavelet transform on a triangle mesh.

* [computing loop wavelet transform](examples/usage_of_loop_wavelet_transform.cpp)
* [computing butterfly wavelet transform](examples/usage_of_butterfly_wavelet_transform.cpp)

The elementary API is provided into which users could incorporate custom operations to define a lifted wavelet transform. The code example below with line-by-line comments illustrate the steps of defining a custom wavelet transform.

* [defining a lifted wavelet transform](examples/define_a_wt_with_custom_lifting_steps.cpp)
