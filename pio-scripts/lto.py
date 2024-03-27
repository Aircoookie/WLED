Import('env')

env.Replace(AR=env['AR'].replace('elf-ar', 'elf-gcc-ar'))
env.Replace(RANLIB=env['RANLIB'].replace('elf-ranlib', 'elf-gcc-ranlib'))

# Something later clobbers AR & RANLIB, so until https://github.com/platformio/platform-espressif32/pull/1329
# is available, wrap the replace function to protect them

# Save a reference to the original env.Replace()
original_replace = env.Replace

def create_replace_wrapper(env):
    def replace_wrapper(**kw):
        if 'AR' in kw:
            kw.pop("AR")
        if 'RANLIB' in kw:
            kw.pop("RANLIB")

        original_replace(**kw)
    
    return replace_wrapper

# Replace the env.Replace with the wrapper
env.Replace = create_replace_wrapper(env)
