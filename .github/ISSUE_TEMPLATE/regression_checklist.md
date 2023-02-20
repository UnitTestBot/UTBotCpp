---
name: Regression checklist
about: Checklist for smoke testing of release build
title: ''
labels: qa
assignees: ''
---

Get build from this run - https://github.com/UnitTestBot/UTBotCpp/actions/workflows/publish-utbot.yml

Setup dedicated server according to following manual - https://github.com/UnitTestBot/UTBotCpp/wiki/install-server
Setup local server according to following manual - https://github.com/UnitTestBot/UTBotCpp/wiki/docker-free-wsl2

_**Delete project folders with sample project on remote and local server if they are left from previous test runs.**_

VSCode:
- [ ] Install plugin (`Ctrl-Shift-P` - `Extensions: Install plugin from VSIX`)
- [ ] Verify version on plugin page (`Ctrl-Shift-X` search for `@installed unittestbot`)

- [ ] Open folder with unpacked attached sample project - UnitTestBot Wizard should be started
- [ ] SFTP and Sarif Viewer plugins are marked as installed on first page of the Wizard
- [ ] On Server configuration check that both local and server connection could be established.
- [ ] When cannot connect proper error should be shown
- [ ] Connect to remote server and complete the wizard - build folder created, project configured


- [ ] Open `lib\calc.c` file, generate tests for `div(int a, int b)` function - tests are generated, Sarif report is opened
- [ ] Run only one test - `regression` tests pass, `error` tests fails, coverage shown in `calc.c` file.
- [ ] Run all generated tests in test file - `regression` tests pass, `error` tests fails, coverage shown in `calc.c` file.


- [ ] Generate and run tests for items listed below. Before the test remove `tests` folder. After test verify proper number of tests files appeared and run one of them.
    - [ ] File `lib\str_utils.c`
    - [ ] Folder `src`
    - [ ] Project

- [ ] Verbose
    - [ ] On
    - [ ] Off

- [ ] Log windows (writing to log could be unstable - see #430)
    - [ ] UTBot: Client Log
    - [ ] UTBot: Server Log
    - [ ] UTBot: Test console

- [ ]  Open UTBot Explorer (at left pane)
- [ ] [Target window](https://github.com/UnitTestBot/UTBotCpp/wiki/targets)
- [ ] Source Code window

- [ ] CLion plugin - do simple test - install plugin - configure to work with local server generate and run some test. Additionally verifying that coverage is generated.
