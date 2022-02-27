# UTBotCpp-CLion-plugin
<!-- Plugin description -->
Plugin for communication with UTBotCpp for C\C++ tests generation in CLion.

### UTBotCpp

To use plugin some configuration steps are needed:

1. Open plugin settings in settings - Tools - UTBot Settings
2. Click `detect paths`. It will try to get source paths, build dir paths from CLion 
CMake model.
3. Specify absolute path to build folder, it should be different from build folder that CLion uses, 
because there can be conflicts between UTBotCpp and CLion. For example, if CLion uses `project_path/cmake-build-debug`, 
then you can specify `project_path/utbot_build`.
4. For target path specify `/utbot/auto/target/path`
5. Specify path to test folder.
6. Specify name of the server and port. 
7. If you use docker to run UTBotCpp and your project is mounted to docker, 
you can specify path to project inside docker and the plugin should work.


### Dummy server

To try plugin with dummy server that does not generate tests, do the following:

1. Run the `main` function in **GrpcServer.kt**.
2. Then run gradle task `Run Plugin`.
3. make sure that port in plugin settings and port in `main` function are the same.

The server was used for development, when UTBotCpp was private.
After availability of original server dummy server was not used,
but can be used for testing purposes.
<!-- Plugin description end -->