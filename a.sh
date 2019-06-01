#!/bin/bash
cd ~/Documents/os2/problem2
make
adb push pagecopy.ko /data/os2
cd ~/Documents/os2/problem2/jni
ndk-build
cd ~/Documents/os2/problem2/libs/armeabi
adb push VATranslate /data/os2

cd ~/Documents/os2/problem2
make
adb push pagecopy.ko /data/os2
cd ~/Documents/os2/problem3/jni
ndk-build
cd ~/Documents/os2/problem3/libs/armeabi
adb push vm_inspector /data/os2

