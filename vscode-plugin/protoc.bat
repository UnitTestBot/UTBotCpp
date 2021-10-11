@echo off
set PROTODIR=%1
set JSDIR=%2

set PROTOC_GEN_TS_PATH="node_modules\.bin\protoc-gen-ts.cmd"
set GRPC_TOOLS_NODE_PROTOC_PLUGIN="node_modules\.bin\grpc_tools_node_protoc_plugin.cmd"
set GRPC_TOOLS_NODE_PROTOC="node_modules\.bin\grpc_tools_node_protoc.cmd"

if not exist %JSDIR% mkdir %JSDIR%

echo "Compiling proto files:"

for %%f in (%PROTODIR%\*) do (
    echo %%f
  %GRPC_TOOLS_NODE_PROTOC% ^
      --js_out=import_style=commonjs,binary:"%JSDIR%" ^
      --grpc_out="%JSDIR%" ^
      --plugin=protoc-gen-grpc="%GRPC_TOOLS_NODE_PROTOC_PLUGIN%" ^
      -I "%PROTODIR%" ^
      "%%f"

  %GRPC_TOOLS_NODE_PROTOC% ^
      --plugin=protoc-gen-ts="%PROTOC_GEN_TS_PATH%" ^
      --ts_out="%JSDIR%" ^
      -I "%PROTODIR%" ^
      "%%f"
)