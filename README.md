# Alakzatok példa

## Letöltés

    git clone https://github.com/szaszm/alakzatok.git
    cd alakzatok

## Fordítás Linuxon

    mkdir build
    cd build
    cmake ..
    make -j$(nproc)
    ./alakzatok

## Fordítás Windowson, Visual Studio / msbuild-del
Telepítsd a Visual Studio 2019 Community editiont, majd nyisd meg az "x64 Native Tools Command Prompt for VS 2019" programot. Ebben letöltés után:

    mkdir build
    cd build
    cmake ..
    cmake --build .
    Debug\alakzatok.exe
