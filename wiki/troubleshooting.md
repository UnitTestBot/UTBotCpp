<!---
name: Troubleshooting
route: /docs/cpp/develop/troubleshooting
parent: Documentation
menu: Develop
description: List of frequently problem   
--->

## Troubleshooting the build

### Read -i invalid option

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

### Failed to load project in CLion

If error like this happens then you try load Cmake in CLion

```
CMake File API: C:\Users\user\work\UTBotCpp\server\cmake-build-debug-remote-host-ml: target-gmock-Debug-0b6fa789e179f468efb4.json: no CMakeLists.txt file at '\utbot_distr\gtest\googlemock\CMakeLists.txt'
```

[check solution in issue](https://youtrack.jetbrains.com/issue/CPP-27998#focus=Comments-27-5697854.0-0)

### Error when run plugin in debugger

If you experience this error:

```shell
command npm run watch terminated with exit code 1
```

Then there is compilation error and if it is not fixed the previous version of the code will run (debugger won't work
correctly)