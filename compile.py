from pathlib import Path
import subprocess as sp
import tarfile
import argparse
import os
import shutil

import requests


root_path: Path = Path(__file__).parent
default_install_path: Path = root_path / "bin"
default_build_dir: Path = root_path / "build"
default_lib_dir: Path = root_path / "lib"
default_cc: str = "gcc"
default_c_flags: str = "-w -O3"
default_cxx: str = "g++"
default_cxx_flags: str = "-w -O3"
#TODO win/linux
default_cmake_generator: str = "MinGW Makefiles"


def clone_repository(rep_name: str, dest: Path) -> bool:
    """Clones git repository into destination directory, returns cloned directory path
    :param rep_name: str, Name of github reository in form <username>/<projectname>
    :param dest: Path, Destination directory to put cloned repository
    :returns bool
    """

    if '/' not in rep_name:
        print("[!] Wrong repository format. Expected <username>/<projectname>")
        return False

    rep_usr, rep_dir = rep_name.split('/')
    if dest.exists():
        print(f"[i] Directory {dest} is not empty, download is skipped")
        return True

    print(f"[*] Cloning {rep_name} into {dest}...")
    job_clone_rep = sp.run(["git", "clone", f"https://github.com/{rep_name}", dest])

    if job_clone_rep.returncode == 0:
        print(f"[*] Done")
        return True

    print(f"[!] Error occurred cloning {rep_name}")
    return False


def find_ccfits(path: Path) -> bool:
    """Looks for extracted CCfits library in path, removes version number on folders if necessary
    :param path: Path to search CCfits in
    :returns bool
    """
    # Find existing CCfits library if present
    glob = list(path.glob("CCfits", case_sensitive=True))
    if glob:
        print(f"[*] Found CCfits at {glob[0]}")
        return True

    # Find extracted CCfits library with version: Remove version number
    glob = list(path.glob("CCfits-*", case_sensitive=True))
    if glob:
        print("[*] Found extracted files, renaming...")
        glob[0].rename(glob[0].with_name("CCfits"))
        return True
    return False


def extract_ccfits(tar_path: Path, dest: Path) -> bool:
    """Extracts all data from tar_path into dest directory"""

    # Test if ccfits library is present
    if result := find_ccfits(dest):
        print("[i] CCfits was already extracted")
        return result

    # Extract all data
    print(f"[*] Extracting ccfits from {tar_path} to {dest}")
    with tarfile.open(tar_path) as tar_file:
        tar_file.extractall(path=dest.absolute())
    print("[*] Done")

    # Rename CCfits directory if necessary
    return find_ccfits(dest)


def download_ccfits(dest: Path, version: str = "2.7") -> bool:
    """
    Downloads ccfits library from https://heasarc.gsfc.nasa.gov/FTP/software/fitsio/ccfits/CCfits-2.7.tar.gz
    into directory <dest>/ccfits_download/ccfits_tmp.tar.gz and extracts it into <dest>. If downloaded files
    are found, downloaded files will only be extracted.
    :param dest: Path to destination of extracted files
    :param version: str declaring version of ccfits
    :returns Path: to downloaded content
    """
    file_dest = dest / "ccfits_download/"
    tar_file = file_dest / "ccfits_tmp.tar.gz"

    if file_dest.exists():
        print(f"[i] Directory {file_dest} is not empty, download is skipped")
        return extract_ccfits(tar_file, dest)

    response = requests.get(f"https://heasarc.gsfc.nasa.gov/FTP/software/fitsio/ccfits/CCfits-{version}.tar.gz", stream=True)
    file_dest.mkdir(parents=True, exist_ok=True)

    print("[*] Downloading ccfits...")
    print("[  0%] Downloading...", end="\r")

    with tar_file.open("wb") as file:
        last_percent = 0
        progress = 0
        file_size = int(response.headers["Content-Length"])

        for chunk in response.iter_content(chunk_size=1024):
            file.write(chunk)

            progress += len(chunk)
            percent = int((progress / file_size) * 100)
            if percent > last_percent:
                last_percent = percent
                print(f"[{last_percent: >3}%]", end="\r")

    print("[*] Done")
    return extract_ccfits(tar_file, dest)


def run_cmake( src_dir: Path, build_dir: Path, install_path: Path,
        prefix_path: list[Path], cmake_options: list[str]) -> bool:

    if (sp.run([
        "cmake", f"-S {src_dir.relative_to(root_path)}", f"-B {build_dir.relative_to(root_path)}",
        f"-G {default_cmake_generator}",
        "-DCMAKE_POLICY_VERSION_MINIMUM=3.5",               # Used for cfitsio
        f"-DCMAKE_INSTALL_PREFIX={install_path.relative_to(root_path)}",
        f"-DCMAKE_PREFIX_PATH={';'.join(map(lambda p: str(p), prefix_path))}",
        *[f"-D{o}" for o in cmake_options]
    ], env=os.environ)).returncode != 0:
        return False

    prefix_path.append(install_path.absolute().as_posix())

    if (sp.run([
        "cmake", "--build", build_dir.relative_to(root_path).as_posix(),
    ], env=os.environ)).returncode != 0:
        return False

    if (sp.run([
        "cmake", "--install", build_dir.relative_to(root_path).as_posix(),
    ], env=os.environ)).returncode != 0:
        return False

    return True

