# Generate tests with default configuration

Make sure the UnitTestBot C/C++ client is connected to the server, and try UnitTestBot C/C++ on a simple example
project (e.g.,
[c-example](https://github.com/UnitTestBot/UTBotCpp/tree/main/integration-tests/c-example))
or go ahead and apply the tool to your real-life code.

## Build and configure your project

As soon as you perform the initial setup, two messages appear in the notification area (lower right):

1. _"UTBot: Build directory not found!
   Build directory does not exist on server filesystem."_ — select **Generate build directory**.

[[images/clion/clion-build-notification.PNG|Build notification]]

2. _"UTBot: Project not configured
   File compile_commands.json is missing in the build directory."_ — select **Generate Missing JSON Files**.

[[images/clion/clion-configure-notification.PNG|Configuration notification]]

You can also configure the project later: in the **Status Bar** (lower right), select the **UTBot** widget and choose
**Configure Project**.

[[images/clion/clion-configure-widget.PNG|Configure project from widget]]

## Generate tests

With UnitTestBot C/C++ you can generate tests for a project, a folder, a file, a function, or a line; tests
with the prompted result, and tests that fail an assertion.

In the CLion **Project** tool window, select the folder or a file, right-click and choose the required command:
* **UTBot: Generate Tests For Current File** (for a selected file only)

[[images/clion/clion-generate-for-file-project.PNG|Generating tests for a file]]

* **UTBot: Generate Tests For Folder**
* **UTBot: Generate Tests For Project**

[[images/clion/clion-generate-for-folder-project.PNG|Generating tests for a folder]]

In the **Editor**, place the caret at the function body. Right-click, scroll down the menu, select
**UTBot Generate Tests...** and choose the required option:

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

[[images/clion/clion-generate-from-editor.PNG|Generating tests from Editor]]

You can also generate tests for code snippet, which is unrelated to a project: open the snippet, go to **Navigate** > 
**Search Everywhere** and enter _UTBot: Generate Tests For Code Snippet_.

[[images/clion/clion-generate-for-snippet.PNG|Generating tests for code snippets]]

All these commands are available via **Search Everywhere**: go to **Navigate** > **Search Everywhere** and enter 
_UTBot: Generate Tests_.

[[images/clion/clion-search-all-commands.PNG|All commands for generating tests]]

_Note:_ it may take time to analyze a big project — check the progress bar. Once analysis is completed, the `tests` 
folder appears in your project folder: test names look like `test_%filename%.cpp`.

[[images/clion/clion-generation-in-progress.PNG|UnitTestBot in progress notification]]