
# How to use plugin

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

### Choose source folders

The server needs to know under what folders source files are.
The source folders are marked with green icons in the project view:

![](images/plugin_usage/source-folders.png)

You can mark/unmark selected folders from context menu. 

![](images/plugin_usage/mark-unmark-folders.gif)

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