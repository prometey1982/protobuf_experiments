conan install . --build=missing --profile:host=debug_host
conan install . --build=missing --profile:host=default_host
cmake --preset conan-default -A Win32

rem cmake -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_PREFIX_PATH="C:\Qt\5.15.2-x32-msvc2022-static" .