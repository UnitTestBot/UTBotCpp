
# How to use plugin

### Check connection status
You must be connected to server to use plugin. \
Connection status can be checked in the status bar.

![](images/plugin_usage/connectionStatus.png)

### Configure project and generate json files

Your project must be configured by server to generate tests.

To send request for project configuration or json files generation,
click on `connection status` in the status bar and choose a suitable option:

![](images/plugin_usage/generateJson.png)

Plugin sends request for project configuration when you open 
the project. Sometimes configuration fails, then after changing
your project you need to send the request for project configuration.

### Choose source folders

The server needs to know the folders source files are located in.
The source folders are marked with green icons in the project view:

![](images/plugin_usage/source-folders.png)

You can mark/unmark selected folders from context menu. 

![](images/plugin_usage/mark-unmark-folders.gif)

### How to request tests generation

Open a c/cpp file, right click in the text editor and 
choose suitable option in the context menu:

![editor actions](images/plugin_usage/editorActions.png)

For example, to generate tests for the folder, right click on it in the project view
and choose `Generate for folder`:

![project view actions](images/plugin_usage/projectViewActions.png)

### See logs from server and client

You can see messages that are sent to the server and server own logging messages. Just click on the `UTBot consoles` tab in the bottom right corner:

![UTBot consoles tab](images/plugin_usage/consolesTab.png)

add choose the required tab:

![UTBot consoles view](images/plugin_usage/consolesToolWindow.png)