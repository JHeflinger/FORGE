@echo off
call ./help.bat audit
"./basilisk_executables/bazelisk-windows-amd64.exe" build --compiler=clang-cl --extra_toolchains=@local_config_cc//:cc-toolchain-x64_windows-clang-cl --extra_execution_platforms=//:x64_windows-clang-cl forge
