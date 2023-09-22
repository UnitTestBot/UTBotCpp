<!---
name: FAQ
route: /docs/cpp/faq
parent: Documentation
description: In case you've faced some issues, we may have already detected them. This page describes all known problems with corresponding solutions. 
--->

# FAQ

### UTBot fails to build from docker

* invoke `source ~/.runtime_env.sh` shell command and try again.

### Quickstart Wizard tab did not open when you launched the plugin

* in VSCode inovke  `Run UTBot: Quickstart Wizard` command from the Command pallette `Shift + Ctrl + P` or `F1`

### Source file is not registered: /path/to/file/filename.c

* Open `UTBot explorer` from left corner or press F1 and type `View: Show UTBot explorer`
* In `UTBOT FOLDERS` mark source as UTBot Source Folder
  `src`, `lib`, `build\src`, `build\lib`

### File not found in compilation_commands.json

* in VSCode inovke  `UTBot: Reset cache and configure project` command from the Command pallette `Shift + Ctrl + P`
  or `F1`

### bear make -j

* in VSCode inovke  `UTBot: Reset cache and configure project` command from the Command pallette `Shift + Ctrl + P`
  or `F1`

### UNAVAILABLE: No connection established in vscode

* Check if utbot server is up
* in VSCode inovke `Developer: Reload Window` command from the Command pallette `Shift + Ctrl + P` or `F1`

### No logs, or incorrect logs

* kill all utbot server instance and run new one.
  `ps aux | grep "utbot server" | grep -v grep | awk '{print $2;}' | xargs kill`