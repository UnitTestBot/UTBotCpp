#define __STDC_WANT_LIB_EXT1__ 1
#include "stdbool.h"
#include "string.h"



#define QOS_SERV_MAX 3
#define BOOL bool
#define UINT32 int
#define VOID void
#define SOME_ENUM int
#define USHORT unsigned short
#define UCHAR unsigned char
#define VOS_ERR 1
#define VOS_OK 0
#define EOK 0
#define errno_t int

typedef struct {
    BOOL state;
    BOOL ability[QOS_SERV_MAX];
    UINT32 spec[QOS_SERV_MAX];
} STRUCT_FUNCTION_1;


STRUCT_FUNCTION_1 g_cacheData = { 0 };

VOID FOO_FUNCTION_1(SOME_ENUM type, BOOL ability, UINT32 spec)
{
    g_cacheData.ability[type] = ability;
    g_cacheData.spec[type] = spec;
    return;
}

typedef struct {
    USHORT type;
    USHORT len;
    VOID *value;
} CUSTOM_STRUCT_1;

typedef struct tag // Removed CUSTOM_TYPE here from test case
{
    USHORT usTLVType;
    USHORT usTLVLen;
}CUSTOM_TYPE;

typedef CUSTOM_TYPE TYPE_ALIAS;

UINT32 FOO_FUNCTION_2(VOID *buff, UINT32 buffLen, const CUSTOM_STRUCT_1 dynTlv[], UINT32 tlvNum, UINT32 *outLen)
{
    *outLen = 0;
    for (UINT32 i = 0; i < tlvNum; i++) {
        UINT32 groupLen = sizeof(TYPE_ALIAS) + dynTlv[i].len;
        if (*outLen + groupLen > buffLen) {
            return VOS_ERR;
        }
        TYPE_ALIAS *tlv = (TYPE_ALIAS *)((UCHAR *)buff + *outLen);
        tlv->usTLVType = dynTlv[i].type;
        tlv->usTLVLen = dynTlv[i].len;
        errno_t ret = 0;//memcpy_s(tlv + 1, buffLen - *outLen - sizeof(*tlv), dynTlv[i].value, dynTlv[i].len);
        if (ret != EOK) {
            return VOS_ERR;
        }
        *outLen += groupLen;
    }

    return VOS_OK;
}

VOID FOO_FUNCTION_3(UINT32 *type, UINT32 *len, VOID **value, VOID *tlvIn)
{
    TYPE_ALIAS *tlvHead = (TYPE_ALIAS *)tlvIn;
    *type = tlvHead->usTLVType;
    *len = tlvHead->usTLVLen;
    *value = (VOID *)(tlvHead + 1);
    return;
}

typedef struct tagCUSTOM_STRUCT_2 {
    UINT32 offset;
    UINT32 len;
    UINT32 id;
} CUSTOM_STRUCT_2;

UINT32 FOO_FUNCTION_4(const CUSTOM_STRUCT_2 *attrib, UINT32 num, UINT32 id, UINT32 *offset, UINT32 *len)
{
    UINT32 i;

    for (i = 0; i < num; i++) {
        if (id == attrib[i].id) {
            *offset = attrib[i].offset;
            *len = attrib[i].len;
            return VOS_OK;
        }
    }
    return VOS_ERR;
}

int main() {
    return 0;
}
