#!/usr/bin/python

import os
import subprocess

# Base directory of the script
script_path = os.path.abspath(__file__)

# Extract dir path
script_dir = os.path.dirname(script_path)

# Build path of sources dir : ../esp3d
base_dir = os.path.abspath(os.path.normpath(os.path.join(script_dir, '..', 'esp3d')))

# Parse all c, h , cpp files in all directories and sub directories
file_paths = []
for root, dirs, files in os.walk(base_dir):
    for file in files:
        if file.endswith(('.c', '.cpp', '.h', '.ino')):
           file_path = os.path.join(root, file)
           file_paths.append(os.path.abspath(os.path.normpath(file_path)))

# Now format all files one by one with clang-format
for file_path in file_paths:
    tmpPath = '"' + file_path + '"'
    print("Formating " + tmpPath, end="")
    try:
        command = ['clang-format', '-i', '--style=Google', file_path]
        subprocess.run(command, check=False)
        print("=> Ok")
    except subprocess.CalledProcessError as e:
        print(f'=>Error : {e}')
