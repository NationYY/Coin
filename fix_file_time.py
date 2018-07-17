
import os
import sys
import time

g_current_time = time.time()

g_except = [".git"]

def is_except(file_name):
    global g_except
    for e in g_except:
        if file_name.find(e) >= 0:
            return True
    return False

def update_file_access_and_modified_time(path):
    file_list = os.listdir(path)

    for file_name in file_list:
        if is_except(file_name):
            continue

        file_path = os.path.join(path, file_name)
        if os.path.isdir(file_path):
            update_file_access_and_modified_time(file_path)
        else:
            #modified time
            global g_current_time
            mtime = os.path.getmtime(file_path)
            if mtime > g_current_time:
                print(file_path)
                os.utime(file_path, None)

run_dir = os.path.split(os.path.realpath(sys.argv[0]))[0]
print(run_dir)
update_file_access_and_modified_time(run_dir)
