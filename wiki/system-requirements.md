# System requirements

## Processor architecture

| Supported | Not supported |
|-----------|---------------|
| x64       | x86, ARM      |

## Operating system

To install UnitTestBot C/C++ _client_, it is OK to have a Linux, macOS, or Windows machine.

To install UnitTestBot C/C++ _server_, you need Ubuntu 18.04–20.04.

| OS                         | Client             | Server                          |
|:---------------------------|:-------------------|:--------------------------------|
| Ubuntu 18.04–20.04         | :heavy_check_mark: | :heavy_check_mark:              |
| other Linux distributions  | :heavy_check_mark: | via Docker                      |
| macOS                      | :heavy_check_mark: | via Docker                      |
| Windows                    | :heavy_check_mark: | via WSL only or WSL with Docker |

## IDE

UnitTestBot C/C++ plugin is available for these IDEs:

* [Visual Studio Code](https://code.visualstudio.com/Download) 1.50 or later
* [CLion](https://www.jetbrains.com/clion/download/#section=windows) 2021.3–2022.*

## Build system

For now, UnitTestBot C/C++ needs no manual configuration only for CMake projects.
If your project is built with Make or other build system, try to configure the project [manually](https://github.com/UnitTestBot/UTBotCpp/issues/451).
