#!/usr/bin/python

import os
import subprocess
from shutil import which

def format_sources():
    """
    Format JavaScript and CSS files using Prettier.

    This script locates all JavaScript and CSS files in the 'src' and 'config' directories
    (including subdirectories) and formats them using the Prettier tool. It requires Node.js
    and the Prettier package to be installed.

    Returns:
        None
    """
    # Base directory of the script
    script_path = os.path.abspath(__file__)

    # Extract dir path
    script_dir = os.path.dirname(script_path)

    # Build paths of sources dirs: ../src and ../config
    src_dir = os.path.abspath(os.path.normpath(os.path.join(script_dir, '..', 'src')))
    config_dir = os.path.abspath(os.path.normpath(os.path.join(script_dir, '..', 'config')))

    # Parse all c, h , cpp, js, css files in all directories and sub directories
    file_paths = []
    for base_dir in [src_dir, config_dir]:
        for root, dirs, files in os.walk(base_dir):
            for file in files:
                if file.endswith(('.js', '.css')):
                    file_path = os.path.join(root, file)
                    file_paths.append(os.path.abspath(os.path.normpath(file_path)))
                    print(os.path.abspath(os.path.normpath(file_path)))

    # Locate the Prettier binary
    node_path = which('node')
    print(node_path)
    if not node_path:
        print("node not found in PATH. Please install it globally or locally.")
        exit(1)
    node_dir = os.path.dirname(node_path)
    client_path = os.path.join('node_modules', 'npm', 'bin', 'npx-cli.js')
    print(client_path)

    # Now format all files one by one with prettier
    prettierrc_path = os.path.abspath(os.path.normpath(os.path.join(script_dir, '..', '.prettierrc')))
    print("Using:" + prettierrc_path)
    for file_path in file_paths:
        tmpPath = file_path
        print("Formating " + tmpPath)
        try:
            command = ['node', client_path, 'prettier', '--write', tmpPath]
            print(command)
            subprocess.run(command, check=False, cwd=node_dir)
            print("=> Ok")
        except subprocess.CalledProcessError as e:
            print(f'=>Error : {e}')

# Call the format_sources function
format_sources()
