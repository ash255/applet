#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

const static char *g_universal_tag_string[] = 
{
    "RESERVED",             //0
    "BOOLEAN",              //1
    "INTEGER",              //2
    "BIT STRING",           //3
    "OCTET STRING",         //4
    "NULL",                 //5
    "OBJECT IDENTIFIER",    //6
    "ObjectDescripion",     //7
    "EXTERNAL",             //8
    "REAL",                 //9
    "ENUMERATED",           //10
    "EMBEDDED PDV",         //11
    "UTF8String",           //12
    "RELATIVE-OI",          //13
    "RESERVED",             //14
    "RESERVED",             //15
    "SEQUENCE",             //16
    "SET",                  //17
    "NumericString",        //18
    "PrintableString",      //19
    "TeletexString",        //20
    "VideotexString",       //21
    "IA5String",            //22
    "UTCTime",              //23
    "GeneralizedTime",      //24
    "GraphicString",        //25
    "VisibleString",        //26
    "GeneralString",        //27
    "UniversalString",      //28
    "CHARACTER STRING",     //29
    "BMPString",            //30
    "RESERVED",             //31
};

/**************** ASN.1 ********************/

/* 
    |  TAG  |   LENGTH   |   VALUE
    |         \
    |            \
    |                \
    |                   \
    |                      \
    |                         \
    | CLASS |   P/C  |   TAGV  |
    |   2   |    1   |     5   |
    CLASS: 00=Universal  01=APPLICATION  10=context-specific  11=PRIVATE
类型
*/
static int asn1_get_tag(uint8_t *data, int size, int *cls, int *pc, int *tag)
{
    int tagv, used_len = 0;
    
    *cls = *data >> 6;
    *pc = (*data >> 5) & 1;
    tagv = *data & 0x1F;
    used_len = 1;
    
    if(tagv != 0x1F)
    {
        *tag = tagv;
    }else
    {
        tagv = 0;
        while(used_len < size)
        {
            used_len++;
            data++;
            tagv = (tagv<<7) | (*data & 0x7F);
            if((*data & 0x80) != 0x80)
                break;
        }
        *tag = tagv;
    }
    return used_len;
} 

/*
    0x80: 不定长
    0x8X：长码，低7位表示长度的字节数
    0xXX：短码，低7位表示长度 
*/
static int asn1_get_len(uint8_t *data, int size, int *len)
{
    int used_len = 0;
    
    used_len = 1;
    if(*data == 0x80)
    {
        *len = -1;
    }else if((*data & 0x80) == 0x80)
    {
        int len_bytes = *data & 0x7F;
        int i, lenv = 0;

        for(i=0;i<len_bytes;i++)
        {
            used_len++;
            data++;
            lenv = (lenv<<8) | *data;
        }
        *len = lenv;
    }else
    {
        *len = *data & 0x7F;
    }
    return used_len;
}

static void print_space(FILE *fp, int level)
{
    int i;
    for(i=0;i<level;i++)
    {
        fputs("    ", fp);
    }  
}

static void asn1_oid_proc(FILE *fp, uint8_t *data, int size)
{
    //x.y.z
    int x, y, tmp;
    
    x = (*data / 40);
    y = *data - x*40;
    size--;
    data++;
    
    fprintf(fp, "%d.%d", x, y);
    while(size > 0)
    {
        tmp = 0;
        while(size > 0)
        {
            tmp = (tmp<<7) | (*data & 0x7F);
            size--;
            if((*data++ & 0x80) != 0x80)
                break;
        }
        fprintf(fp, ".%d", tmp);
    }
    fputs(" ", fp);
}

static void asn1_value_proc(FILE *fp, int cls, int tag, uint8_t *data, int len)
{
    int i, j;
    
    if(cls == 0)    //universal
    {
        switch(tag)
        {
            case 1:     //"BOOLEAN"
            {
                fprintf(fp, "%s ", *data == 0?"True":"False");
                break;
            }
            case 3:     //"BIT STRING"
            {
                int pading = *data++;
                len--;
                int bits_len = len * 8 - pading;
                for(i=0; i<len; i++)
                {
                    for(j=0;j<8;j++)
                    {
                        if(bits_len <= 0)
                            break;
                        fprintf(fp, "%d", (data[i] >> (7-j)) & 1);
                        bits_len--;
                    }
                }
                fputs(" ", fp);
                break;
            }
            case 6:     //"OBJECT IDENTIFIER"
            {
                asn1_oid_proc(fp, data, len); 
                break;
            }
            case 12:    //"UTF8String"
            {
                for(i=0; i<len; i++)
                {
                    fprintf(fp, "%c", data[i]);
                }
                fputs(" ", fp);
                break;
            }
            default:
                for(i=0; i<len; i++)
                {
                    fprintf(fp, "%02X ", data[i]);
                }
        }
    }else
    {
        for(i=0; i<len; i++)
        {
            fprintf(fp, "%02X ", data[i]);
        }
    }
    

}

static int asn1_decode_to_file(FILE *fp, int level, uint8_t *data, int size)
{
    int cls, tag, len, ret = 0, pc, i;
    uint8_t *cur = data;
    
    while(size > 0)
    {
        ret = asn1_get_tag(cur, size, &cls, &pc, &tag);
        if(ret == 0)
            return -1;
        cur += ret;
        size -= ret;
        ret = asn1_get_len(cur, size, &len);
        if(ret == 0)
            return -1;
        cur += ret;
        size -= ret;
        
        if(len == -1)
        {
            printf("error len: %d\n", len);
            return -1;
        }
        
        print_space(fp, level);
        if(cls == 0)
        {
            fprintf(fp, "<%s> ", g_universal_tag_string[tag]);
        }else
        {
            fprintf(fp, "<%d:%d> ", cls, tag);
        }
        
        
        if(pc == 1)
        {
            fputs("\n", fp);
            asn1_decode_to_file(fp, level+1, cur, len);
            print_space(fp, level);
        }else
        {
            asn1_value_proc(fp, cls, tag, cur, len);
        }
        cur += len;
        size -= len;
        
        if(cls == 0)
        {
            fprintf(fp, "</%s>\n", g_universal_tag_string[tag]);
        }else
        {
            fprintf(fp, "</%d:%d>\n", cls, tag);
        } 
    }

    return 1;
}

int main(int argc, char **argv)
{
    uint8_t *data = NULL;
    int size, i;
    FILE *fp_r = fopen("asn1.dat", "rb");
    if(fp_r == NULL)
    {
        printf("file %s open failed\n", "asn1.dat");
        return 0;
    }
    FILE *fp_w = fopen("ans1.xml", "w");
    if(fp_w == NULL)
    {
        printf("open file failed\n");
        return 0;
    }
    
    fseek(fp_r, 0, SEEK_END);
    long fsize = ftell(fp_r);
    fseek(fp_r, 0, SEEK_SET);
    
    uint8_t *data = (uint8_t*)malloc(fsize);
    if(fread(data, 1, fsize, fp_r) != fsize)
    {
        printf("read file error\n");
        return 0;
    }
    
    asn1_decode_to_file(fp, 0, data, size);
      
    if(data != NULL)
        free(data);
    fclose(fp);
    return 0;
}

