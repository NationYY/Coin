import os;

def process(res_root_dir):
	for path, folder, files in os.walk(res_root_dir):
		for f in files:
			if f.find(".cpp") >= 0:
				#print(f)
				add_pch(os.path.join(path, f))

def add_pch(file_name):
	f = open(file_name)
	lines = f.readlines();
	f.close();

	f = open(file_name, "w+")
	f.writelines('#include "stdafx.h"' + "\n")
	f.writelines(lines);
	f.close();


process("./cplus/client/gnet");
process("./cplus/server/gnet");




