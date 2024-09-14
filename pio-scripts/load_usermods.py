Import('env')
usermods = env.GetProjectOption("custom_usermods","")
if usermods:
  proj = env.GetProjectConfig()
  deps = env.GetProjectOption('lib_deps')  
  src_dir = proj.get("platformio", "src_dir")
  src_dir = src_dir.replace('\\','/')
  usermods = [f"{mod} = symlink://{src_dir}/../usermods/{mod}" for mod in usermods.split(" ")]  
  proj.set("env:" + env['PIOENV'], 'lib_deps', deps + usermods)  
