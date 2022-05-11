
# How to use plugin

### Configure plugin
To configure plugin open setting in running Clion instance,
go to `Tools` -> `UTBot Settings`. See README.md for how to 
configure plugin.

![](images/plugin_usage/settings.png)

1. Click `detect paths`. It will try to get source paths, build dir paths from CLion
   CMake model of currently selected CMake configuration.
2. Specify absolute path to build folder, it should be different from build folder that CLion uses,
   because there can be conflicts between UTBotCpp and CLion. For example, if CLion uses `project_path/cmake-build-debug`,
   then you can specify `project_path/utbot_build`.
3. For target path specify `/utbot/auto/target/path`
4. Specify path to test folder.
5. Specify name of the server and port.
6. If you use docker to run UTBotCpp and your project is mounted to docker,
specify path inside docker to your project in field `remote path`
   you can specify path to project inside docker and the plugin should work.


### Check connection status
To use plugin you should be connected to server, you can check 
the connection status in status bar:

![](images/plugin_usage/connectionStatus.png)

### Configure project and generate json files
For test generation to work your project must be configured by server. 


To send request for project configuration or generation of json files
click on connection status in the status bar and choose suitable option:

![](images/plugin_usage/generateJson.png)

Plugin sends request for project configuration when you open 
the project. Sometimes configuration fails, and then after you changed
your project you need to send the request for project configuration.


### How to request tests generation
Open a c/cpp file, right click in the text editor and in context menu
choose suitable option:

![editor actions](images/plugin_usage/editorActions.png)

To generate tests for a folder right click on a folder in project view
and choose `Generate for folder`:

![project view actions](images/plugin_usage/projectViewActions.png)

### See logs from server and client
You can see what messages are sent to the server and server logging messages.
For that click on the `UTBot consoles` tab in the bottom right corner:

![UTBot consoles tab](images/plugin_usage/consolesTab.png)

add choose the needed tab:

![UTBot consoles view](images/plugin_usage/consolesToolWindow.png)