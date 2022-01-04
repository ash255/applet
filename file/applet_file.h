#ifndef __APPLET_FILE_H__
#define __APPLET_FILE_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long fsize(FILE *fp);
int fmap(string file, void **mem, long *size);

#endif