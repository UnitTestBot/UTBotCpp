# Install and configure the UnitTestBot C/C++ plugin

<!-- TOC -->
  * [Install the plugin](#install-the-plugin)
  * [Open the project](#open-the-project)
  * [Configure the plugin using _UTBot: Quickstart_ wizard](#configure-the-plugin-using-utbot-quickstart-wizard)
    * [On the same machine or via WSL](#on-the-same-machine-or-via-wsl)
    * [On different machines or via Docker](#on-different-machines-or-via-docker)
      * [Configure an SFTP connection](#configure-an-sftp-connection)
      * [Upload the project to the remote server](#upload-the-project-to-the-remote-server)
      * [Perform the initial plugin setup](#perform-the-initial-plugin-setup)
<!-- TOC -->

## Install the plugin

To install the plugin from the ZIP file:

1. Download a ZIP archive with the release artifact. You can choose the release on [GitHub](https://github.com/UnitTestBot/UTBotCpp/releases). Unarchive this ZIP file.
2. In your CLion, go to **File** > **Settings**.
3. Select **Plugins**, click the gear icon and then **Install Plugin from Disk**.

[[images/clion/clion-install-from-disk.PNG|Install plugin from disk]]

4. In the folder with the unarchived files from the release artifact, select the `clion_plugin.zip` file an and click **OK**.
5. On **Plugins**, click **OK** to apply the changes and restart your IDE if prompted.

To check if the plugin is enabled, go to **File** > **Settings** > **Plugins** and choose **Installed**.

[[images/clion/clion-plugin-installed.PNG|PLugin installed]]

## Open the project

Go to **File** > **Open...** and select the project to generate tests for.

[[images/clion/clion-open-project.PNG|Open project]]

## Configure the plugin using _UTBot: Quickstart_ wizard

The UnitTestBot C/C++ server and the client can be installed on the same or on different machines. WSL and Docker
also behave differently: WSL automatically forwards the source paths to the server host,
while Docker requires establishing an SSH connection.

Choose one of two paths in the wizard to start using UnitTestBot C/C++:
* On the same machine or via WSL
* On different machines or via Docker

To launch the _UTBot Quickstart_ wizard, go to **Navigate** > **Search Everywhere** and enter _UTBot Quickstart_. 
Click on it.

[[images/clion/clion-search-wizard.PNG|Search everywhere: UTBot Quickstart]]

You can also find the wizard in the **Status Bar** (lower right): select the **UTBot** widget and choose 
**Quickstart wizard**.

[[images/clion/clion-utbot-widget.PNG|UTBot widget: UTBot Quickstart]]

### On the same machine or via WSL

If the UnitTestBot C/C++ server and the client are installed _on the same machine_, or the server is installed
_via WSL_ (without using Docker), make sure the **Default server configuration on localhost (or WSL2)** checkbox in 
the wizard is selected.

The default settings are:
* **Host:** `localhost`
* **Port:** `2121`
* **Remote Path:** autofilled
* **Build Directory:** `build`
* **CMake options:** <br>`-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`</br><br>`-DCMAKE_EXPORT_LINK_COMMANDS=ON`</br>

[[images/clion/clion-wizard-default-step-1.PNG|UTBot Quickstart wizard: default configuration — Step 1]]

Follow the default procedure — press **Next** until configuration is complete and proceed to generating tests.

[[images/clion/clion-wizard-default-step-2.PNG|UTBot Quickstart wizard: default configuration — Step 2]]

### On different machines or via Docker

If the UnitTestBot C/C++ server and the CLion plugin are installed on different machines, or the server is
installed via Docker, you have to use the [Deployment](https://www.jetbrains.com/help/clion/deploying-applications.html) feature.

#### Configure an SFTP connection

Go to **Settings** > **Build, Execution, Deployment** > **Deployment**.
* On the **Connection** tab, select the plus sign to create an SFTP connection (`UTBot server`): 
   * For **Type**, choose **SFTP**.

[[images/clion/clion-sftp.PNG|Configuring SFTP connection]]

   * For **SSH configuration**, select a browse button.

[[images/clion/clion-sftp-browse.PNG|Creating SFTP connection]]

   * In **SSH Configurations**, enter your 
      information:

      * **Host:** `127.0.0.1`
      * **Username:** `utbot`
      * **Port:** `5522`
      * **Authentication type:** **OpenSSH config and authentication agent**

[[images/clion/clion-sftp-ssh-config.PNG|SSH configurations]]

   * For the **Root path**, select **Autodetect** (e.g., you get `/home/utbot`).

[[images/clion/clion-sftp-ssh-config-connection.PNG|Configuring SSH connection]]

* On the **Mappings** tab, specify the path to your project on the remote machine:

[[images/clion/clion-sftp-ssh-config-mapping.PNG|Deployment path]]

#### Upload the project to the remote server

In the **Project** tool window, right-click on your project root folder, select **Deployment** > **Upload to...** and 
choose the name of your SFTP configuration (e.g., `UTBot server`).

[[images/clion/clion-upload-to-server.PNG|Upload to server]]

#### Perform the initial plugin setup

Launch the _UTBot Quickstart_ wizard: go to **Navigate** > **Search Everywhere** and enter _UTBot Quickstart_. Click on it.
Make sure the **Default server configuration on localhost (or WSL2)** checkbox is cleared.

[[images/clion/clion-wizard-remote-step-1.PNG|Configuring plugin]]

The default settings are:
* **Host:** `127.0.0.1`
* **Port:** `5521`
* **Remote Path:** `/home/utbot/remote/c-example`
* **Build Directory:** `build`
* **CMake options:** <br>`-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`</br><br>`-DCMAKE_EXPORT_LINK_COMMANDS=ON`</br>

Follow the default procedure — press **Next** until configuration is complete.

The UnitTestBot C/C++ plugin builds and configures the project under test automatically — see notifications.
