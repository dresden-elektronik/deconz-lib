#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "deconz/file.h"

static int utf8ToWString(const char *str, PWCHAR out, unsigned outlen)
{
    *out = '\0';

    if (!str)
        return 0;

    int required_size = MultiByteToWideChar(
        CP_UTF8,       // Code page
        0,             // Flags
        str, // Source UTF-8 string
        -1,           // Length of source string in bytes (-1 for null-terminated)
        NULL,          // Unused - no output buffer yet
        0              // Unused - buffer size is 0
    );

    if (required_size == 0 || outlen < required_size)
        return 0;

    int result = MultiByteToWideChar(
        CP_UTF8,
        0,
        str,
        -1,
        &out[0], // Pointer to the output buffer
        required_size    // Size of the output buffer
    );

    if (result != 0 && result < outlen)
    {
        out[result] = '\0';
        return 1;
    }

    return 0;
}

static int wstringToUtf8(const PWCHAR str, char *out, unsigned outlen)
{
    *out = '\0';

    int required_size = WideCharToMultiByte(
        CP_UTF8,            // Code page
        0,                  // Flags
        str, // Source wide string
        -1,                 // Length of source string in characters (-1 for null-terminated)
        NULL,               // Unused - no output buffer yet
        0,                  // Unused - buffer size is 0
        NULL,               // Unused
        NULL                // Unused
    );

    if (0 < required_size && required_size < outlen)
    {
        int result = WideCharToMultiByte(
            CP_UTF8,
            0,
            str,
            -1,
            &out[0],    // Pointer to the output buffer
            required_size,      // Size of the output buffer in bytes
            NULL,
            NULL
        );

        if (0 < result && result < outlen)
        {
            out[result] = '\0';
            return 1;
        }
    }

    return 0;
}

