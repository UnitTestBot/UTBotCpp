# UTBotCpp-CLion-plugin
<!-- Plugin description -->
Plugin for communication with UTBotCpp server in CLion.

### Installation

1. Clone UTBotCpp and cd there and do:
```
cd clion-plugin
./gradlew assembleDist
```
2. Open CLion, go to settings -> plugins
3. Click `Install Plugin from Disk...`: 
![](images/install/install-from-disk.png)
4. Navigate to plugin folder `clion-plugin`, then go to `build/distributions` 
and choose `zip` file:
![](images/install/choose-zip.png)
5. Restart CLion


### Configuration

#### Using quickstart wizard
When you open a plugin for the first time the quickstart wizard will appear.

![](images/plugin_usage/wizard.png)

You can open it manually at any time by clicking on connection status and choosing
`Show Quickstart Wizard`.

![](images/plugin_usage/show_wizard.png)

Or you can do it by pressing `shift` twice and finding an action: 


![](images/plugin_usage/find-wizard-action.png)

#### Using settings

You also can configure plugin manually in settings, where you will find
all configuration options.

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
you can specify path to project inside docker.


<!-- Plugin description end -->