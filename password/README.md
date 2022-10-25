# password
linux passowrd

# usage
```
usage: password [arg]
	-h: print this helf
	-f: input from file, such as /etc/shadow
	-i: input from string, such as $1$AXP3QWMEMLZXC92QWL11
	-d: dict file, split by \n
	-t: thread number, defalut 5
```

# issue
crypto函数并非线程安全的，本程序无法在多线程下运行，故本程序没有实际意义。
