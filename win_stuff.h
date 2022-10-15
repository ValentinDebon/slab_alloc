#pragma once

#ifndef __clang__
#error win32 clang build please (hint: use Visual Studio)
#endif 

#include <io.h>
#define WIN32_LEAN_AND_MEAN 1
#define STRICT 1
#include <windows.h>
#include <sys/types.h>


#ifdef __cpluplus
extern "C" {
#endif // __cpluplus

#pragma region MMAP4WIN
	/*
	 *  mmap.h
	 *
	 *  copyright (c) 2018 Xiongfei Shi
	 *
	 *  author: Xiongfei Shi <jenson.shixf(a)gmail.com>
	 *  license: MIT
	 */

#ifndef __MMAP_H__
#define __MMAP_H__

#include <stddef.h>

#define PROT_NONE       0
#define PROT_READ       1
#define PROT_WRITE      2
#define PROT_EXEC       4

#define MAP_FILE        0
#define MAP_SHARED      1
#define MAP_PRIVATE     2
#define MAP_TYPE        0x0F
#define MAP_FIXED       0x10
#define MAP_ANONYMOUS   0x20
#define MAP_ANON        MAP_ANONYMOUS

#define MAP_FAILED      ((void *)-1)

	void* mmap(void* addr, size_t len, int prot, int flags, int fildes, unsigned long off);
	int munmap(void* addr, size_t len);
	int msync(void* addr, size_t len, int flags);
	int mprotect(void* addr, size_t len, int prot);
	int mlock(const void* addr, size_t len);
	int munlock(const void* addr, size_t len);


#endif  /* __MMAP_H__ */

#ifdef MMAP_WIN_IMPLEMENTATION

	static inline /* unsigned long aka */ DWORD win_page_size(void)
	{
		static SYSTEM_INFO sysInfo_ = { .dwPageSize = 0 };         // Useful information about the system
		if (sysInfo_.dwPageSize == 0) {
			GetSystemInfo(&sysInfo_);     // Initialize the structure.
		}
		return sysInfo_.dwPageSize;
	}

	/*
	 *  mmap.c
	 * https://github.com/shixiongfei/mmap-win32
	 *
	 *  copyright (c) 2018 Xiongfei Shi
	 *
	 *  author: Xiongfei Shi <jenson.shixf(a)gmail.com>
	 *  license: MIT
	 */

#include <Windows.h>
#include <errno.h>
#include <io.h>

#define MS_ASYNC            1
#define MS_SYNC             2
#define MS_INVALIDATE       4

#ifndef FILE_MAP_EXECUTE
# define FILE_MAP_EXECUTE   0x0020
#endif

	static int _mmap_error(DWORD err, int deferr) {
		if (0 == err)
			return deferr;
		return err;
	}

	static DWORD _mmap_prot_page(int prot) {
		DWORD protect = 0;

		if (PROT_NONE == prot)
			return protect;

		if (prot & PROT_EXEC)
			protect = (prot & PROT_WRITE) ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
		else
			protect = (prot & PROT_WRITE) ? PAGE_READWRITE : PAGE_READONLY;

		return protect;
	}

	static DWORD _mmap_prot_file(int prot) {
		DWORD desiredAccess = 0;

		if (PROT_NONE == prot)
			return desiredAccess;

		if (prot & PROT_READ)
			desiredAccess |= FILE_MAP_READ;

		if (prot & PROT_WRITE)
			desiredAccess |= FILE_MAP_WRITE;

		if (prot & PROT_EXEC)
			desiredAccess |= FILE_MAP_EXECUTE;

		return desiredAccess;
	}

	void* mmap(void* addr, size_t len, int prot, int flags, int fildes, unsigned long off) {
		HANDLE fm, h;
		void* map = MAP_FAILED;

		DWORD protect = _mmap_prot_page(prot);
		DWORD desiredAccess = _mmap_prot_file(prot);

		DWORD dwFileOffsetHigh = 0;
		DWORD dwFileOffsetLow = (DWORD)off;

		DWORD dwMaxSizeHigh = 0;
		DWORD dwMaxSizeLow = (DWORD)(off + len);

		errno = 0;

		if (!len
			/* Unsupported flag combinations */
			|| (flags & MAP_FIXED)
			/* Usupported protection combinations */
			|| (PROT_EXEC == prot)) {
			errno = EINVAL;
			return MAP_FAILED;
		}

		h = !(flags & MAP_ANONYMOUS) ? (HANDLE)_get_osfhandle(fildes) : INVALID_HANDLE_VALUE;

		if ((INVALID_HANDLE_VALUE == h) && !(flags & MAP_ANONYMOUS)) {
			errno = EBADF;
			return MAP_FAILED;
		}

		fm = CreateFileMapping(h, NULL, protect, dwMaxSizeHigh, dwMaxSizeLow, NULL);

		if (!fm) {
			errno = _mmap_error(GetLastError(), EPERM);
			return MAP_FAILED;
		}

		map = MapViewOfFile(fm, desiredAccess, dwFileOffsetHigh, dwFileOffsetLow, len);

		CloseHandle(fm);

		if (!map) {
			errno = _mmap_error(GetLastError(), EPERM);
			return MAP_FAILED;
		}

		return map;
	}

	int munmap(void* addr, size_t len) {
		if (!UnmapViewOfFile(addr)) {
			errno = _mmap_error(GetLastError(), EPERM);
			return -1;
		}
		return 0;
	}

	int msync(void* addr, size_t len, int flags) {
		if (!FlushViewOfFile(addr, len)) {
			errno = _mmap_error(GetLastError(), EPERM);
			return -1;
		}
		return 0;
	}

	int mprotect(void* addr, size_t len, int prot) {
		DWORD newProtect = _mmap_prot_page(prot);
		DWORD oldProtect = 0;

		if (!VirtualProtect(addr, len, newProtect, &oldProtect)) {
			errno = _mmap_error(GetLastError(), EPERM);
			return -1;
		}
		return 0;
	}

	int mlock(const void* addr, size_t len) {
		if (!VirtualLock((LPVOID)addr, len)) {
			errno = _mmap_error(GetLastError(), EPERM);
			return -1;
		}
		return 0;
	}

	int munlock(const void* addr, size_t len) {
		if (!VirtualUnlock((LPVOID)addr, len)) {
			errno = _mmap_error(GetLastError(), EPERM);
			return -1;
		}
		return 0;
	}

#endif // MMAP_WIN_IMPLEMENTATION

#pragma endregion MMAP4WIN


#ifdef __cpluplus
} // extern "C" 
#endif // __cpluplus
