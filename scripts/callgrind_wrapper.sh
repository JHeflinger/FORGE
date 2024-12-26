


RUNTIME_BIN=$1
RUNTIME_DIR=$(echo "$RUNTIME_BIN" | rev | cut -d'/' -f 2- | rev)

mkdir "$RUNTIME_DIR/profiles_callgrind"
valgrind --tool=callgrind  --trace-children=yes --dump-instr=yes --trace-jump=yes --callgrind-out-file=profiles_callgrind/callgrind.out.%pD "$RUNTIME_BIN"
cd "$RUNTIME_DIR/profiles_callgrind"
kcachegrind $(ls | tr "\n" " ")


