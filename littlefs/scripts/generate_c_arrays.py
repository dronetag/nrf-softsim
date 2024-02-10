import os
import re

HEADER  = """
#include "ss_static_files.h"
"""


def to_c_array(path='files', out_path='ss_static_files.c'):
    # walk the files directory and get a list of all files and subdirectories
    s = HEADER


    files, directories = get_dirs_and_files(path)    

    s += 'const sf_dir_t onomondo_sf_dirs_arr[] = {\n'  
    ds = []
    for d in directories:
        if d == '':
            continue    
        ds.append( f'{{.name = "{d}"}}')
    s += ', \n'.join(ds) + '\n};\n' 
    
    for f in files:
        c_f_name = (f).replace("/", "_").replace(".", "_")
        s += f"static const uint8_t {c_f_name}[] = \n"
        with open(path + f, 'r') as file:
            data = file.read()
            data_len = len(data)/2
            # get it in chunks of 2
            data = re.findall('..', data)
            data = ','.join([f'0x{byte}' for byte in data])
            s += f'{{{data}}};\n'

    s += 'const sf_file_t onomondo_sf_files_arr[] = {\n'
    fs = []

    file_name_len = 0
    for i,f in enumerate(files):
        c_f_name = (f).replace("/", "_").replace(".", "_")
        with open(path + f, 'r') as file:
            file_name_len += len(f)
            data = file.read()
            data_len = len(data)/2
            # get it in chunks of 2
            data = re.findall('..', data)
            data = ','.join([f'0x{byte}' for byte in data])
            fs.append(f'{{.name = "{f}", .data = {c_f_name}, .size = sizeof({c_f_name}), .nvs_key = {i+4000}}}')
    s += ', \n'.join(fs) + '\n};\n'
    
    s += f'const uint32_t onomondo_sf_files_names_len = {file_name_len};\n'
    s += f'const uint32_t onomondo_sf_files_len = {len(files)};\n'
    s += f'const uint32_t onomondo_sf_dirs_len = {len(directories)};\n'
    s += f'const sf_file_t *onomondo_sf_files = onomondo_sf_files_arr;\n'
    s += f'const sf_dir_t *onomondo_sf_dirs = onomondo_sf_dirs_arr;\n'

    with open(out_path, 'w') as file: 
        file.write(s)

def get_dirs_and_files(path):
    files = []
    directories = []

    for root, d_names, f_names in os.walk(path):
        current_dir = root.split('files')[-1]
        directories.append(current_dir)
        for file in f_names:
            files.append(current_dir+'/'+file)
    return files, directories
to_c_array('../files')
    
