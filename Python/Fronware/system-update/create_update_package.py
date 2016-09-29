#!/usr/bin/python
#  -*- coding: utf-8 -*-

'''
Created on 2014年7月24日
Author:Herman
'''

import os
import sys
import shutil
import commands

DIFF_LIST_PATH = 'diff-list/'

def get_folder_name(path):
    names = path.strip().split('/')
    for name in names[::-1]:   
        if name != '':
            return name

def create_del_list(new_path, old_path):
    old_path_tmp = 'fronview2000_tmp'
    cmd = 'rsync -aH --delete ' + old_path + ' ' + old_path_tmp
    (status, output) = commands.getstatusoutput(cmd)
    if status:
        print cmd
        print(output)
        return (False, '')
    
    cmd = 'rsync -aHv --delete-after ' + new_path + ' ' + old_path_tmp
    (status, output) = commands.getstatusoutput(cmd)
    if status:
        print cmd
        print(output)
        return (False, '')
    
    if not os.path.isdir(DIFF_LIST_PATH):
        os.makedirs(DIFF_LIST_PATH)
        
    new_name = get_folder_name(new_path)
    old_name = get_folder_name(old_path)
    diff_list_name = DIFF_LIST_PATH + new_name + '_to_' + old_name + '.list'
    diff_list = file(diff_list_name, 'w')
    diff_list.write(output + '\n')
    diff_list.close()
    return (True, diff_list_name)
     
def rsync_dir(new_path, update_path, direction):
            cmd = 'rsync -a --exclude=* ' + new_path + direction + ' ' + update_path + direction
            (status, output) = commands.getstatusoutput(cmd)
            if status == 3072:
                folders = direction.split('/')
                del folders[-2]
                father_dir = '/'.join(folders) 
                rsync_dir(new_path, update_path, father_dir)
                rsync_dir(new_path, update_path, direction)
            elif status:
                print cmd
                print(output)      

def rsync_file(new_path, update_path, filename):
            cmd = 'rsync -a ' + new_path + filename + ' ' + update_path + filename 
            (status, output) = commands.getstatusoutput(cmd)
            if status == 3072:
                folders = filename.split('/')
                del folders[-1]
                father_dir = '/'.join(folders) + '/'
                rsync_dir(new_path, update_path, father_dir)
                rsync_file(new_path, update_path, filename)
            elif status:
                print cmd
                print(output)      

def create_package(argv): 
    if len(argv) != 4:
        print(argv[0] + ' + [new file system path] + [old file system path] + [update package path]')
        sys.exit(0)
        
    for option in argv:
        if option == 'help':
            print(argv[0] + ' + [new file system path] + [old file system path] + [update package path]')
            sys.exit(0)
       
    new_path = argv[1]
    old_path = argv[2]
    update_path = argv[3]
    
    if not os.path.isdir(new_path):
        print('no this directory: ' + new_path)
        return False
    
    if not os.path.isdir(old_path):
        print('no this directory: ' + old_path)
        return False
        
    if not new_path.endswith('/'):
        new_path = new_path + '/'
        
    if not old_path.endswith('/'):
        old_path = old_path + '/'
    
    if not update_path.endswith('/'):
        update_path = update_path + '/'
    
    if os.access(update_path, os.F_OK):
        shutil.rmtree(update_path, True)
        
    os.mkdir(update_path)
    
    (status, diff_list_name) = create_del_list(new_path, old_path)
    if not status:
        return False
    
    del_list = file(update_path + 'del.list', 'w')
    
    folder_list = []
    file_list = []
    list_lines = file(diff_list_name).readlines()
    for line in list_lines:
        line = line.strip()
        if os.path.isfile(new_path + line):
            file_list.append(line)
        elif os.path.isdir(new_path + line):
            folder_list.append(line)
        elif line.startswith('deleting '):
            del_list.write(line.split('deleting ')[1] + '\n')
        else:
            print('not file or folder: ' + line)
    
    del_list.close()
     
    folder_list.sort()       
    for line in folder_list:
        if not os.path.isdir(update_path + line):
            rsync_dir(new_path, update_path, line)

    
    for line in file_list:
        if not os.path.isfile(update_path + line):
            rsync_file(new_path, update_path, line)
    
    update_pkg = update_path.rstrip('/') + '.tar.bz2'
    (status, output) = commands.getstatusoutput('tar -jcvf ' + update_pkg + ' -C ' + update_path + ' .')
    if status:
        print cmd
        print(output)
        return False
        
    print 'Create ' + update_pkg    
    
    return True
            

if __name__ == '__main__':
    if not create_package(sys.argv):
        sys.exit(1)
    sys.exit(0)
