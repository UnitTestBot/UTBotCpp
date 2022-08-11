# Configuration

Before using UTBot some configuration is needed.

## Remote Scenario 
Depending on where UTBot runs configuration may take additional 
steps. If you run utbot not on the same host as CLion, 
synchronization of project files is necessary:

- If you use 
UTBot inside docker container, you can mount a project folder to Docker. 
- If you run UTBot under WSL, no additional configuration is needed, just 
specify remote path to project in settings or use Quickstart Wizard to 
fill it for you
- For a general case, when server is not running locally, CLion 
deployment can be used for synchronization, although it won't be 
fully automated, see below

### Configuring CLion SFTP deployment for remote scenario
To configure SFTP do the following:
- Open `Settings -> Build, Execution, Deployment -> Deployment`
- Create sftp configuration: click `+` -> choose/create ssh config with server host and port -> in mappings
  specify path to your project on the remote machine, it is the same path you specify in plugin settings for `remote path`
- Go to `options` and turn on upload on change:

![](images/plugin_usage/sftp/sftp-config.gif)

- Go to project view and right click on your project root, then
  choose deployment and click `upload to ...` and choose the name of your
  sftp configuration:

![](images/plugin_usage/sftp/upload.gif)

Now server can access your project files. When you change your files, 
CLion should upload changes to server. 

## Wizard

Wizard allows you to quickly configure your project, rather than 
specifying options manually in settings.
If you run UTBot remotely, be 
sure to setup synchronization as described in Remote Scenario section.

When you open your project for the first time, UTBot Wizard will be
opened.

![](images/plugin_usage/wizard/wizard-welcome.png)

### Connection
After the first introductory step, you will be asked to
fill server port, server host and remote path.
Remote path specifies path to project on remote machine.

If you run UTBot on WSL or Linux (locally) be sure 
to check the checkbox that fills in default path and port. 

![](images/plugin_usage/wizard/wizard-connection.png)

### Build Directory and CMake Options
In the end, UTBot will ask you to specify relative path to
the build directory and set custom CMake options:

![](images/plugin_usage/wizard/wizard-build-options.png)
### Demo

A demo on how to go through wizard:

![](images/plugin_usage/wizard/wizard-demo.gif)

## Plugin settings
The recommended way to configure plugin is by using UTBot Wizard. 
In settings more options are available for configuration.
You can view plugin settings in `Settings -> Tools -> UTBot Settings`

![](images/plugin_usage/settings-demo.gif)

### Remote path
Remote path specifies path to your project on remote host.
If you run UTBot locally (this is the case for linux) leave it empty.


## Check configuration
When you are connected to server, you can check project configuration 
to make sure you correctly specified all path. 

For that invoke `Configure project` action:

![](images/check-config/check-not.gif)

When project is configured you'll see the `Project is configured` notification:

![](images/check-config/check-ok.gif)

When you configured plugin and checked configuration, you can use it 
for generating tests, see `USAGE.md`.