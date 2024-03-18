#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "deconz/file.h"

int FS_OpenFile(FS_File *fp, int flags, const char *path)
{
    HANDLE f;

    fp->fd = 0;
    fp->flags = 0;

    if ((flags & FS_MODE_RW) == FS_MODE_RW)
    {
        f = CreateFileA(path, GENERIC_READ | GENERIC_WRITE,
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
        f = CreateFileA(path, GENERIC_READ,
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
    if (DeleteFileA(path))
        return 1;

    return 0;
}

int FS_OpenDir(FS_Dir *dir, const char *path)
{
    int i;
    TCHAR szDir[MAX_PATH];
    WIN32_FIND_DATA ffd;

    dir->p = 0;
    dir->entry.type = FS_TYPE_UNKNOWN;
    dir->entry.name[0] = '\0';

    // Check that the input path plus 3 is not longer than MAX_PATH.
    // Three characters are for the "\*" plus NULL appended below.
    for (i = 0; path[i]; i++)
    {}

    if (i > (MAX_PATH - 3))
        return 0;

    StringCchCopy(szDir, MAX_PACKAGE_NAME, path);
    StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

    dir->p = FindFirstFileA(szDir, &ffd);

    if (dir->p == INVALID_HANDLE_VALUE)
    {
        dir->p = 0;
        return 0;
    }

    // copy first entry
    for (i = 0; ffd.cFileName[i]; i++)
        dir->entry.name[i] = ffd.cFileName[i];

    dir->entry.name[i] = '\0';

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

int FS_ReadDir(FS_Dir *dir)
{
    int i;
    WIN32_FIND_DATA ffd;

    if (dir->state == 1) // first call return existing result
    {
        dir->state = 0;
        return 1;
    }

    dir->entry.type = FS_TYPE_UNKNOWN;
    dir->entry.name[0] = '\0';

    if (FindNextFile(dir->p, &ffd))
    {
        for (i = 0; ffd.cFileName[i]; i++)
            dir->entry.name[i] = ffd.cFileName[i];

        dir->entry.name[i] = '\0';

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
