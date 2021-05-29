from os.path import join, isfile
import re
Import("env")
# access to global construction environment
ROOT_DIR = env['PROJECT_DIR']
# configuration file
configuration_file = join(ROOT_DIR, "esp3d", "configuration.h")
if isfile(configuration_file):
    fh = open(configuration_file, 'r')
    for line in fh:
        entry = re.search('^#define(\s)*SD_DEVICE(\s)*ESP_SDFAT', line)
        if entry:
            print("Add SDFat library to path")
            env["LIBSOURCE_DIRS"].append("extra-libraries/SDFat")
    fh.close()
else:
    print("No configuration.h file found")
print(env["LIBSOURCE_DIRS"])
