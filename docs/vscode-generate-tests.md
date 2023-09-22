# Generate tests with default configuration

Make sure the UnitTestBot C/C++ client is connected to the server, and try UnitTestBot C/C++ on a simple example 
project (e.g.,
[c-example](https://github.com/UnitTestBot/UTBotCpp/tree/main/integration-tests/c-example)) 
or go ahead and apply the tool to your real-life code.

## Build and configure your project

When you open the project for the first time, the _UTBot: Quickstart_ wizard opens automatically. As soon as you 
perform the initial setup, two messages appear in the notification area (lower right):

1. _"Build folder "build" specified in Preferences, does not exist."_ — select **Create build folder**.

[[images/vscode/vscode-build-notification.PNG|Build notification]]

2. _"Project is not configured properly: 'compile_commands.json' is missing in the build folder. Please, 
follow the guide to fix it, or UTBot can try to configure it automatically."_ — select **Configure**.

[[images/vscode/vscode-configure-notification.PNG|Configuration notification]]

You can also configure the project later: go to **View** > **Command Palette...** and enter _UTBot: Configure 
project_ or _UTBot: Reset cache and configure project_.

[[images/vscode/vscode-command-palette-configure.png|Configuring from Command Palette]]

## Generate tests

With UnitTestBot C/C++ you can generate tests for a project, a folder, a file, a function, or a line; tests 
with the prompted result, and tests that fail an assertion.

In Visual Studio Code **Explorer**, select the folder or a file, right-click and choose the required command:
* **UTBot: Generate Tests For Current File** (for a selected file only)
* **UTBot: Generate Tests For Folder**
* **UTBot: Generate Tests For Project**

[[images/vscode/vscode-generate-from-explorer.PNG|Generating tests from Explorer]]

In the **Editor**, place the caret at the function body. Right-click, scroll down the menu, select 
**UTBot: Generate Tests...** and choose the required option:

- **for Current File**
> You can generate tests for `*.c` or `*.cpp` files.

- **for Current Function**
> Only public functions (declared in a corresponding header file) are considered as test targets.

- **for Current Line**
> You can generate tests that cover a specific line.

- **that Fail Current Assertion**
> We do not recommend to include tests that fail an assertion to a common test suite with regular checks as a failure
triggers the engine to terminate test execution.

- **with Prompted Result**
> UnitTestBot C/C++ cannot solve NP-hard problems, so do not use this feature on hash functions. Set up a 
> condition (an equation or inequation) the target output should satisfy: choose a proper operator (in C syntax) and insert a value (constants supported only).

[[images/vscode/vscode-generate-from-editor.PNG|Generating tests from Editor]]

You can also generate tests for code snippet, which is unrelated to a project: open the snippet, go to **View** > 
**Command Palette...** and enter _UTBot: Generate Tests For Code Snippet_.

[[images/vscode/vscode-command-palette-code-snippet.png|Generating tests for code snippets]]

All these commands are available via **Command Palette...**: go to **View** > **Command Palette...** and 
enter _UTBot: Generate Tests_.

[[images/vscode/vscode-command-palette-generate.png|All commands for generating tests]]

_Note:_ it may take time to analyze a big project. If the message in the notification area (lower right) stays active, 
analysis is still in progress. 

[[images/vscode/vscode-running.PNG|UnitTestBot in progress notification]]

Once it is completed, the `tests` folder appears in your project folder: test names 
look like `test_%filename%.cpp`.
