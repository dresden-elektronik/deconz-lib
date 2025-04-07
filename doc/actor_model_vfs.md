# Actor Model VFS

## Message IDs

Header: `deconz/am_vfs.h`

```
VFS_M_ID_LIST_DIR_REQ    AM_MESSAGE_ID_COMMON_REQUEST(1)
VFS_M_ID_LIST_DIR_RSP    AM_MESSAGE_ID_COMMON_RESPONSE(1)
VFS_M_ID_READ_ENTRY_REQ  AM_MESSAGE_ID_COMMON_REQUEST(2)
VFS_M_ID_READ_ENTRY_RSP  AM_MESSAGE_ID_COMMON_RESPONSE(2)
VFS_M_ID_CHANGED_NTFY    AM_MESSAGE_ID_COMMON_NOTIFY(5)
VFS_M_ID_ADDED_NTFY      AM_MESSAGE_ID_COMMON_NOTIFY(6)
VFS_M_ID_REMOVED_NTFY    AM_MESSAGE_ID_COMMON_NOTIFY(7)
```


## List directory

### Request

Request up to `max_count` children starting at `index`.

```
U16 tag
STR url
U32 index
U32 max_count
```

### Response

```
U16 tag
U8 status

// status == AM_RESPONSE_STATUS_OK

U32 index
U32 next_index // 0 == done
U32 count

Entry entries[count]
```

```
Entry {
    STR name
    U16 flags
    U16 icon
}
```

**flags** field

```
Bit 0 is_directory
Bits 1-15 reserved (set to 0)
```

## Read entry

Reads the value of an entry. This is for small up to 64-bit in memory values, but not files.

### Request

```
U16 tag
STR url
```

### Response

```
U16 tag
U8 status

// status == AM_RESPONSE_STATUS_OK

STR type
U32 mode
U64 modification_time
* data  U8 | U16 ... blob | str
```

**Mode field**

```
Bit 0 writeable
Bits 16-19 display
     0 auto
     1 hex
     2 binary 
```

## Notify changed

The resource has been updated.

```
STR url
U32 flags
```

**Flags** field

```
Bit 0 recursive
```

If the `recursive` flag is set, the resource is a directory and children should be re-fetched again.

## Notify removed

The resource is no longer available.

```
STR url
```

## Notify added

A new resource was added.

```
STR url
```