# Yadro

## Linux/MacOS
Тестировалось на MacOS, компилятор - g++ 12.2.0
Сборка через CMake :

```bash
mkdir build
cd build
cmake ..
make
```

Пример запуска программы:
```bash
./yadro file.csv
```

## Windows
Тестировалось через консоль msys2 mingw, компилятор - g++ 12.2.0
Сборка через CMake :
```bash
mkdir build
cd build
cmake -DCMAKE_CXX_COMPILER=g++ ..
cmake --build .
```

Пример запуска программы:
```bash
./yadro.exe file.csv
```
