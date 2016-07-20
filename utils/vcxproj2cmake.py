#!/usr/bin/env python3
import os
import argparse
from xml.dom.minidom import parse

parser = argparse.ArgumentParser(description='Turn vcxproj file into CMakeLists.txt')
parser.add_argument('vcxproj')
args = parser.parse_args()

def parse_vcxproj(infile):
    dom = parse(infile)

    files = []
    shared_files = []
    for el in dom.getElementsByTagName("ClCompile"):
        if el.hasAttribute("Include"):
            path = el.getAttribute("Include")
            if "shared" in path or "public" in path or "common" in path:
                shared_files.append(path + "\n")
            else:
                files.append(path + "\n")
                
    files.sort()
    shared_files.sort()
        
    with open("CMakeLists_" + os.path.basename(infile) + ".txt", 'w') as out:
        out.write("set(SOURCE_FILES \n")
        out.writelines(files)
        out.write(")\n\n")
        
        out.write("set(SHARED_FILES \n")
        out.writelines(shared_files)
        out.write(")\n\n")

if __name__ == "__main__":
    if not args.vcxproj.endswith('.vcxproj') or not os.path.exists(args.vcxproj):
        print("Please provide a valid vcxproj file!")
        exit(1)

    print("Parsing vcxproj file...")
    parse_vcxproj(args.vcxproj)
    print("Completed parsing, output stored in CMakeLists_" + args.vcxproj + ".txt")