int FS_OpenFile(FS_File *fp, int flags, const char *path)
{
    HANDLE f;
    WCHAR wpath[MAX_PATH + 1];

    fp->fd = 0;
    fp->flags = 0;

    if (utf8ToWString(path, wpath, MAX_PATH) == 0)
        return 0;

    if ((flags & FS_MODE_RW) == FS_MODE_RW)
    {
        f = CreateFileW(wpath, GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

        if (f == INVALID_HANDLE_VALUE)
        {
            f = 0;
        }
    }
    else if (flags == (FS_MODE_R))
    {
        f = CreateFileW(wpath, GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
        if (f == INVALID_HANDLE_VALUE)
        {
            f = 0;
        }
    }
    else
    {
        f = 0;
    }

    if (f)
    {
        fp->fd = f;
        fp->flags = flags;
        FS_SeekFile(fp, 0L, FS_SEEK_SET);
        return 1;
    }

    return 0;
}

int FS_FileExists(const char *path)
{
    WCHAR wpath[MAX_PATH + 1];
    WIN32_FIND_DATA findFileData;

    if (utf8ToWString(path, wpath, MAX_PATH))
    {
        HANDLE handle = FindFirstFileW(wpath, &findFileData);

        if (handle != INVALID_HANDLE_VALUE) {
            FindClose(handle); // Close the handle to avoid resource leaks
            return 1;       // File exists
        }
    }

    return 0;
}

int FS_CloseFile(FS_File *fp)
{
    if (fp->fd)
    {
        if (CloseHandle(fp->fd))
        {
            fp->fd = 0;
            fp->flags = 0;
            return 1;
        }
    }
    fp->fd = 0;
    fp->flags = 0;
    return 0;
}

long FS_GetFileSize(FS_File *fp)
{
    DWORD szlo;
    DWORD szhi;

    if (fp && fp->fd)
    {
        szlo = GetFileSize(fp->fd, &szhi);
        if (szlo == INVALID_FILE_SIZE)
            return 0;

        return (long)szlo;
    }

    return 0;
}

long FS_ReadFile(FS_File *fp, void *buf, long max)
{
    DWORD n;
    long result;

    result = 0;
    if (fp && fp->fd && max > 0)
    {
        if (ReadFile(fp->fd, buf, (DWORD)max, &n, NULL))
            result = (long)n;
    }

    return result;
}

long FS_WriteFile(FS_File *fp, const void *buf, long size)
{
    DWORD n;
    long result;

    result = 0;
    if (fp && fp->fd && size > 0 && (fp->flags & FS_MODE_RW))
    {
        if (WriteFile(fp->fd, buf, (DWORD)size, &n, NULL))
        {
            if (n > 0)
                result = (long)n;
        }
    }

    return result;
}

int FS_SeekFile(FS_File *fp, long offset, int whence)
{
    if      (whence == FS_SEEK_SET) whence = FILE_BEGIN;
    else if (whence == FS_SEEK_CUR) whence = FILE_CURRENT;
    else if (whence == FS_SEEK_END) whence = FILE_END;
    else
    {
        return 0;
    }

    if (fp && fp->fd)
    {
        if (INVALID_SET_FILE_POINTER != SetFilePointer(fp->fd, offset, 0, whence))
            return 1;
    }

    return 0;
}

int FS_TruncateFile(FS_File *fp, long size)
{
    if (fp && fp->fd && size >= 0)
    {
        if (FS_SeekFile(fp, size, FS_SEEK_SET))
        {
            if (SetEndOfFile(fp->fd))
                return 1;
        }
    }

    return 0;
}

int FS_DeleteFile(const char *path)
{
    WCHAR wpath[MAX_PATH + 1];

    if (utf8ToWString(path, wpath, MAX_PATH) == 0)
        return 0;

    if (DeleteFileW(wpath))
        return 1;

    return 0;
}

int FS_OpenDir(FS_Dir *dir, const char *path)
{
    int i;
    WCHAR szDir[MAX_PATH + 1];
    WCHAR wpath[MAX_PATH + 1];
    WCHAR fname[0xFF + 1];
    WIN32_FIND_DATAW ffd;

    if (utf8ToWString(path, wpath, MAX_PATH) == 0)
        return 0;

    dir->p = 0;
    dir->entry.type = FS_TYPE_UNKNOWN;
    dir->entry.name[0] = '\0';

    // Check that the input path plus 3 is not longer than MAX_PATH.
    // Three characters are for the "\*" plus NULL appended below.
    for (i = 0; path[i]; i++)
    {}

    if (i > (MAX_PATH - 3))
        return 0;

    StringCchCopyW(szDir, MAX_PACKAGE_NAME, wpath);
    StringCchCatW(szDir, MAX_PATH, TEXT("\\*"));

    dir->p = FindFirstFileW(szDir, &ffd);

    if (dir->p == INVALID_HANDLE_VALUE)
    {
        dir->p = 0;
        return 0;
    }

    // copy first entry
    if (wstringToUtf8(ffd.cFileName, dir->entry.name, sizeof(dir->entry.name) - 1))
    {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            dir->entry.type = FS_TYPE_DIRECTORY;
        }
        else
        {
            dir->entry.type = FS_TYPE_FILE;
        }

        dir->state = 1; // mark that first read is already filled

        return 1;
    }

    // should not happen
    FS_CloseDir(dir);
    return 0;
}

int FS_ReadDir(FS_Dir *dir)
{
    int i;
    WIN32_FIND_DATAW ffd;

    if (dir->state == 1) // first call return existing result
    {
        dir->state = 0;
        return 1;
    }

    dir->entry.type = FS_TYPE_UNKNOWN;
    dir->entry.name[0] = '\0';

    if (FindNextFileW(dir->p, &ffd))
    {
        if (wstringToUtf8(ffd.cFileName, dir->entry.name, sizeof(dir->entry.name) - 1))
        {
            if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                dir->entry.type = FS_TYPE_DIRECTORY;
            }
            else
            {
                dir->entry.type = FS_TYPE_FILE;
            }

            return 1;
        }
    }

    return 0;
}

int FS_CloseDir(FS_Dir *dir)
{
    if (dir->p)
    {
        FindClose(dir->p);
        dir->p = 0;
        return 1;
    }
    return 0;
}
