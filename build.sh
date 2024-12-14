if [ ! -e "$1" ]; then
    echo "Usage: $0 <path-to-obj-dir>"
    exit 1
fi
script_dir=$(dirname "$(realpath "$0")")
cmake -B "$script_dir/build/" "$script_dir"
cmake --build "$script_dir/build/" --config Release -j $(nproc)
cp "$script_dir/build/libxgrammar.so" "$1"
