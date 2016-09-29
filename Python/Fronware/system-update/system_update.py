#!/usr/bin/python
# -*- coding: utf-8 -*-

'''
Created on 2014年7月23日
Author:Herman
'''

import os
import sys
import time
import glob
import socket
import commands
import shutil

LOGFILE = "/var/log/update.log"
PACKAGE_PATH = '/etc/system-update/'
UPDATE_PATH = '/tmp/system-update/'
DELETE_LIST = '/del.list'
PROGRAM_NAME = 'system_update.py'
PROGRAM_PATH = '/usr/bin/'

NEW_UPDATE_PROGRAM = '升级程序已更新,正在重启升级程序'

SOCKET = {
    "server":"",
    "addr":("127.0.0.1", 51229),
}


def add_strs_time(strs):
    """Add log strs time prev."""

    timedc = time.strftime('%Y-%m-%d-%X', \
                           time.gmtime(time.time() - time.timezone))
    return str(timedc) + " " + strs.strip() + "\n\n"

def write_update_log(strs):
    """Write update log."""

    filedesc = file(LOGFILE, "a")
    filedesc.write(add_strs_time(strs))
    print(add_strs_time(strs))
    filedesc.close()
    return (True, "")

def clear_log_file():
    '''Clear log file'''
    
    filedesc = file(LOGFILE, 'w')
    filedesc.close()
    return (True, '')

def init_socket_server():
    """Init socket server."""

    # e.g server client.
    server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    SOCKET["server"] = server
    return (True, "")

def check_system_update_exist():

    cmd = 'ps -ax | grep ' + PROGRAM_NAME
    cmdoutput = commands.getstatusoutput(cmd)
    for x in cmdoutput[1].split("\n"):
        if PROGRAM_PATH + PROGRAM_NAME in x:
            if str(os.getpid()) != x.strip().split()[0]:
                return (False, "")
    return (True, "")

def init_update_environment():
    """Init upadte environment."""

    (flag, state) = check_system_update_exist()
    if not flag:
        write_update_log("升级脚本被多次调用:当前pid为:" + str(os.getpid()))
        return (False, state)

    clear_log_file()

    write_update_log("开始升级:当前pid为:" + str(os.getpid()))

    # e.g init socket server
    (flag, state) = init_socket_server()
    if not flag:
        return (False, state)

    return (True, "")

def output_result(msg):
    """Send result."""

    # e.g send socket strs
    SOCKET["server"].sendto(msg, SOCKET["addr"])

    # e.g log result
    write_update_log("进度更新:"+msg)

    return (True, "")

def close_socket():
    """Close socket server."""

    SOCKET["server"].close()
    return (True, "")

def clear_update_folder():
    '''Delete all files and folders in system-update'''
    
    shutil.rmtree(UPDATE_PATH, True)
    
    os.chdir(PACKAGE_PATH)
    filelist = os.listdir(PACKAGE_PATH)
    for f in filelist:
        if os.path.isfile(f):
            os.remove(f)
        elif os.path.isdir(f):
            shutil.rmtree(f, True)

def clear_update_environment():
    """Clear upadte environment."""
  
    close_socket()

    return (True, "")

def return_result(finish, desc):
    """Return upadte result."""

    if desc != NEW_UPDATE_PROGRAM:
        clear_update_folder()

    write_update_log("升级结束:\n"+finish+":"+desc)

    clear_update_environment()

    return (finish, desc)

def find_update_package_and_extract():

    if not os.path.isdir(PACKAGE_PATH):
        os.mkdir(PACKAGE_PATH)
        
    if not os.path.isdir(UPDATE_PATH):
        os.mkdir(UPDATE_PATH)
    
    os.chdir(PACKAGE_PATH)
    # find out all update package
    packages = glob.glob('fronview2000-update*.tar.bz2')
    num_pkg = len(packages)
    if num_pkg is 0:
        return (False, '无法找到升级包', '')
       
    #find the newest package
    packages.sort()
    write_update_log("发现升级包:" + packages[-1])
    update_package = packages[-1].split('.tar.bz2')[0]
    if not os.path.isdir(UPDATE_PATH + update_package):
        # extract the newest package
        output_result('正在解压')
        (status, output) = commands.getstatusoutput('tar xvf ' + PACKAGE_PATH + packages[-1] + ' -C ' + UPDATE_PATH)    
        if status:
            return(False, '解压失败: ' + packages[-1] + '\n' + output, '')
    
        # verify the package name
        if update_package != output.lstrip().split('/')[0]:
            return(False, '非法升级包', '')
    else:
        write_update_log('此为更新后的升级程序')
        
    return (True, '', update_package)

def find_new_update_program(update_package):
    os.chdir(UPDATE_PATH + update_package)
    if os.path.isfile('system_update.py'):
        shutil.move('system_update.py', '/usr/bin/system_update.py')
        os.system('chmod +x /usr/bin/system_update.py')
        output_result('升级程序本身有更新')
        return (False, NEW_UPDATE_PROGRAM)
    return (True, '')

def delete_useless_files():
    if os.access(DELETE_LIST, os.F_OK):
        del_lines = file(DELETE_LIST).readlines()
        for del_file in del_lines:
            del_file = '/' + del_file.rstrip()
            if not os.access(del_file, os.F_OK):
                write_update_log('no this file or folder: ' + del_file)
                continue
            
            if del_file.endswith('/') and os.path.isdir(del_file):
                shutil.rmtree(del_file, True)
            elif os.path.isfile(del_file):
                os.remove(del_file)
        os.remove(DELETE_LIST)

def install_update_packages(update_package):
    output_result('正在升级')
    os.chdir(UPDATE_PATH + update_package)
    packages = glob.glob('update*.tar.bz2')
    num_pkg = len(packages)
    if num_pkg is 0:
        return (False, '空的升级包')
    
    packages.sort()
    for pkg in packages:
        write_update_log('开始安装: ' + pkg)
        (status, output) = commands.getstatusoutput('tar xvf ' + pkg + ' -C /')
        if status:
            return (False, '解压失败: ' + pkg+ '\n' + output)
        delete_useless_files()
        write_update_log('完成安装: ' + pkg)
  
    # delete update package
    os.chdir(UPDATE_PATH)
    shutil.rmtree(update_package, True)

    return (True, '')
    
def run_update(argv):
    (flag, state) = init_update_environment()
    if not flag:
        return return_result('failed', state)
        
    (flag, state, update_package) = find_update_package_and_extract()
    if not flag:
        return return_result('failed', state)
    
    (flag, state) = find_new_update_program(update_package)
    if not flag:
        return return_result('failed', state)
    
    (flag, state) = install_update_packages(update_package)
    if not flag:
        return return_result('failed', state)
    
    output_result('升级成功')
    
    clear_update_folder()    
    clear_update_environment()

    return ('successed', 'Complete!')

if __name__ == '__main__':
    if run_update(sys.argv)[0] == "failed":
        sys.exit(1)
    sys.exit(0)
