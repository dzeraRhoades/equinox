/**
 *  @file InventoryUnix.c
 *	@short Реализация подулючения к libinvetory и поиска Java для linux и macos
 *  @author Кесаев Владислав
 */

#include <dlfcn.h>
#include <dirent.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <graal_isolate_dynamic.h>
#include <libinventory_dynamic.h>
#include "Inventory.h"

#define _ASSERT(x,...) if((x) == 0) {fprintf(stderr, __VA_ARGS__); return; }
#define MAX_PATH 260

#ifdef MACOSX
char LIB_PATH[MAX_PATH] = "libinventory.dylib";
#else
char LIB_PATH[MAX_PATH] = "libinventory.so";
#endif // MACOSX

int findFileRecursive(const char* filename, const char* dir, char** dstFilePath);
char* findLib(const char* name);
char* executablePath();
void* loadFuncPtr(const char* funcName, void* handle);

void findJavaInInventory(int javaMinVersion, void** dst)
{
    char* invLib = findLib(LIB_PATH);
    _ASSERT(invLib != NULL, "can't find %s!\n", LIB_PATH)
    void* handle = dlopen(invLib, RTLD_GLOBAL | RTLD_NOW);
    _ASSERT(handle != NULL, "can't open libinventory library!\n")
    graal_isolatethread_t* isolateThread = NULL;
    graal_create_isolate_fn_t createIsolateFuncPtr = (graal_create_isolate_fn_t)loadFuncPtr("graal_create_isolate", handle);
    _ASSERT(createIsolateFuncPtr != NULL, "createIsolateFuncPtr is null\n")
    graal_tear_down_isolate_fn_t graalTearDownIsolateFuncPtr = (graal_tear_down_isolate_fn_t) loadFuncPtr("graal_tear_down_isolate", handle);
    _ASSERT(graalTearDownIsolateFuncPtr != NULL, "graalTearDownIsolateFuncPtr is null\n")
    findJavaComponent_fn_t findJavaComponentFuncPtr = (findJavaComponent_fn_t) loadFuncPtr("findJavaComponent", handle);
    _ASSERT(findJavaComponentFuncPtr != NULL, "findJavaComponentFuncPtr is null\n")

    // now we find java
    int graalInitCode = createIsolateFuncPtr(NULL, NULL, &isolateThread);
    _ASSERT(graalInitCode == 0, "error occured while creating isolate\n")
    char* execPath = executablePath();
    char* arch = "x86_64\0";
    char* path = NULL;
    path = findJavaComponentFuncPtr(isolateThread, javaMinVersion, arch,
            execPath, 0);
    if(path == NULL)
        return;
    *dst = malloc(sizeof(char) * strlen(path));
    strcpy(*dst, path);
    dlclose(handle);
    return;
}

void* loadFuncPtr(const char* funcName, void* handle)
{
    void* sym = dlsym(handle, funcName);
    return (dlerror() == NULL) ? sym : NULL;
}

char* executablePath()
{
    char* buf = (char*)malloc(sizeof(char) * MAX_PATH);
    if (readlink("/proc/self/exe", buf, MAX_PATH - 1) == -1)
        return ".";
    return buf;
}

char* findLib(const char* name)
{
    char* filePath = NULL;
#ifdef MACOSX
    // on Mac the location of plugin is not under the same directory as the launcher itself
    findFileRecursive(name, "..", &filePath);
#else
    findFileRecursive(name, ".", &filePath);
#endif
    if(filePath != NULL)
        printf("libinventory was found: %s\n", filePath);
    return filePath;
}

int findFileRecursive(const char* filename, const char* dir, char** dstFilePath)
{
    struct dirent *entry = NULL;
    DIR *dp = NULL;
    struct stat st;
    char curName[MAX_PATH];

    dp = opendir(dir);
    if (dp != NULL) {
        while ((entry = readdir(dp)))
        {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            strcpy(curName, dir);
            strcat(curName, "/");
            strcat(curName,entry->d_name);
            if(stat(curName, &st))
            {
                fprintf(stderr, "can't get file stat: %s\n", entry->d_name);
                return -1;
            }
            if (S_ISDIR(st.st_mode))
            {
                if(findFileRecursive(filename, curName, dstFilePath) == 0)
                {
                    closedir(dp);
                    return 0;
                }
                continue;
            }
            if(strcmp(filename, entry->d_name) == 0)
            {
                *dstFilePath = (char*) malloc(sizeof(char) * MAX_PATH);
                strcpy(*dstFilePath, curName);
                closedir(dp);
                return 0;
            }
        }
    }
    else
    {
        fprintf(stderr, "can't open file %s\n", dir);
        return -1;
    }
    return 1;
    closedir(dp);
}