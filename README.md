
git子模块使用了[cjson官方的源码](https://github.com/DaveGamble/cJSON)：`git@github.com:DaveGamble/cJSON.git`
需要使用本教程则需要使用该指令拉取代码
```
git clone git@github.com:sexjun/cds_cjson.git --recurse-submodules
```
拿到GitHub的源码先干什么？
1. 查看证书，可否商用，自己代码是否需要开源
2. 查看使用
3. 查看注意事项
我们来看看cjson的readme有什么好看的。
1. cJSON.h和cJSON.c复制到您的项目源就可以开始使用。cJSON 是用 ANSI C (C89) 编写的。
2. cJSON 仅支持 UTF-8 编码的输入
3. double除了 IEEE754 双精度浮点数之外，cJSON 不正式支持任何实现。它可能仍然适用于其他实现，但这些错误将被视为无效。
4. 一般来说，cJSON不是线程安全的。
5. 最初创建 cJSON 时，它没有遵循 JSON 标准，也没有区分大小写字母。如果您想要正确的、符合标准的行为，您需要使用CaseSensitive可用的函数。
6. cJSON 支持解析和打印包含具有多个同名成员的对象的 JSON。cJSON_GetObjectItemCaseSensitive但是总是只返回第一个。

## cjson学习指南
cjson模块主要的方法都在`cJSON.h`里面存储着。在学习cjson之前看了一下cjson的源码，里面有一段很优秀的宏定义和命名方式值得学习，代码风格很好，所以我打算先记录一下代码风格，然后在讲如何使用。
- 兼容c++

```c
// 1. 防止重复包含
#ifndef cJSON__h
#define cJSON__h

// 2. 防止有cpp编译器
#ifdef __cplusplus
extern "C"
{
#endif
    // here is the code 
#ifdef __cplusplus
}
#endif

#endif
```
- 返回值类型的宏定义，定义了public方法，可以学习定义private方法，这样别人看代码就显而易见了
```c
#define CJSON_CDECL __cdecl
#define CJSON_STDCALL __stdcall

/* export symbols by default, this is necessary for copy pasting the C and header file */
#if !1 /* defined(CJSON_HIDE_SYMBOLS) */ && !defined(CJSON_IMPORT_SYMBOLS) && !defined(CJSON_EXPORT_SYMBOLS)
#define CJSON_EXPORT_SYMBOLS
#endif

#if 1 /* defined(CJSON_HIDE_SYMBOLS) */
#define CJSON_PUBLIC(type)   type CJSON_STDCALL
#elif defined(CJSON_EXPORT_SYMBOLS)
#define CJSON_PUBLIC(type)   __declspec(dllexport) type CJSON_STDCALL
#elif defined(CJSON_IMPORT_SYMBOLS)
#define CJSON_PUBLIC(type)   __declspec(dllimport) type CJSON_STDCALL
#endif
#else /* !__WINDOWS__ */
#define CJSON_CDECL
#define CJSON_STDCALL

#if (defined(__GNUC__) || defined(__SUNPRO_CC) || defined (__SUNPRO_C)) && defined(CJSON_API_VISIBILITY)
#define CJSON_PUBLIC(type)   __attribute__((visibility("default"))) type
#else
#define CJSON_PUBLIC(type) type
#endif
#endif

```

- 定义返回值类型

```c
/* cJSON Types: */
#define cJSON_Invalid (0)
#define cJSON_False  (1 << 0)
#define cJSON_True   (1 << 1)
#define cJSON_NULL   (1 << 2)
#define cJSON_Number (1 << 3)
#define cJSON_String (1 << 4)
#define cJSON_Array  (1 << 5)
#define cJSON_Object (1 << 6)
#define cJSON_Raw    (1 << 7) /* raw json */

```

- `goto`语言
```c
如果有错误发生，则使用goto语句进行处理。
goto fail:
    清理
```

## 代码教程
1. 解析cjson
2. 生成cjson
### 1. 解析cjson
1. 首先要将文本流转化为IO流
2. 然后调用如下借口进行转化为cjson的`cJSON`指针对象 **该指针对象自动分配内存**，需要调用接口`cJSON_Delete(cJSON *item)`将自动分配的内存进行删除。
```c
/* Supply a block of JSON, and this returns a cJSON object you can interrogate. */
// 传入一段buffer，将所有buffer都解析
CJSON_PUBLIC(cJSON *) cJSON_Parse(const char *value);
// 传入一段buffer，解析buffer的指定长度为cjson的数据结构
CJSON_PUBLIC(cJSON *) cJSON_ParseWithLength(const char *value, size_t buffer_length);
/* ParseWithOpts allows you to require (and check) that the JSON is null terminated, and to retrieve the pointer to the final byte parsed. */
/* If you supply a ptr in return_parse_end and parsing fails, then return_parse_end will contain a pointer to the error so will match cJSON_GetErrorPtr(). */
CJSON_PUBLIC(cJSON *) cJSON_ParseWithOpts(const char *value, const char **return_parse_end, cJSON_bool require_null_terminated);
CJSON_PUBLIC(cJSON *) cJSON_ParseWithLengthOpts(const char *value, size_t buffer_length, const char **return_parse_end, cJSON_bool require_null_terminated);

/* Delete a cJSON entity and all subentities. */
CJSON_PUBLIC(void) cJSON_Delete(cJSON *item);
```
3. 解析得到cjson的指针结构如下：
```c

/* The cJSON structure: */
typedef struct cJSON
{
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct cJSON *next;
    struct cJSON *prev;
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    struct cJSON *child;

    /* The type of the item, as above. */
    int type;

    /* The item's string, if type==cJSON_String  and type == cJSON_Raw */
    char *valuestring;
    /* writing to valueint is DEPRECATED, use cJSON_SetNumberValue instead */
    int valueint;
    /* The item's number, if type==cJSON_Number */
    double valuedouble;

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
} cJSON;
```

然后就可以进行便利查询得到内容的数值了。

4. 通过接口查询数值的类型，然后获取值

```c
/* These functions check the type of an item */
CJSON_PUBLIC(cJSON_bool) cJSON_IsInvalid(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsFalse(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsTrue(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsBool(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsNull(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsNumber(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsString(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsArray(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsObject(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsRaw(const cJSON * const item);


/* Returns the number of items in an array (or object). */
CJSON_PUBLIC(int) cJSON_GetArraySize(const cJSON *array);
/* Retrieve item number "index" from array "array". Returns NULL if unsuccessful. */
CJSON_PUBLIC(cJSON *) cJSON_GetArrayItem(const cJSON *array, int index);
/* Get item "string" from object. Case insensitive. */
CJSON_PUBLIC(cJSON *) cJSON_GetObjectItem(const cJSON * const object, const char * const string);
CJSON_PUBLIC(cJSON *) cJSON_GetObjectItemCaseSensitive(const cJSON * const object, const char * const string);
CJSON_PUBLIC(cJSON_bool) cJSON_HasObjectItem(const cJSON *object, const char *string);
/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when cJSON_Parse() returns 0. 0 when cJSON_Parse() succeeds. */
CJSON_PUBLIC(const char *) cJSON_GetErrorPtr(void);

/* Check item type and return its value */
CJSON_PUBLIC(char *) cJSON_GetStringValue(const cJSON * const item);
CJSON_PUBLIC(double) cJSON_GetNumberValue(const cJSON * const item);
```