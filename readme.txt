* CPCF
It is a C++ library provides primitives for building cross-platform native systems and applications. It is refactored from a collection of codes accumulated when I was doing research on Computer Graphics/Vision and Distributed Systems in MSR and ICT/CAS since 2002. The library favors performance (more speed, less resource) over simplicity and maintainability, use with caution. 

** Major Components


** Dependency
The library is totally self-contained. It embedded specific version of other open source code and credited below.

CPCF embeds the Intel IPP 7.1.1 and the MKL 11.3.3. (Static linking libraries) for image processing, cryptography and advanced numeric computation. ** Download static library package and follow instructions at https://github.com/jiapw/CPCF/releases/tag/CPCF-Libs **

** External Projects Embedded
* Intel IPP: https://software.intel.com/en-us/intel-ipp
* Intel MKL: https://software.intel.com/en-us/mkl
* LZMA SDK: https://www.7-zip.org/sdk.html
* Botan: https://botan.randombit.net/
* exprtk: http://www.partow.net/programming/exprtk/index.html
* libgif: http://giflib.sourceforge.net/
* libpng: http://www.libpng.org/pub/png/libpng.html
* libjpeg: http://libjpeg.sourceforge.net/
* OpenEXR: http://www.openexr.com/index.html
* RocksDB: https://rocksdb.org/
* Snappy: https://github.com/google/snappy
* zlib: https://zlib.net/
