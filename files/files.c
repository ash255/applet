#include "files.h"

/*
vector<string> flist(string folder)
{
	vector<string> file_list;
	struct _finddata_t fileInfo;
	long long findResult = _findfirst(fileFolder.c_str(), &fileInfo);

	if (findResult != -1)
	{
		do
		{
			file_list.push_back(fileInfo.name);
		} while (_findnext(findResult, &fileInfo) == 0);
	}

	_findclose(findResult);
	return file_list;
}
*/

long fsize(FILE *fp)
{
    if (fp != NULL)
    {
        long cur = ftell(fp);
        fseek(fp, 0L, SEEK_END);
        long size = ftell(fp);
        fseek(fp, cur, SEEK_SET);
        return size;
    }
    return -1;
}

int fmap(string file, void** mem, long *size)
{
    FILE* fp = NULL;
    int flag = 0;
    long file_size;
    void* temp_ptr;
    
    fp = fopen(file, "rb");
    if (fp != NULL)
    {
        file_size = fsize(fp);
        if (size > 0)
        {
            temp_ptr = malloc(file_size);
            if (temp_ptr != NULL && fread(temp_ptr, 1, file_size, fp) == file_size)
            {
                *mem = temp_ptr;
                *size = file_size;
                flag = true;
            }
            if(temp_ptr != NULL)
                free(temp_ptr);
        }
        fclose(fp);
    }
    return flag;
}
