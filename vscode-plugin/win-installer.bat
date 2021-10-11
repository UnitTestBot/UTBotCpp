REM This script only needed by developers who compiles vscode plugin on windows
call npm install
call protoc.bat ..\server\proto src\proto-ts
