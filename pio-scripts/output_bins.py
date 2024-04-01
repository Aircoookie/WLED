Import('env')
import os
import shutil
import gzip

OUTPUT_DIR = os.path.join("build_output")

def _get_cpp_define_value(env, define):
    define_list = [item[-1] for item in env["CPPDEFINES"] if item[0] == define]

    if define_list:
        return define_list[0]

    return None

def _create_dirs(dirs=["map", "release"]):
    for d in dirs:
        os.makedirs(os.path.join(OUTPUT_DIR, d), exist_ok=True)

def create_release(source):
    release_name = _get_cpp_define_value(env, "WLED_RELEASE_NAME")
    if release_name:
        version = _get_cpp_define_value(env, "WLED_VERSION")
        release_file = os.path.join(OUTPUT_DIR, "release", f"WLED_{version}_{release_name}.bin")
        release_gz_file = release_file + ".gz"
        print(f"Copying {source} to {release_file}")
        shutil.copy(source, release_file)
        bin_gzip(release_file, release_gz_file)

def bin_rename_copy(source, target, env):
    _create_dirs()
    variant = env["PIOENV"]

    # create string with location and file names based on variant
    map_file = "{}map{}{}.map".format(OUTPUT_DIR, os.path.sep, variant)

    create_release(str(target[0]))

    # copy firmware.map to map/<variant>.map
    if os.path.isfile("firmware.map"):
        shutil.move("firmware.map", map_file)

def bin_gzip(source, target):
    # only create gzip for esp8266
    if not env["PIOPLATFORM"] == "espressif8266":
        return
    
    print(f"Creating gzip file {target} from {source}")
    with open(source,"rb") as fp:
        with gzip.open(target, "wb", compresslevel = 9) as f:
            shutil.copyfileobj(fp, f)

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", bin_rename_copy)
