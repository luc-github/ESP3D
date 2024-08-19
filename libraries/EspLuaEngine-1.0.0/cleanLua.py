import os
import glob
import shutil

# Lists of files to keep and disable
files_to_keep = [
    "lapi.c", "lauxlib.c", "lbaselib.c",
    "lcode.c", "lctype.c", "ldebug.c",
    "ldo.c","lfunc.c","lparser.c",
    "lgc.c", "llex.c", "lmem.c",
    "lobject.c", "lopcodes.c","lstring.c",
    "ltable.c","ltm.c","lstate.c",
    "lundump.c", "lundump.h", "lvm.c", "lzio.c", 
    "lua.hpp", "lstrlib.c", "ltablib.c","ldump.c","lutf8lib.c",
    "lmathlib.c"
]

files_to_disable = [
    "lua.c", "luac.c", "ldblib.c", "liolib.c", "loadlib.c", "loslib.c", "lcorolib.c","linit.c"
]

def find_lua_directory():
    current_dir = os.path.dirname(os.path.abspath(__file__))
    lua_dirs = glob.glob(os.path.join(current_dir, "src","lua*"))
    if lua_dirs:
        return os.path.join(lua_dirs[0], "src")
    return None

def rename_file(src, dst):
    try:
        shutil.move(src, dst)
        print(f"Renamed: {os.path.basename(src)} -> {os.path.basename(dst)}")
    except Exception as e:
        print(f"Error renaming {os.path.basename(src)}: {e}")

lua_src_dir = find_lua_directory()

if not lua_src_dir:
    print("Lua source directory not found.")
    exit(1)

print(f"Analyzing files in: {os.path.basename(lua_src_dir)}")

all_files = set(os.listdir(lua_src_dir))
files_to_keep_set = set(files_to_keep)
files_to_disable_set = set(files_to_disable)

#print("All files:", all_files)

# Files to keep
print("\nFiles to keep:")
for file in files_to_keep_set:
    if file in all_files:
        print(file)
    elif file + ".disabled" in all_files:
        print(f"Renaming {file}.disabled to {file}")
        rename_file(os.path.join(lua_src_dir, file + ".disabled"),
                    os.path.join(lua_src_dir, file))
    else:
        print(f"Warning: {file} not found in directory")

# Files to disable
print("\nFiles renamed to .disabled:")
for file in files_to_disable_set.intersection(all_files):
    if not file.endswith('.disabled'):
        rename_file(os.path.join(lua_src_dir, file),
                    os.path.join(lua_src_dir, file + ".disabled"))

# Files not in either list
other_files = all_files - set(file + ".disabled" for file in files_to_disable_set) - files_to_keep_set
print("\nFiles not in either list:")
for file in other_files:
    if not file.endswith('.disabled') and not file.endswith('.h') and file != "Makefile":
        print(file)

print("\nOperation completed.")