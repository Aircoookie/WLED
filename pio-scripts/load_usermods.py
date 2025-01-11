Import('env')
import os

def find_usermod(mod_dir: str, mod: str):
  """Locate this library in the usermods folder.
     We do this to avoid needing to rename a bunch of folders;
     this could be removed later
  """
  # Check name match
  mp = f"{mod_dir}/{mod}"
  if os.path.exists(mp):
    return mp
  mp = f"{mod_dir}/usermod_v2_{mod}"
  if os.path.exists(mp):
    return mp
  raise RuntimeError(f"Couldn't locate module {mod} in usermods directory!")

usermods = env.GetProjectOption("custom_usermods","")
if usermods:
  proj = env.GetProjectConfig()
  deps = env.GetProjectOption('lib_deps')  
  src_dir = proj.get("platformio", "src_dir")
  src_dir = src_dir.replace('\\','/')
  mod_paths = {mod: find_usermod(f"{src_dir}/../usermods", mod) for mod in usermods.split(" ")}
  usermods = [f"{mod} = symlink://{path}" for mod, path in mod_paths.items()]
  proj.set("env:" + env['PIOENV'], 'lib_deps', deps + usermods)  
