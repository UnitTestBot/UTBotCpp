# UTBot Developer Guide

## How to install docker image

UTBot has many dependencies, so the easiest way to build the tool from source and develop it is to use the docker
container.

UTBot has a published docker package
called [base_env](https://github.com/UnitTestBot/UTBotCpp/pkgs/container/utbotcpp%2Fbase_env). It contains all the
needed dependencies such as Git, LLVM, GRPC, GoogleTest and others. **base_env** has multiple versions tagged with
dates. If you are developing the tool, you are most likely to need the most recent version
from [here](https://github.com/UnitTestBot/UTBotCpp/pkgs/container/utbotcpp%2Fbase_env).

Supported and tested development configuration are Ubuntu 20.04 or Windows Subsystem for Linux (Ubuntu 20.04).

1. Install docker for [Ubuntu](https://docs.docker.com/engine/install/ubuntu/)
   or [WSL+Ubuntu](https://docs.docker.com/desktop/windows/wsl/)
2. Install docker plugin that allows to mount host filesystem and remap owner and group:
   ```
   docker plugin install lebokus/bindfs
   ```
3. Login into github docker registry:
   ```
   docker login -u <github-username> -p <github-personal-access-token> ghcr.io
   ``` 
   > You can create new <personal-access-token> on [this github page](https://github.com/settings/tokens/new). Don't forget to check `read:packages` permission.   
   > More details are described [here](https://docs.github.com/en/packages/working-with-a-github-packages-registry/working-with-the-container-registry#authenticating-to-the-container-registry).


4. Run `docker/utbot_docker_dev.sh` [script](https://github.com/UnitTestBot/UTBotCpp/blob/main/docker/utbot_docker_dev.sh).
   It will unpack the docker image and mount UTBot sources inside it. UTBot binary can be built in the docker and run in
   it. The script will prompt you to enter docker image tag. You can find the most recent
   tag [here](https://github.com/UnitTestBot/UTBotCpp/pkgs/container/utbotcpp%2Fbase_env), for example `24-11-2021`.

   Installer will ask about `ssh_port` on host machine. This port will be forwarded inside container's ssh port (by
   default `sshd` in container listens 2020). You may then login inside docker via `ssh utbot@host -p $ssh_port`,
   where `host` is a host machine IP address.
   > ⚠ Specify free port that is not in use on host machine!

   Also, you will be prompted to enter a gRPC port on host machine that will be forwarded inside container's 2121 port
   where UTBot listens gRPC requests.
   > Script will run docker image, mount specified folder on host filesystem into container's filesystem and forward ports for ssh and gRPC.

5. Login via ssh into newly started container
   ``` 
   ssh utbot@host -p $ssh_port 
   ``` 
   > You can type `localhost` as `host` if you are inside terminal of a host machine)
   > If you are prompted a password, enter `utbot`.

## Install UTBotCpp server from source

1. Clone UTBotCpp repository into home directory **inside docker container**, preferably with ssh.
2. `cd` into `UTBotCpp` directory and run `build.sh` — it is the script that builds KLEE UTBot and runs UTBot unit tests
3. Clone submodules `git submodule update --init --recursive`
4. Navigate to `UTBotCpp/server/build` directory and launch the binary with `./utbot server` command. Now the server is
   running.

## How to develop UTBotCpp with VS Code

1. Launch VS Code on your local machine. Use VS Code [Remote-SSH](https://code.visualstudio.com/docs/remote/ssh) to get
   access to the docker insides. Navigate to `UTBotCpp/server` directory and run `build.sh` script.
2. Install necessary
   plugins [C/C++ Extension Pack](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack)
   , [GDB Debug](https://marketplace.visualstudio.com/items?itemName=DamianKoper.gdb-debug)
3. Add debug configuration `Run -> Add configuration` and choose `(gdb) launch` set:
    * `"program": "${workspaceFolder}/build/utbot"`
    * `"args": ["server", "--verbosity=trace"]`
4. Configure from CMake
5. Build and Run

## How to develop UTBotCpp with an CLion

You can edit UTBot sources, rebuild and rerun the server.

To setup it:

1. Open UTBotCpp project in CLion on your local machine.
2. Open **Settings > Build, Execution, Deployment > Toolchains**.
3. Add a new *Remote Host* toolchain.
4. In *Credentials* section, set up an `ssh` connection to the UTBot docker container you have. Consider enabling
   connection via OpehSSH authentification agent to ease files synchronization.
5. 
    * Cmake: `/utbot_distr/install/bin/cmake`
    * Makes: `/usr/bin/make`
    * C Compiler: `/utbot_distr/install/bin/clang`
    * C++ Compiler: `/utbot_distr/install/bin/clang++`
6. Open **Settings > Build, Execution, Deployment > Deployment** in mappings Add new mapping from `/utbot_distr/gtest`
   to new local folder
7. Sync files
8. If you work on Windows, in docker install dos2unix and run `find . -type f -print0 | xargs -0 dos2unix` in UTBotCpp directory
9. In docker run `build.sh` from project directory
10. Load cmake from server/CMakeLists.txt

## Build VS Code plugin

1. Launch VS Code on your local machine. Use VS Code [Remote-SSH](https://code.visualstudio.com/docs/remote/ssh) to get
   access to the docker insides. Navigate to `UTBotCpp/vscode-plugin` directory and run `build.sh` script.
2. Press F5 (*Run Extension*). This will run UTBot VS Code plugin.
3. A new VS Code window will open; this window will have UTBot VS Code plugin enabled. In this new folder,
   open `UTBotCpp/integration-tests/c-example` directory.
4. When UTBot Quickstart Wizard requests you to enter server host and port, specify `localhost` and UTBot server run
   port 2121, respectively.
   ![Oops, something went wrong! Please look at wizardInstall.gif](https://raw.githubusercontent.com/UnitTestBot/unittestbot.github.io/source/resources/gifs/wizardInstall.gif "UTBot Wizard Demo")
5. Select project path as `/home/utbot/UTBotCpp/integration-tests/c-example`;
6. You are now ready to experience UTBot capabilities! You can view possible commands in Command Palette (Press F1 and
   type in UTBot).

If you want to change UTBot test generation preferences, you can edit them in File > Preferences > Settings >
Extensions > UnitTestBot. After UTBot configuration, you can select your source directories with the tab on the VSCode
toolbar on the left. Then, you can generate tests with the use of Command Palette. Press **F1** and type in "UTBot": You
will see tests generation options.

## [Troubleshooting](https://github.com/UnitTestBot/UTBotCpp/wiki/troubleshooting)
