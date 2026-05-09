# coding: utf-8

import os
import shutil
import tomllib
import time
import urllib.request
import ctypes
import subprocess
import sys
import zipfile
import stat
import ssl

# read config
config_dict = tomllib.load(open("build_config.toml", "rb"))
general_dict = config_dict["General"]
version = general_dict["version"]
is_clean: bool = bool(general_dict["is_clean"])
is_install_tools: bool = bool(general_dict["is_install_tools"])
is_uninstall_tools: bool = bool(general_dict["is_uninstall_tools"])

path_dict = config_dict["Path"]
ninja_path = path_dict["ninja_path"]
vcpkg_path = path_dict["vcpkg_path"]
mingw_path = path_dict["mingw_path"]
cmake_path = "cmake"

install_dict = config_dict["Install"]
install_path = install_dict["install_path"]
is_install: bool = bool(install_dict["is_install"])
is_general_setup: bool = install_dict["is_general_setup"]

tools_dict = config_dict["Tools"]

# ssl
try:
    _create_unverified_https_context = ssl._create_unverified_context
except AttributeError:
    pass
else:
    ssl._create_default_https_context = _create_unverified_https_context

ctx = ssl.create_default_context()
ctx.check_hostname = False
ctx.verify_mode = ssl.CERT_NONE
ctx.options |= ssl.OP_LEGACY_SERVER_CONNECT
opener = urllib.request.build_opener(urllib.request.HTTPSHandler(context=ctx))
urllib.request.install_opener(opener)

# 7z
def get_7z_executable():
    """检测系统中可用的 7z 命令行工具"""
    # find 7z
    for cmd in ["7z", "7zr"]:
        path = shutil.which(cmd)
        if path: return path

    default_paths = [
        r"C:\Program Files\7-Zip\7z.exe",
        r"C:\Program Files (x86)\7-Zip\7z.exe"
    ]
    for path in default_paths:
        if os.path.exists(path):
            return f'"{path}"'

    # install 7z
    temp_7z = os.path.abspath("7zr.exe")
    if not os.path.exists(temp_7z):
        print("未在系统中找到 7-Zip，正在下载临时解压组件...")
        url = "https://www.7-zip.org/a/7zr.exe"
        urllib.request.urlretrieve(url, temp_7z)

    return temp_7z

# mingw
def ensure_qt_mingw():
    # find mingw
    gcc_exe = shutil.which("gcc")
    if gcc_exe:
        try:
            version_out = subprocess.check_output([gcc_exe, "-dumpversion"]).decode().strip()
            if version_out.startswith("7.3"):
                print(f"Found compatible MinGW in PATH: {gcc_exe}")
                return True
        except:
            pass

    search_dirs = [
        os.path.abspath("external/mingw730_64"),
        r"C:\Qt\Qt5.14.2\Tools\mingw730_64",
        r"D:\Qt\Qt5.14.2\Tools\mingw730_64",
        r"C:\Qt\Tools\mingw730_64",
    ]

    config_mingw = tools_dict.get("mingw_path")
    if config_mingw:
        search_dirs.insert(0, config_mingw)

    for d in search_dirs:
        if os.path.exists(d):
            return True

    # install
    print("No compatible MinGW found. Starting automatic installation...")

    target_dir = os.path.abspath("external/mingw730_64")
    sz_cmd = get_7z_executable()

    archive_file = "mingw730_64.7z"
    mingw_url = tools_dict["mingw_url"]

    if not os.path.exists(archive_file):
        print(f"Downloading from {mingw_url}...")
        try:
            # download file
            urllib.request.urlretrieve(mingw_url, archive_file)

            file_size = os.path.getsize(archive_file)
            if file_size < 10 * 1024 * 1024:
                os.remove(archive_file)
                raise Exception(f"下载的文件过小 ({file_size} bytes)，可能下载已中断，请检查网络或更换镜像源。")

        except Exception as e:
            if os.path.exists(archive_file): os.remove(archive_file)
            print(f"Error downloading MinGW: {e}")
            return False

    if not os.path.exists(archive_file):
        print("Archive file missing, download might have failed.")
        return False

    print("Extracting MinGW (This may take a while)...")
    subprocess.run(f'{sz_cmd} x {archive_file} -o"external" -y', shell=True, check=True)
    shutil.move(r"external/Tools/mingw730_64", target_dir)
    shutil.rmtree(r"external/Tools")

    # clean
    if os.path.exists(archive_file):
        os.remove(archive_file)

    # add mingw to PATH
    os.environ["PATH"] = os.path.join(target_dir, "bin") + os.pathsep + os.environ["PATH"]
    return True

