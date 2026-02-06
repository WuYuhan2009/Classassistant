# BUILD

## Windows (Qt 5.15.2 + MSVC2019)
1. 安装 Qt 5.15.2 (MSVC2019 64bit)
2. `cmake -S . -B build -G "Ninja" -DCMAKE_PREFIX_PATH=C:/Qt/5.15.2/msvc2019_64`
3. `cmake --build build --config Release`

## Linux
1. 安装依赖：`qtbase5-dev libqt5svg5-dev`
2. `cmake -S . -B build`
3. `cmake --build build -j`
