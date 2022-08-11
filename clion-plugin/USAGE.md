
# How to use plugin

### Status bar icons
Before you start working with UnitTestBot, it's worth to pay attention to the IDE's status bar 
and UTBot-specific controls there:

<img src="images/plugin_usage/statusbar/status-bar-icons.png" alt="drawing" height="25"/>

#### Connection status

Connection status indicates whether the client and the server are 
established a connection. 
If the connection is lost, you'll see 
<img src="images/plugin_usage/statusbar/not-connected.png" alt="drawing" height="25"/>. 
In this case you need to check if UTBot container is still running.

If you click on the connection status icon, you'll see some actions 
that you can invoke:

<img src="images/plugin_usage/statusbar/status-bar-actions.png" alt="drawing" height="100"/>

#### Verbose mode

You can change the verbose option for generating tests from status bar. 
If verbose mode is disabled, you'll see:

<img src="images/plugin_usage/statusbar/verbose-off.png" alt="drawing" height="23"/> 

If it is enabled: 

<img src="images/plugin_usage/statusbar/verbose-on.png" alt="drawing" height="23"/> 

### Plugin description

![](images/plugin_usage/overview.png)

Main UI elements of plugin are:
- Status bar icons, which show connection status and verbose mode option
- UTBot consoles toolwindow, which allows to see logs from server (GTest log, Server log) and plugin (Client log)
- UTBot targets toolwindow, which shows targets found by server in current project
- UTBot Source Directories view which shows source directories with green color

#### Generate tests
You can trigger tests generation from
- Context menu in editor: 

<img src="images/plugin_usage/generate/editor-gen.png" alt="drawing"/> 

- Context menu in project view:

<img src="images/plugin_usage/generate/project-view-gen.png" alt="drawing" height="300"/> 

- Search window: pressing `Shift` two times and searching for action

<img src="images/plugin_usage/generate/gen-search.png" alt="drawing" height="300"/> 

