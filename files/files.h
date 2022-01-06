#ifndef __FILES_H__
#define __FILES_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long fsize(FILE *fp);
int fmap(string file, void **mem, long *size);

#endif