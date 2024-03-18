#define _DEFAULT_SOURCE
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#undef _DEFAULT_SOURCE

#include <stdio.h>
#include <errno.h>

#include "deconz/file.h"

int FS_OpenFile(FS_File *fp, int flags, const char *path)
{
    FILE *f;
    struct stat sb;

    fp->fd = 0;
    fp->flags = 0;

    if ((flags & FS_MODE_RW) == FS_MODE_RW)
    {
        f = fopen(path, "r+"); /* fails if file doesn't exist */
        if (!f)
        {
            errno = 0;
            if (stat(path, &sb) == -1)
            {
                if (errno == ENOENT)
                {
                    f = fopen(path, "w+"); /* create new file */
                }
            }
        }
    }
    else if (flags == (FS_MODE_R))
    {
        f = fopen(path, "r");
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
        if (fclose(fp->fd) == 0)
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
    long sz;
    long pos;

    if (fp && fp->fd)
    {
        pos = ftell(fp->fd);
        pos = pos >= 0 ? pos : 0;
        fseek(fp->fd, 0L, SEEK_END);
        sz = ftell(fp->fd);
        fseek(fp->fd, pos, SEEK_SET);
        return sz >= 0 ? sz : 0;
    }

    return 0;
}

long FS_ReadFile(FS_File *fp, void *buf, long max)
{
    size_t n;
    long result;

    result = 0;
    if (fp && fp->fd && max > 0)
    {
        n = fread(buf, 1, (size_t)max, fp->fd);
        if (n > 0)
            result = (long)n;
    }

    return result;
}

long FS_WriteFile(FS_File *fp, const void *buf, long size)
{
    size_t n;
    long result;

    result = 0;
    if (fp && fp->fd && size > 0 && (fp->flags & FS_MODE_RW))
    {
        n = fwrite(buf, 1, (size_t)size, fp->fd);
        if (n > 0)
            result = (long)n;
    }

    return result;
}

int FS_SeekFile(FS_File *fp, long offset, int whence)
{
    if      (whence == FS_SEEK_SET) whence = SEEK_SET;
    else if (whence == FS_SEEK_CUR) whence = SEEK_CUR;
    else if (whence == FS_SEEK_END) whence = SEEK_END;
    else
    {
        return 0;
    }

    if (fp && fp->fd)
    {
        if (fseek(fp->fd, offset, whence) == 0)
            return 1;
    }

    return 0;
}

int FS_TruncateFile(FS_File *fp, long size)
{
    int fd;
    if (fp && fp->fd && size >= 0)
    {
        fflush(fp->fd);
        fd = fileno(fp->fd);
        if (ftruncate(fd, size) == 0)
            return 1;
    }

    return 0;
}

int FS_DeleteFile(const char *path)
{
    if (unlink(path) == 0)
        return 1;

    return 0;
}

int FS_OpenDir(FS_Dir *dir, const char *path)
{
    dir->p = opendir(path);
    dir->entry.type = FS_TYPE_UNKNOWN;
    dir->entry.name[0] = '\0';

    if (dir->p)
        return 1;

    return 0;
}

int FS_ReadDir(FS_Dir *dir)
{
    int i;
    struct dirent *entry;

    dir->entry.type = FS_TYPE_UNKNOWN;
    dir->entry.name[0] = '\0';
    entry = readdir(dir->p);
    if (entry)
    {
        for (i = 0; entry->d_name[i]; i++)
            dir->entry.name[i] = entry->d_name[i];

        dir->entry.name[i] = '\0';

        switch (entry->d_type)
        {
            case DT_BLK:  dir->entry.type = FS_TYPE_BLOCK; break;
            case DT_CHR:  dir->entry.type = FS_TYPE_CHARACTER_DEVICE; break;
            case DT_DIR:  dir->entry.type = FS_TYPE_DIRECTORY; break;
            case DT_FIFO: dir->entry.type = FS_TYPE_FIFO; break;
            case DT_LNK:  dir->entry.type = FS_TYPE_LINK; break;
            case DT_REG:  dir->entry.type = FS_TYPE_FILE; break;
            case DT_SOCK: dir->entry.type = FS_TYPE_SOCKET; break;
            default:      dir->entry.type = FS_TYPE_UNKNOWN; break;
        }

        return 1;
    }

    return 0;
}

int FS_CloseDir(FS_Dir *dir)
{
    if (dir->p)
    {
        closedir(dir->p);
        dir->p = 0;
        return 1;
    }
    return 0;
}
