/**
 *  @file Inventory.h
 *	@short Класс заголовок для подключения к libinvetory и поиску Java
 *  @author Кесаев Владислав
 */

#pragma once
#include <libinventory_dynamic.h>

/**
 * This function searches java in inventory using the dynamic inventory library
 * @param javaMinVersion - minimum version of java
 * @param dst - buffer for the result java path. The type is void** because the memmory is allocated inside this func and 
 * we don't know is it a pointer to wchar* or to char*
*/
void findJavaInInventory(int javaMinVersion, void** dst);