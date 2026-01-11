# coding: utf-8

import os
import shutil
import tomllib

VERSION = "1.1.0"

config_dict = tomllib.load(open("config.toml", "rb"))
path_dict = config_dict["Path"]

ninja_path = path_dict["ninja_path"]
vcpkg_path = path_dict["vcpkg_path"]

is_debug: bool = bool(config_dict["General"]["is_debug"])
build_mode = "debug" if is_debug else "release"

os.system(
    fr'cmake -DCMAKE_BUILD_TYPE={"Debug" if is_debug else "Release"} "-DCMAKE_MAKE_PROGRAM={ninja_path}" '
    fr'-DCMAKE_TOOLCHAIN_FILE={vcpkg_path} -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic -G Ninja -S . -B cmake-build-{build_mode}')
os.system('cmake --build cmake-build-release --target NuitkaStudio -j 30')

if not is_debug:
    output_path = f'releases/NuitkaStudio{VERSION}'
    if os.path.exists(output_path):
        shutil.rmtree(output_path)
    os.makedirs(output_path)

    shutil.copyfile("cmake-build-release/NuitkaStudio.exe", f"{output_path}/NuitkaStudio.exe")
    shutil.copyfile("cmake-build-release/libfmt.dll", f"{output_path}/libfmt.dll")
    shutil.copyfile("cmake-build-release/libspdlog.dll", f"{output_path}/libspdlog.dll")

    os.chdir(output_path)
    # remove files
    os.system("windeployqt NuitkaStudio.exe")
    os.remove("Qt5Svg.dll")
    os.remove("libGLESv2.dll")
    os.remove("libEGL.dll")
    os.remove("D3Dcompiler_47.dll")
    os.remove("opengl32sw.dll")

    shutil.rmtree("iconengines")
    shutil.rmtree("imageformats")
    shutil.rmtree("styles")
    shutil.rmtree("translations")


print(f"Nuitka Studio {VERSION} build successfully!")
