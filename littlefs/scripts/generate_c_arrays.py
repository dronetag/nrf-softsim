import os
import re
from typing import List, Tuple

HEADER  = """
#include "ss_static_files.h"
"""

def order_files(files, cache_hits : List[Tuple[str, int]]):
    def find_cache_hit(file):
        for name, hit in cache_hits:
            if file == name:
                return hit
        return 0

    files = sorted(files, key=lambda x: find_cache_hit(x), reverse=True)
    return files
    

def to_c_array(path='files', cache_hits_file='softsim-fs-stats.log', out_path='ss_static_files.c'):
    # walk the files directory and get a list of all files and subdirectories
    s = HEADER

    with open(cache_hits_file, 'r') as file:
        cache_hits_str = file.readlines()
        cache_hits_strs = [hit.split() for hit in cache_hits_str]
        cache_hits = [(hit[1], int(hit[0])) for hit in cache_hits_strs]

    files, directories = get_dirs_and_files(path)
    files = order_files(files, cache_hits)  

    s += 'const sf_dir_t onomondo_sf_dirs_arr[] = {\n'  
    ds = []
    for d in directories:
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
        if(len(current_dir) > 0):
            directories.append(current_dir)
        for file in f_names:
            files.append(current_dir+'/'+file)
    return files, directories

to_c_array('../files', cache_hits_file='../softsim-fs-stats.log')
    
