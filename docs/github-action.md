# GitHub Action

WIth [UnitTestBot GitHub Action](https://github.com/UnitTestBot/UTBotCpp-action) you can generate and run unit tests 
for your project directly on GitHub — and get the SARIF report.

## How to generate tests and a SARIF report with UnitTestBot GitHub Action:

1. On [GitHub.com](https://github.com/), navigate to the main page of the repository. Under your repository name, select **Settings**. In the 
left sidebar, select **Actions** > **General**.

[[images/github-action/gh-action-1.PNG|Configuring GitHub Action — Step 1]]

Scroll down and select **Allow GitHub Actions to create and approve pull requests**.

[[images/github-action/gh-action-2.PNG|Configuring GitHub Action — Step 2]]

2. Create a `.github/workflows` directory in your GitHub repository with a `.yml` file in it:

``` yml
name: "UnitTestBot code analysis"

on:
  workflow_dispatch:

jobs:
  build:
    runs-on: ubuntu-latest
    permissions: write-all
    steps:
      - name: UTBot code analysis
        uses: UnitTestBot/UTBotCpp-action@test-1.0.26
        with:
          add_tests: 'true'
          refresh_tests: 'false'
          utbot_version: '2022.12.0'
          scope: 'project'
```

Make sure you use the up to date versions:
* `uses: UnitTestBot/UTBotCpp-action@[version]}` — for [UnitTestBot GitHub Action](https://github.com/UnitTestBot/UTBotCpp-action/releases)
* `utbot_version: '[version]'` — for the [UnitTestBot](https://github.com/UnitTestBot/UTBotCpp/releases) engine

[[images/github-action/gh-action-yml.PNG|Adding workflow]]

3. To run the workflow, under your repository name, select **Actions**, then choose the UnitTestBot workflow. 

[[images/github-action/gh-action-actions.PNG|Choosing a workflow]]

In the 
**Run workflow** dropdown section, select the branch to use the workflow from. Press **Run workflow**.

[[images/github-action/gh-action-run.PNG|Run the workflow]]

## How to use the generated tests and the SARIF report:

UnitTestBot GitHub Action creates a new project branch named like `utbot-code-analysis-[short-commit-hash]` and a
pull request to a branch where you ran the workflow.

[[images/github-action/gh-action-pr.PNG|GitHub Action creating a PR]]

In this new branch, two new folders appear:
* the `tests` folder contains the generated tests with the related files,
* the `utbot_report` folder contains the SARIF report for a project (`project_code_analysis.sarif`) as well as the 
  generation statistics.

[[images/github-action/gh-action-branch.PNG|New branch with tests]]

To view the alerts, under your repository name, select **Security**, and in the left sidebar, select **Code 
scanning**.

[[images/github-action/gh-action-alerts.PNG|Code scanning alerts]]

View and manage code analysis results with the [standard GitHub tools](https://docs.github.com/en/code-security/code-scanning/automatically-scanning-your-code-for-vulnerabilities-and-errors/managing-code-scanning-alerts-for-your-repository).