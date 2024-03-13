# Install Windows Subsystem for Linux (WSL)

Before installing WSL1 or WSL2 on you Windows 10 or Windows 11 machine, make sure virtualization is enabled:

1. Open **Task manager** (press **Ctrl+Alt+Del**), choose **Performance** and check if virtualization is enabled. If
   not, perform the following two steps.
3. Find **Turn Windows Features on or off** in Windows settings and turn on **Hyper-V** features.
5. Enable virtualization via BIOS.

To install WSL2 on you Windows 10 or Windows 11 machine:

1. Create an empty WSL2 container with Ubuntu 20.04:
```PowerShell
wsl --install -d Ubuntu-20.04
```
2. Perform the initial setup for WSL2 user (in this example, we use "utbot"):

> ```bash
> Installing, this may take a few minutes...
> Please create a default UNIX user account. The username does not need to match your Windows username.
> For more information visit: https://aka.ms/wslusers
> Enter new UNIX username: utbot
> Enter new UNIX password:
> Retype new UNIX password:
> passwd: password updated successfully
> Installation successful!
> To run a command as administrator (user "root"), use "sudo <command>".
> See "man sudo_root" for details.
> ```
3. Check if installation is successful: the new WSL2 container is called _Ubuntu-20.04_ by default:
```PowerShell
  C:\Users\user> wsl --list --verbose
  NAME                   STATE           VERSION
  Ubuntu-20.04           Running         2      <---------- Newly installed WSL2 container   
```
