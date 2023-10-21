#!@KATIE_SHELL@

set -e

if [ -z "$LD_LIBRARY_PATH" ];then
    export LD_LIBRARY_PATH="@CMAKE_BINARY_DIR@/lib"
else
    export LD_LIBRARY_PATH="@CMAKE_BINARY_DIR@/lib:$LD_LIBRARY_PATH"
fi
export QT_PLUGIN_PATH="@CMAKE_BINARY_DIR@/plugins"

bin="$1"
shift

cd "$(dirname "$bin")"
exec gdb --args "./$(basename "$bin")" "$@"
