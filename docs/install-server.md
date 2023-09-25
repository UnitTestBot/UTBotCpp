# Install the UnitTestBot C/C++ server

To meet [system requirements](system-requirements), 
choose one of the UnitTestBot C/C++ installation options and prepare the corresponding environment:

* [**Linux**](linux)
	* [Local installation](install-server-on-ubuntu)
	* [Remote installation via Docker](linux-remote)
* [**Windows**](windows)
	* [Local installation via WSL](windows-local)
	* [Remote installation via Docker](windows-remote)
* [**macOS**](macos)

**Local or remote?**

The easiest way to use UnitTestBot C/C++ is to install it _locally_ — this option is available only if you have 
Windows or Ubuntu 18.04–20.04 on your computer.

The flip side of local installation is the risk of having dependency conflicts. UnitTestBot C/C++ requires using a specific toolchain: exact LLVM and GCC versions, and a particular solver. The other version of the necessary utility may be installed on a user
machine — it may cause a version conflict.

_Remote_ installation helps to avoid the conflicts and deploy the necessary toolchain safely. It is also the 
only available option for macOS or any Linux distribution except Ubuntu 18.04–20.04.