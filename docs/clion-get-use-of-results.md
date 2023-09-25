# Get use of test results

You can read the resulting tests, run them, and view coverage. The SARIF reports are also available.

## Read generated tests

You can find the resulting tests in the default `tests` directory unless you have changed it manually.

Test names look like `test_%filename%.cpp`.

[[images/clion/clion-test-folder-file.PNG|Test folder]]

Test are grouped into _regression_ or _error_ regions with the passing or failing tests correspondingly.

Regression region:

[[images/clion/clion-region-regression.PNG|Regression region]]

Error region:

[[images/clion/clion-region-error.PNG|Error region]]

Test structure follows the [Google Test format](http://google.github.io/googletest/).

According to settings, they may have short or verbose descriptions:
* Verbose descriptions include parameter declarations, expected and actual values, and the function call (as in the
  examples above).
* Short descriptions are concise checks suitable for utility functions with many branches to cover (see below).

[[images/clion/clion-concise-test.PNG|Concise test description]]

## Run tests with coverage

To run _the given test_ right from the **Editor**, use the **UTBot: Run with coverage** gutter icon near the test 
method name.

[[images/clion/clion-run-test.PNG|Running test from gutter]]

To run _all the tests in the file_, use the **Run all tests with coverage** gutter icon near `namespace UTBot`.

[[images/clion/clion-run-all.PNG|Running all tests from gutter]]

You can also go to **Navigate** > **Search Everywhere** and enter _UTBot: Run All Tests and Show Coverage_.

[[images/clion/clion-search-run-all-tests.PNG|Run all tests from Search Everywhere]]

After the test run, the line coverage information appears in the CLion gutter near the source code:
* green — covered
* yellow — partially covered
* red — uncovered

[[images/clion/clion-coverage-information.PNG|Coverage information]]

Each time you run tests with coverage, the coverage data is created, and you get a dialog: "Do you want to display 
coverage data for "UTBot coverage suite"?" 

Choose the required scenario:
* to display the merged coverage data from multiple runs — select **Add to active suites**
* to replace the initial coverage data with the current one — select **Replace active suites**
* to display the initial coverage data — select **Do not apply collected coverage**

[[images/clion/clion-coverage-scenarios.PNG|Choosing coverage scenarios]]

UnitTestBot C/C++ displays the test run results near the test headers. The green signs mark the passing tests, while 
the red ones mark the failing tests.

[[images/clion/clion-coverage-show-results.PNG|Showing test results in gutter]]

## Get SARIF reports

You can find the SARIF reports in the `utbot_report` directory.

### SARIF

SARIF is the Static Analysis Results Interchange Format — a standard, JSON-based format for the output of static
analysis tools. For more information, please refer to [SARIF Tutorials](https://github.com/microsoft/sarif-tutorials/blob/main/README.md).

You can view the `.sarif` files right in CLion **Editor**.

[[images/clion/clion-sarif.PNG|SARIF report]]


