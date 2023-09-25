# Install and configure the UnitTestBot C/C++ plugin

<!-- TOC -->
  * [Install the plugin](#install-the-plugin)
  * [Open the project](#open-the-project)
  * [Configure the plugin using _UTBot: Quickstart_ wizard](#configure-the-plugin-using-utbot-quickstart-wizard)
    * [On the same machine or via WSL](#on-the-same-machine-or-via-wsl)
    * [On different machines or via Docker](#on-different-machines-or-via-docker)
      * [Configure an SFTP connection](#configure-an-sftp-connection)
      * [Perform the initial plugin setup](#perform-the-initial-plugin-setup)
<!-- TOC -->

## Install the plugin

Make sure you have already [installed the UnitTestBot C/C++ server](install-server).

To install the plugin from the VSIX file:

1. Download a ZIP archive with the release artifact. You can choose the release on [GitHub](https://github.com/UnitTestBot/UTBotCpp/releases). Unarchive this ZIP file.
2. In your Visual Studio Code, go to **File** > **Preferences** > **Extensions** and select **Views and More Actions...**.
   Select **Install from VSIX**.

[[images/vscode/vscode-install-from-vsix.PNG|Installing from VSIX]]

3. In the folder with the unarchived files from the release artifact, select the `utbot_plugin.vsix` file and press
   **Install**.

Now the UnitTestBot C/C++ plugin is displayed in **File** > **Preferences** > **Extensions** > **Installed**.

[[images/vscode/vscode-installed.PNG|Plugin installed]]

## Open the project

Go to **File** > **Open Folder** and select the one containing the project to generate tests for.

[[images/vscode/vscode-open-folder.png|Opening folder]]

## Configure the plugin using _UTBot: Quickstart_ wizard

To start using UnitTestBot C/C++, follow the wizard:

* The wizard opens _automatically_, if you use UnitTestBot C/C++ for your project for the first time.
* To open the wizard _on demand_, go to **View** > **Command Palette...**.

[[images/vscode/vscode-command-palette.png|Command Palette]]

* Enter _Run UTBot: Quickstart Wizard_. Click on it.

[[images/vscode/vscode-command-palette-wizard.png|Running UnitTestBot Quickstart wizard from Command Palette]]

The UnitTestBot C/C++ server and the client can be installed on the same or on different machines. WSL and Docker
also behave differently: WSL automatically forwards the source paths to the server host,
while Docker requires establishing an SSH connection.
Choose one of two paths in the wizard to start using UnitTestBot C/C++.

### On the same machine or via WSL

If the UnitTestBot C/C++ server and the client are installed _on the same machine_, or the server is installed
_via WSL_ (without using Docker), make sure the **Default server configuration on localhost (or WSL2)** checkbox is 
selected.

[[images/vscode/vscode-wizard-default-step-1.PNG|UnitTestBot Quickstart wizard — default configuration — step 1]]

Follow the default procedure — press **Next** until configuration is complete.

[[images/vscode/vscode-wizard-default-step-2.PNG|UnitTestBot Quickstart wizard — default configuration — step 2]]

The default settings are:
* **Host:** `localhost`
* **gRPC port:** `2121`
* **SFTP port:** not used
* **Project Path On Server:** autofilled
* **Build Directory:** `build`
* **CMake options:** <br>`-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`</br><br>`-DCMAKE_EXPORT_LINK_COMMANDS=ON`</br>

### On different machines or via Docker

If the UnitTestBot C/C++ server and the client are installed _on different machines_, or the server is installed
_via Docker_, you need to sync files within a local directory to a remote server directory using SSH protocol.

#### Configure an SFTP connection

1. Install the [SFTP plugin](https://marketplace.visualstudio.com/items?itemName=liximomo.sftp) for your Visual Studio Code.
2. Go to **View** > **Command Palette...** and enter _SFTP: config_. Enter your values in `sftp.json`:

```json 
{
   "name": "UTBot Server",
   "host": "127.0.0.1", // IP address of the UnitTestBot server
   "protocol": "sftp",
   "port": 5522,
   "username": "utbot",
   "password": "utbot",
   "remotePath": "/home/utbot/remote/c-example/", // the path to your project on the server host
   "uploadOnSave": true,
   "useTempFile": false,
   "openSsh": false
}
``` 
3. Go to **View** > **Command Palette...** and enter _SFTP: Sync Local -> Remote_. Check **SFTP Explorer** for the 
   uploaded files.

[[images/vscode/vscode-sftp-explorer.PNG|Visual Studio Code SFTP Explorer]]

#### Perform the initial plugin setup

1. Go to **View** > **Command Palette...** and enter _Run UTBot: Quickstart Wizard_.
2. Clear the **Default server configuration on localhost (or WSL2)** checkbox.
3. Enter your values in the setting fields — the same as in `sftp.json`:

	* **Host:** your host name or IP address
	* **gRPC port:** `5521`
	* **SFTP port:** `5522`
	* **Project Path On Server:** the path to your project on the server host
	* **Build Directory:** `build`
	* **CMake options:** <br>`-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`</br><br>`-DCMAKE_EXPORT_LINK_COMMANDS=ON`</br>

[[images/vscode/vscode-wizard-remote-step-1.PNG|UnitTestBot Quickstart wizard — remote configuration — step 1]]

Follow the default procedure — press **Next** until configuration is complete.

The UnitTestBot C/C++ plugin builds and configures the project under test automatically — see notifications.
