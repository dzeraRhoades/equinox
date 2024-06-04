/**
 *  @file InventoryWin.c
 *	@short Реализация подулючения к libinvetory и поиска Java для windows
 *  @author Кесаев Владислав
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <graal_isolate_dynamic.h>
#include <libinventory_dynamic.h>
#include "Inventory.h"

#define _ASSERT(x,...) if((x) == 0) {fprintf(stderr, __VA_ARGS__); return; }

char LIB_PATH[MAX_PATH] = "libinventory.dll";

int findFileRecursive(const char* filename, const char* dir, char** dstFilePath);
char* findLib(const char* name);
char* executablePath();
void* loadFuncPtr(const char* funcName, void* handle);


wchar_t* charToWchar(const char* str)
{
    wchar_t *pwcsName;
    // required size
    int nChars = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    // allocate it
    pwcsName = malloc(sizeof(wchar_t) * nChars);
    MultiByteToWideChar(CP_ACP, 0, str, -1, (LPWSTR)pwcsName, nChars);
    return pwcsName;
}

void findJavaInInventory(int javaMinVersion, void** dst)
{
    char* invLib = findLib(LIB_PATH);
    _ASSERT(invLib != NULL, "can't find %s!\n", LIB_PATH)
    void* handle = LoadLibraryA(invLib);
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
    _ASSERT(graalInitCode == 0, "error occured while creating isolate");
    char* execPath = executablePath();
    char* arch = "x86_64\0";
    char* path = NULL;
    path = findJavaComponentFuncPtr(isolateThread, javaMinVersion, arch,
            execPath, 0);
    if(path == NULL)
        return;
    
    *dst = (void*) charToWchar(path);
    FreeLibrary((HMODULE)handle);
    return;
}

void* loadFuncPtr(const char* funcName, void* handle)
{
    return (void *)GetProcAddress((HMODULE)handle, funcName);
}

char* executablePath()
{
    char* buf = (char*)malloc(sizeof(char) * MAX_PATH);
    if (GetModuleFileNameA(NULL, buf, MAX_PATH) == 0)
        return ".";
    return buf;
}


int findFileRecursive(const char* filename, const char* dir, char** dstFilePath)
{
    char curName[256];
    WIN32_FIND_DATAA FindFileData;

    strcpy(curName, dir);
    strcat(curName, "\\*");
    HANDLE hFind = FindFirstFileA(curName, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return -1;
    } 
    while (FindNextFileA(hFind, &FindFileData) != 0) {
        if(strcmp(FindFileData.cFileName, ".") == 0 || strcmp(FindFileData.cFileName, "..") == 0)
            continue;
        strcpy(curName, dir);
        strcat(curName, "\\");
        strcat(curName,FindFileData.cFileName);
        if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if(findFileRecursive(filename, curName, dstFilePath) == 0)
            {
                FindClose(hFind);
                return 0;
            }
            
        }
        else
        {
            if(strcmp(FindFileData.cFileName, filename) == 0)
            {
                *dstFilePath = (char*) malloc(sizeof(char) * MAX_PATH);
                strcpy(*dstFilePath, curName);
                FindClose(hFind);
                return 0;
            }
        }
    }

    FindClose(hFind);
    return 1;
}


char* findLib(const char* name)
{
    char* filePath = NULL;
    findFileRecursive(name, ".", &filePath);
    if(filePath != NULL)
        printf("libinventory was found: %s\n", filePath);
    return filePath;
}