# ninja
def ensure_ninja():
    # check ninja
    ninja_path = shutil.which("ninja")
    local_ninja = os.path.abspath("external/ninja/ninja.exe")

    if ninja_path:
        print(f"Found Ninja in PATH: {ninja_path}")
        return ninja_path

    if os.path.exists(local_ninja):
        print(f"Found local Ninja: {local_ninja}")
        # add ninja to PATH
        os.environ["PATH"] = os.path.dirname(local_ninja) + os.pathsep + os.environ["PATH"]
        return local_ninja

    common_paths = [
        r"C:\Program Files\Ninja\ninja.exe",
        ninja_path
    ]
    for path in common_paths:
        if os.path.exists(path):
            print(f"Detected Ninja at: {path}")
            os.environ["PATH"] = os.path.dirname(path) + os.pathsep + os.environ["PATH"]
            return path

    # install
    print("Ninja not found. Downloading...")
    url = tools_dict["ninja_url"]
    zip_name = "ninja.zip"
    extract_dir = "external/ninja"

    if not os.path.exists(extract_dir):
        os.makedirs(extract_dir)

    try:
        urllib.request.urlretrieve(url, zip_name)
        with zipfile.ZipFile(zip_name, 'r') as zip_ref:
            zip_ref.extractall(extract_dir)
        os.remove(zip_name)

        # add ninja to PATH
        os.environ["PATH"] = os.path.abspath(extract_dir) + os.pathsep + os.environ["PATH"]
        print(f"Ninja installed to: {extract_dir}")
        return os.path.join(extract_dir, "ninja.exe")
    except Exception as e:
        print(f"Failed to download Ninja: {e}")
        return None

# cmake
def ensure_cmake():
    global cmake_path
    search_paths = [
        r"C:\Program Files\CMake",
        r"C:\Program Files (x86)\CMake",
        os.path.abspath("external/cmake-3.31.6-windows-x86_64")
    ]
    for d in search_paths:
        if os.path.exists(d):
            return True

    print("CMake not found. Downloading portable version...")
    url = tools_dict["cmake_url"]
    zip_file = "cmake.zip"
    extract_dir = "external/cmake"

    if not os.path.exists(extract_dir):
        urllib.request.urlretrieve(url, zip_file)
        with zipfile.ZipFile(zip_file, 'r') as zip_ref:
            zip_ref.extractall("external")
        os.remove(zip_file)

    # add cmake to PATH
    full_path = os.path.abspath(os.path.join("external", "cmake-3.31.6-windows-x86_64", "bin"))
    os.environ["PATH"] = full_path + os.pathsep + os.environ["PATH"]
    cmake_path = os.path.join(full_path, "cmake.exe")
    return True

# vcpkg
def ensure_vcpkg():
    vcpkg_root = os.path.abspath("external/vcpkg")
    vcpkg_exe = os.path.join(vcpkg_root, "vcpkg.exe")

    if os.path.exists(vcpkg_root):
        return vcpkg_root
    if os.path.exists(vcpkg_path):
        return vcpkg_path
    # clone
    print("vcpkg not found. Cloning repository...")
    subprocess.run(["git", "clone", tools_dict["vcpkg_url"], vcpkg_root], check=True)

    # install
    if not os.path.exists(vcpkg_exe):
        print("Bootstrapping vcpkg...")
        # Windows 下执行 .bat
        cmd = os.path.join(vcpkg_root, "bootstrap-vcpkg.bat")
        subprocess.run([cmd], cwd=vcpkg_root, check=True)

    return vcpkg_root

# NSIS
def ensure_nsis_installed():
    # check NSIS
    nsis_path = shutil.which("makensis")
    default_path = r"C:\Program Files (x86)\NSIS\makensis.exe"

    if nsis_path or os.path.exists(default_path):
        print("Done: NSIS is already installed.")
        return True

    print("NSIS not found. Starting automatic installation...")

    # admin
    if not ctypes.windll.shell32.IsUserAnAdmin():
        print("Error: Administrative privileges are required to install NSIS.")
        ctypes.windll.shell32.ShellExecuteW(None, "runas", sys.executable, " ".join(sys.argv), None, 1)
        return False

    # install NSIS setup
    nsis_installer = "nsis_setup.exe"
    nsis_url = tools_dict["nsis_url"]

    try:
        print(f"Downloading NSIS from {nsis_url}...")
        urllib.request.urlretrieve(nsis_url, nsis_installer)
    except Exception as e:
        print(f"Failed to download NSIS: {e}")
        return False

    # install NSIS
    print("Installing NSIS silently...")
    try:
        subprocess.run([nsis_installer, "/S"], check=True)
        print("NSIS installed successfully.")

        # add NSIS to PATH
        os.environ["PATH"] += os.pathsep + r"C:\Program Files (x86)\NSIS"

        # remove setup
        os.remove(nsis_installer)
        return True
    except Exception as e:
        print(f"Installation NSIS failed: {e}")
        return False

