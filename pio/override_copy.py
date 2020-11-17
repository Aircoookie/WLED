Import('env')
import os
import shutil

# copy WLED00/user_config_override_sample.h to WLED00/user_config_override.h
if os.path.isfile("wled00/user_config_override.h"):
    print ("*** use provided user_config_override.h as planned ***")
else: 
    shutil.copy("wled00/user_config_override_sample.h", "wled00/user_config_override.h")