if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument("-c", "--compiler",
                        default="c++",
                        choices=["gcc", "g++", "c++", "cpp", "gpp", "gnu", "icc", "clang", "clang++"])
    parser.add_argument("-g", "--generator",
                        default="mingw",
                        choices=["mingw", "windows", "unix", "make", "ninja"])
    parser.add_argument("-p", "--profile",
                        default="release",
                        choices=["release", "debug", "fast-debug"])
    parser.add_argument("-d", "--download-only", action="store_true")

    # TODO: add paths
    # TODO: build all
    args = parser.parse_args()

    match args.generator:
        case "mingw" | "windows":
            default_cmake_generator = "MinGW Makefiles"

        case "unix" | "make":
            default_cmake_generator = "Unix Makefiles"

        case "ninja":
            default_cmake_generator = "Ninja"

        case _:
            print(f"Unknown cmake generator {args.generator}")
            exit(0)

    match args.compiler:
        case "icc":
            # TODO: Hosts
            profiles = {
                "debug": "-O1 -g3 -debug inline-debug-info",
                "fast-debug": "-O3 -parallel -ip -ipo -g3 -debug inline-debug-info",
                "release": "-O3 -parallel -ip -ipo -g" # + hosts
            }

        case "gcc" | "g++" | "c++" | "gnu" | "cpp" | "gpp":
            profiles = {
                "debug": "-O1 -g3 -Wall",
                "fast-debug": "-march=native -O3 -funroll-loops -g3 -Wall -flto=auto -fuse-linker-plugin -fuse-ld=gold",
                "release": "-march=native -O3 -funroll-loops -flto=auto -fuse-linker-plugin -fuse-ld=gold"
            }

            default_cc = "gcc"
            default_cxx = "g++"

        case "clang" | "clang++":
            profiles = {
                "debug": "-O1 -g3 -Wall",
                "fast-debug": "-march=native -O3 -g3 -Wall -flto=auto",
                "release": "-march=native -O3 -flto=auto"
            }

            default_cc = "clang"
            default_cxx = "clang++"

        case _:
            print(f"Unknown Compiler type {args.compiler}")
            exit(0)

    default_cxx_flags = profiles[args.profile]

    zlib_path = clone_repository("madler/zlib", default_lib_dir / "zlib")
    cfitsio_path = clone_repository("HEASARC/cfitsio", default_lib_dir / "cfitsio")
    ccfits_path = download_ccfits(default_lib_dir)

    if not zlib_path or not cfitsio_path or not ccfits_path:
        print("[!] Could not prepare all files")
        exit(1)

    print("[*] Dependencies Downloaded")

    if args.download_only:
        exit(0)

    print("[*] Compiling Dependencies...")
    # TODO: add optimization flags
    targets = {
        "zlib": ['ZLIB_BUILD_TESTING:BOOL=OFF', 'ZLIB_BUILD_STATIC:BOOL=OFF', 'ZLIB_INSTALL_COMPAT_DLL:BOOL=OFF'],

        "cfitsio": ['TESTS:BOOL=OFF', 'USE_CURL:BOOL=OFF', 'UTILS:BOOL=OFF'],

        "CCfits": []
    }

    prefix_path: list[Path] = []
    for target_name, cmake_options in targets.items():
        print(f"[*] Compile target {target_name}")
        print(f"\tCMake Options: {cmake_options}")
        if not run_cmake(
            src_dir=default_lib_dir / target_name,
            build_dir=default_build_dir / target_name,
            install_path=default_install_path / target_name,
            prefix_path=prefix_path,
            cmake_options=cmake_options):
            print(f"[!] Could not compile {target_name}, aborting")
            exit(1)

    print("[*] Compiled Dependencies")
    print(f"[i] For CMake configuration use:\n\t-DCMAKE_PREFIX_PATH=\"{';'.join(map(lambda p: str(p), prefix_path))}\""
          f"\n\t-DBUILD_TESTING=OFF")
    print("[*] Compiling Polaris...")

    if not run_cmake(
        src_dir=root_path / "src",
        build_dir=default_build_dir / "polaris",
        install_path=default_install_path / "polaris",
        prefix_path=prefix_path,
        cmake_options=["BUILD_TESTING=OFF"] ):
        print("[!] Could not compile polaris")
        print("[i] You might have to add ${CCFITS_INCLUDE_DIR} to target_include_directories in src/CMakeLists.txt")
        exit(1)

    print("[*] Compilation successful")
    print("[*] Copy Library Files...")

    dest_dir = default_install_path / "polaris/bin/"

    shutil.copyfile(default_install_path / "zlib/bin/libz.dll", dest_dir / "libz.dll")
    shutil.copyfile(default_install_path / "cfitsio/bin/libcfitsio.dll", dest_dir / "libcfitsio.dll")
    shutil.copyfile(default_install_path / "CCfits/lib/libCCfits.a", dest_dir / "libCCfits.a")

    # Polaris: -DBUILD_TESTING:BOOL="0"
    # print(f"[i] Windows users may have to copy *.dll files from {default_install_path.absolute()} into their install directory")
    print("[*] Done")
