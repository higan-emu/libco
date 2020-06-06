# Supported targets
Note that supported targets are only those that have been tested and confirmed
working. It is quite possible that libco will work on more processors, compilers
and operating systems than those listed below.

## libco.x86
* Overhead: ~5x
* Supported processor(s): 32-bit x86
* Supported compiler(s): any
* Supported operating system(s):
  * Windows
  * Mac OS X
  * Linux
  * BSD

##   libco.amd64
* Overhead: ~10x (Windows), ~6x (all other platforms)
* Supported processor(s): 64-bit amd64
* Supported compiler(s): any
* Supported operating system(s):
  * Windows
  * Mac OS X
  * Linux
  * BSD

## libco.ppc
* Overhead: ~20x
* Supported processor(s): 32-bit PowerPC, 64-bit PowerPC
* Supported compiler(s): GNU GCC
* Supported operating system(s):
  * Mac OS X
  * Linux
  * BSD
  * Playstation 3

Note: this module contains compiler flags to enable/disable FPU and Altivec
support.

## libco.fiber
* Overhead: ~15x
* Supported processor(s): Processor independent
* Supported compiler(s): any
* Supported operating system(s):
  * Windows

## libco.sjlj
* Overhead: ~30x
* Supported processor(s): Processor independent
* Supported compiler(s): any
* Supported operating system(s):
  * Mac OS X
  * Linux
  * BSD
  * Solaris

## libco.ucontext
* Overhead: ***~300x***
* Supported processor(s): Processor independent
* Supported compiler(s): any
* Supported operating system(s):
  * Linux
  * BSD
