## How to build UTBotCpp from source

UTBot has many dependencies, so the easiest way to build the tool from source and develop it is to use the docker container.

UTBot has a published package called [base_env](https://github.com/UnitTestBot/UTBotCpp/pkgs/container/utbotcpp%2Fbase_env).
It contains all the needed dependencies such as Git, LLVM, GRPC, GoogleTest and so on. **base_env** has multiple versions tagged with dates.
If you are developing the tool, you are most likely to need the most recent version.

To build UTBot from sources:
1. Install [docker](https://docs.docker.com/engine/install/ubuntu/)
2. Run the command  
   ```
   docker plugin install lebokus/bindfs
   ```
3. You need to do `docker login` to `ghcr.io` as described [here](https://docs.github.com/en/packages/working-with-a-github-packages-registry/working-with-the-container-registry#authenticating-to-the-container-registry).
4. Run `utbot_docker_dev.sh` [script](https://github.com/UnitTestBot/UTBotCpp/blob/main/docker/utbot_docker_dev.sh). It will unpack the docker image and mount UTBot sources inside it.
   UTBot binary can be built in the docker and run in it.
   The script will prompt you to enter docker image tag. You can find the most recent tag [here](https://github.com/UnitTestBot/UTBotCpp/pkgs/container/utbotcpp%2Fbase_env), for example `24-11-2021`. Also it will ask about `ssh` port required to ssh into the container using `ssh utbot@host -p $port`, where `host` is docker host IP address (it may be `localhost`). Please, specify a port that is not taken by any process. Also, you will be prompted to enter a port where UTBot itself will be run.
5. Get access to the container via
   
  `ssh utbot@host -p $port` 

   If you are prompted a password, type in `utbot`.

5. Clone UTBotCpp repository into home directory inside docker container, preferably with ssh. And don't forget to clone modules with `git submodule update --init --recursive`
6. `cd` into `UTBotCpp` directory and run `build.sh` — it is the script that builds KLEE UTBot and runs UTBot unit tests
7. Navigate to `UTBotCpp/server/build` directory and launch the binary with `./utbot server` command. Now the server is running.
8. Launch VS Code on your local machine. Use VS Code [Remote-SSH](https://code.visualstudio.com/docs/remote/ssh) to get access to the docker insides. Navigate to `UTBotCpp/vscode-plugin` directory and run `build.sh` script.
9. Press F5 (*Run Extension*). This will run UTBot VS Code plugin.
10. A new VS Code window will open; this window will have UTBot VS Code plugin enabled. In this new folder, open `UTBotCpp/integration-tests/c-example` directory.
11. When UTBot Quickstart Wizard requests you to enter server host and port, specify `localhost` and UTBot server run port 2121, respectively.
   ![Oops, something went wrong! Please look at wizardInstall.gif](media/wizardInstall.gif "UTBot Wizard Demo")
12. Select project path as `/home/utbot/UTBotCpp/integration-tests/c-example`;
13. You are now ready to experience UTBot capabilities! You can view possible commands in Command Palette (Press F1 and type in UTBot).

If you want to change UTBot test generation preferences, you can edit them in  File > Preferences > Settings > Extensions > UnitTestBot.
After UTBot configuration, you can select your source directories with the tab on the VSCode toolbar on the left. Then, you can generate tests with the use of Command Palette. Press **F1** and type in "UTBot": You will see tests generation options.

## How to develop UTBotCpp with an IDE

You can edit UTBot sources, rebuild and rerun the server with an IDE. Usually, CLion *Remote host* toolchain is used for it.

To setup it:
1. Open UTBotCpp project with CLion on your local machine.
2. Open **Settings > Build, Execution, Deployment > Toolchains**.
3. Add a new *Remote Host* toolchain.
4. In *Credentials* section, set up an `ssh` connection to the UTBot docker container you have. Consider enabling connection via OpehSSH authentification agent to ease files synchronization.
5. All other fields should fill in automatically.


## Troubleshooting the build
### read -i invalid option
You can experience this on Mac OS:
```shell
read: -i: invalid option
```
It is because Mac OS has too old bash version. Upgrade bash version and set it to default:
```shell
brew install bash
chsh -s /usr/local/bin/bash
```
### Problem with gRPC
If you experience this problem:
```shell
--grpc_out: protoc-gen-grpc: Plugin failed with status code 1.
/bin/grpc_cpp_plugin: program not found or is not executable
```
```shell
ninja: error: 'protobuf/testgen.grpc.pb.h', needed by 'CMakeFiles/utbot.dir/main.cpp.o', missing and no known rule to make it
```
Copy the program to /bin:
```shell
cp /utbot_distr/install/bin/grpc_cpp_plugin /bin/
```
### Error while mounting empty volume
If this error happens on Mac OS:
```shell
docker: Error response from daemon: error while mounting volume '': VolumeDriver.Mount: exit status 1%!(EXTRA []interface {}=[]).
```
Remove the following line in utbot_docker_dev.sh: 
```shell
 -v $MOUNT_LOCAL_NAME:/home/utbot/mnt \
```