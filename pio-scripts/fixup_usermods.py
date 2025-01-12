Import('env')

# Patch up each usermod's include folders to include anything referenced by wled
# This is because usermods need to include wled.h
lib_builders = env.GetLibBuilders()
um_deps = [dep for dep in lib_builders if "/usermods" in dep.src_dir]
other_deps = [dep for dep in lib_builders if "/usermods" not in dep.src_dir]

for um in um_deps:
    # Add include paths for all non-usermod dependencies
    for dep in other_deps:
        for dir in dep.get_include_dirs():
            um.env.PrependUnique(CPPPATH=dir)
    # Add the wled folder to the include path
    um.env.PrependUnique(CPPPATH=env["PROJECT_SRC_DIR"])
       
#raise RuntimeError("debug")
