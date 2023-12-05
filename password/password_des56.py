# !/usr/bin/env python
# -*- coding:utf-8 -*- 

import sys, time
import crypt
from itertools import product
from itertools import permutations

#可见字符共95个，95^8=6,634,204,312,890,625(6千兆)
lower = 'abcdefghijklmnopqrstuvwxyz'
upper = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
number = '0123456789'
letter = ' !"#$%&\'()*+,-./:;<=>?@[\]^_`{|}~'   #len=33

def test_time():
    start = time.time()
    it = product('0123', repeat=8)
    cnt = 0
    for pwd in it:
        cnt += 1
        crypt.crypt(''.join(pwd))
    used_time = time.time() - start
    print('[test]used_time=%ds avg=%fs' % (used_time, used_time/cnt))

'''
def get_pwd():
    it = product(lower+upper+number+letter, repeat=8)
    for pwd in it:
        yield ''.join(pwd)
'''
def get_pwd():
    # letter = '!"#$%&\'()*+,-./:;<=>?@[\]^_`{}~' #缩小特殊字符搜索空间
    cand1 = [''.join(x) for x in product('Cc','Oo','Mm','Bb','Aa')]
    # for pwd in product(lower+upper+number+letter, repeat=3):    #预计循环(2^5)*(95^3)*4=109,744,000 缩小后(2^5)*(93^3)*4=102,957,696
    # for pwd in permutations(lower+upper+number+letter, 3):    #预计循环(2^5)*(95*94*93)*4=106,302,720 缩小后(2^5)*(93*92*91)*4=99,660,288
    for pwd in product(number+letter, repeat=3):    #预计循环(2^5)*(43^3)*4=10,176,896 缩小后(2^5)*(41^3)*4=8,821,888
        for c1 in cand1:
            yield c1 + ''.join(pwd)
            yield pwd[0] + c1 + ''.join(pwd[1:])
            yield ''.join(pwd[0:2]) + c1 + pwd[2]
            yield ''.join(pwd) + c1

def main(argc, argv):
    test_time()
    expect_pwd = 'Wu/RXZBRWoEuY'
    start = time.time()
    cnt = 0
    for pwd in get_pwd():
        cnt += 1
        # print(pwd)
        if(crypt.crypt(pwd, expect_pwd[0:2]) == expect_pwd):
            print('password=%s' % pwd)
    used_time = time.time() - start
    print('used_time=%ds avg=%fs' % (used_time, used_time/cnt))
    
if(__name__ == '__main__'):
    main(len(sys.argv), sys.argv)
