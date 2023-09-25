<!---
name: UTBot Logging Principles
route: /docs/cpp/advanced/utbot-logging
parent: Documentation
menu: Advanced
description: UTBot writes logs for most of the operations it executes. The server generates logs and temporary files while tests generation. Client logs monitor requests sent to server and received responses.
--->

# UTBot logging principles

## Server log

The server generates two types of data: logs and temporary files needed to generate tests. Log data is written
into `logs` folder placed inside the docker container. The path in which folder is created can be managed by
passing `--log /my/path` option to UnitTestBot binary. The default path for this option is the home
folder: `/home/{$USERNAME}/logs`.

The same principles are applied to the temporary directory `tmp`, in which the artifacts are placed and which path you
can control via `--tmp /my/path` option.

The `logs` folder structure is represented in the following way:

* logs
    * client1
        * everything.log
        * latest_readable.log
        * project1
            * stage1
                * timestamp.log
            * stage2
                * timestamp.log
        * project2
    * client2
    * everything.log
    * latest_readable.log

As you can see, the server creates a subdirectory for each client. There are two files inside it: one of them stores
every log message written during the execution of the requests from this client, while the other stores **INFO** level
logs from the last session. There are **ERROR**, **WARNING**, **INFO** and **DEBUG** logging levels.

Besides writing server log to files, UTBot sends the same logs to clients so they can be viewed in VS Code directly. You
can find them in a tab called **UTBot: Server Log**.

![utbotServerLogImg](https://github.com/UnitTestBot/unittestbot.github.io/raw/source/resources/images/utbotServerLog.png)

Logging level can be changed using the **UTBot: Log Settings** menu item.

![logLevelGif](https://github.com/UnitTestBot/unittestbot.github.io/raw/source/resources/gifs/logLevel.gif)

## Client logs

Client logs monitors sent requests and received responses. They can be viewed in **UTBot: Client Log** tab.

![utbotClientLogImg](https://github.com/UnitTestBot/unittestbot.github.io/raw/source/resources/images/utbotClientLog.png)
