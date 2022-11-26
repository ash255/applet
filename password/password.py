# !/usr/bin/env python
# -*- coding:utf-8 -*- 

import threading
import sys
import crypt
import random
from itertools import product
    
mutex = None #线程锁
found = False
min_task_pwd = 300  #get_dist_passwd()返回的最小值
max_task_pwd = 1000 #get_dist_passwd()返回的最大值
lower = "abcdefghijklmnopqrstuvwxyz"
upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
number = "0123456789"
letter = "_-"

def get_dist_passwd_from_string(dict_string=None, min_len=0, max_len=0):
    global mutex, g_min_len, g_max_len, g_dict_string, g_dict_product, g_cur_len
    
    mutex.acquire()
    if(dict_string != None):
        g_dict_string = dict_string
        g_max_len = max_len
        g_min_len = min_len
        g_cur_len = g_min_len
        g_dict_product = product(g_dict_string, repeat=g_cur_len)
        mutex.release()
        return
    elif(g_dict_string != None):
        num = random.randint(min_task_pwd, max_task_pwd)
        pwds = []
        for i in range(0, num):
            try:
                # pwd = g_dict_product.__next__() #for python 3
                pwd = g_dict_product.next() #for python 2
            except StopIteration:
                if(g_cur_len != g_max_len):
                    g_cur_len += 1
                    g_dict_product = product(g_dict_string, repeat=g_cur_len)
                break
            pwds.append("".join(pwd))
        mutex.release()
        return pwds
    mutex.release()
    return []
    
def get_dist_passwd_from_file(file=None):
    global mutex, g_dict_file, g_dict_fd, min_task_pwd, max_task_pwd
    
    mutex.acquire()
    if(file != None):
        g_dict_file = file
        g_dict_fd = open(g_dict_file)
        mutex.release()
        return
    elif(g_dict_file != None):
        num = random.randint(min_task_pwd, max_task_pwd)
        pwds = []
        for i in range(0, num):
            try:
                line_pwd = g_dict_fd.readline()
            except:
                break
            pwds.append(line_pwd)
        mutex.release()
        return pwds
    mutex.release()
    return []
    
def crack_passwd(method, password, salt):
    global found
    
    dict_pwd = []
    while(True):
        if(method == 1):
            dict_pwd = get_dist_passwd_from_file()
        else:
            dict_pwd = get_dist_passwd_from_string()
        # print("dict_len: %d" % len(dict_pwd))
        if(dict_pwd == []):
            return
        for pwd in dict_pwd:
            # print("test pwd: %s" % pwd)
            e_password = crypt.crypt(pwd, salt)
            if(password == e_password):
                print("found password: %s" % pwd)
                found = True
                break
        if(found):
            return

def get_salt(pwd):
    return pwd[0:2]

def main(argc, argv):
    global mutex
    
    password = "Wu/RXZBRWoEuY"
    # password = "ab01FAX.bQRSU"
    password = crypt.crypt("123", "ab")
    print("encrypt password: %s" % password)
    dict_file = ""
    dict_string = number + upper + lower
    # dict_string = number
    thread_num = 3

    mutex = threading.Lock()
    get_dist_passwd_from_string(dict_string, 6, 6)
    threads = []
    for i in range(thread_num):
        t = threading.Thread(target=crack_passwd, args=(0, password, get_salt(password)))
        t.start()
        threads.append(t)
 
    for t in threads:
        t.join()

if(__name__ == '__main__'):
    main(len(sys.argv), sys.argv)
    