# uninstall
def handle_remove_readonly(func, path, excinfo):
    os.chmod(path, stat.S_IWRITE)
    func(path)

def uninstall_tools():
    cmake_dir = os.path.abspath("external/cmake")
    vcpkg_dir = os.path.abspath("external/vcpkg")

    # uninstall cmake
    if os.path.exists(cmake_dir):
        print(f"Removing CMake from {cmake_dir}...")
        shutil.rmtree(cmake_dir)

    # uninstall vcpkg
    if os.path.exists(vcpkg_dir):
        print(f"Removing vcpkg from {vcpkg_dir}...")
        try:
            shutil.rmtree(vcpkg_dir, onerror=handle_remove_readonly)
        except Exception as e:
            print(f"Error removing vcpkg: {e}")

    # uninstall ninja
    ninja_dir = os.path.abspath("external/ninja")
    if os.path.exists(ninja_dir):
        print(f"Removing Ninja from {ninja_dir}...")
        shutil.rmtree(ninja_dir, ignore_errors=True)
    print("Ninja uninstalled.")

    # uninstall mingw
    target_dir = os.path.abspath("external/mingw730_64")
    if os.path.exists(target_dir):
        print(f"正在卸载 MinGW: {target_dir}")
        shutil.rmtree(target_dir, ignore_errors=True)

    print("Tools uninstalled successfully.")

def uninstall_nsis_automatically():
    uninstaller = r"C:\Program Files (x86)\NSIS\uninst.exe"

    if not os.path.exists(uninstaller):
        print("NSIS uninstaller not found. It might have been uninstalled already.")
        return

    # admin
    if not ctypes.windll.shell32.IsUserAnAdmin():
        print("Error: Administrative privileges are required to uninstall NSIS.")
        return

    print("Uninstalling NSIS silently...")
    try:
        subprocess.run([uninstaller, "/S"], check=True)
        print("NSIS uninstallation command sent.")

        nsis_dir = r"C:\Program Files (x86)\NSIS"
        if os.path.exists(nsis_dir):
            import shutil
            shutil.rmtree(nsis_dir, ignore_errors=True)

    except Exception as e:
        print(f"Uninstallation failed: {e}")

start_time = time.time()

if is_install_tools:
    ensure_qt_mingw()
    ensure_cmake()
    vcpkg_path = ensure_vcpkg()
    ensure_nsis_installed()

build_start_time = time.time()
if is_clean:
    os.system(f'{cmake_path} --build cmake-build-release --target clean')

# build
os.system(
    fr'{cmake_path} -DCMAKE_BUILD_TYPE=Release "-DCMAKE_MAKE_PROGRAM={ninja_path}" '
    fr'-DCMAKE_TOOLCHAIN_FILE={vcpkg_path} -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic -DVCPKG_HOST_TRIPLET=x64-mingw-dynamic '
    fr'-DCMAKE_PREFIX_PATH=D:/Develop/QtDev/Qt5.14.2/5.14.2/mingw73_64 '
    fr'-DCMAKE_INSTALL_PREFIX=D:/Develop/Projects/nuitka-studio/{install_path} '
    fr'-G Ninja -S . -B cmake-build-release')
os.system('cmake --build cmake-build-release --target NuitkaStudio -j 30')

# install
if is_install:
    print("Installing...")
    os.system(f'{cmake_path} --install cmake-build-release')

# general setup
if is_general_setup:
    print("Generating installation package...")
    os.system('cd cmake-build-release && cpack -C Release')

build_end_time = time.time()

# uninstall tools
if is_uninstall_tools:
    print("Uninstalling build tools...")
    uninstall_tools()
    uninstall_nsis_automatically()

end_time = time.time()
print(f"Nuitka Studio {version} build successfully!")
print(f"All took {end_time - start_time:.3f} seconds. Build took {build_end_time - build_start_time:.3f} seconds.")
