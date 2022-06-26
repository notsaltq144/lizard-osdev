# File Range List Table (FRLT)
FRLT is the file system used for the lizard operating system. This document describes version 1.0 of FRLT. It is little endian.
# The descriptor sector
The descriptor sector is either the first sector on a non-bootable (LBA 0) partition or the second sector (LBA 1) on a bootable partition.

Table 1
| Offset | Length | Definition
| :----: | :----: | :--------:
| 0      | 8      | First sector of FRLT
| 8      | 8      | Length of FRLT
| 510    | 2      | Undefined, must not be 0x55 followed by 0xAA. Recommended to be zero.

# The range list table sector<span style="opacity:.2">s
The FRLT file system uses a list of ranges of sectors for files.

An example of a file:

Table 2
| * | +0  | +1 | +2 | +3 | +4 | +5 | +6 | +7 | +8 | +9 | +A | +B | +C | +D | +E | +F |
| :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-:
| +00 | <span style="color:blue"> FF | <span style="color:blue"> FF | <span style="color:blue"> FF | <span style="color:blue"> FF | <span style="color:blue"> FF | <span style="color:blue"> FF | <span style="color:blue"> FF | <span style="color:blue"> FF | <span style="color:red"> 80 | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:red"> 00
| +10 |  <span style="color:red"> 8F | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:red"> 00 |  <span style="color:red"> C0 | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:red"> 00 |
| +20 | <span style="color:red"> FF | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:red"> 00 | <span style="color:orange"> FF | <span style="color:orange"> FF | <span style="color:orange"> FF | <span style="color:orange"> FF | <span style="color:orange"> FF | <span style="color:orange"> FF | <span style="color:orange"> FF | <span style="color:orange"> 00
| +30 | <span style="color:green"> 2F | <span style="color:green"> 73 | <span style="color:green"> 79 | <span style="color:green"> 73 | <span style="color:green"> 74 | <span style="color:green"> 65 | <span style="color:green"> 6D | <span style="color:green"> 2F | <span style="color:green"> 6B | <span style="color:green"> 65 | <span style="color:green"> 72 | <span style="color:green"> 6E | <span style="color:green"> 65 | <span style="color:green"> 6C | <span style="color:green"> 00 | <span style="color:green"> 00
| +40 | <span style="color:#D95030"> D7 | <span style="color:#D95030"> FC | <span style="color:#D95030"> 00 | <span style="color:#D95030"> 00 | <span style="color:#D95030"> 00 | <span style="color:#D95030"> 00 | <span style="color:#D95030"> 00 | <span style="color:#D95030"> 00 | 95 | 8B | B1 | 42 | 00 | 00 | 00 | 00 |
| +50 | <span style="color:#828282"> 2D | <span style="color:#828282"> D5 | <span style="color:#828282"> 81 | <span style="color:#828282"> 52 | <span style="color:#828282"> 00 | <span style="color:#828282"> 00 | <span style="color:#828282"> 00 | <span style="color:#828282"> 00 | <span style="color:#70c910"> 12 | <span style="color:#70c910"> 00 | <span style="color:#70c910"> 00 | <span style="color:#70c910"> 00 | <span style="color:#70c910"> 00 | <span style="color:#70c910"> 00 | <span style="color:#70c910"> 00 | <span style="color:#70c910"> 00 |

<span style="color:blue"> The blue set of FFs indicates metadata.
At +000 some bits may be inverted:

Table 2.1
| Bit | Definition                                                          |
| :-: | :-----------------------------------------------------------------: |
| 0   | System file. If set this entry and the sectors associated wont move |
| 1   | Hidden. May be interpreted in any way                               |
| 2   | Read-only, should not be modified.                                  |
| 3   | Executable                                                          |
| 4   | Directory                                                           |
| 5   | Error correction (hamming)                                          |
| 6   | Offline, cannot be modified, contents cant be modified, nothing can be modified except this flag. Modifying this flag requires the owner run sudo to modify this. May be offlined due to ECC                                     |

For +001 (owner), +002 (root), +003 (group 0), +004 (group 1), and +005 (global) the following may be inverted for permissions:

Table 2.2
| Bit | Permission |
| :-: | :--------: |
| 0   | Execute    |
| 1   | Read       |
| 2   | Write      |

All* other bits must be 1. If this is not met the file is unusable and accessing it causes undefined results. A permission is granted if any permission that is set (binary cleared) that applies to the accessing context is set (binary cleared).

<span style="color:red"> The first two red 64-bit entries mark that the file starts at sector 80 and stops at 8F. The 3rd and 4th 64-bit entries mark that the file continues at sector C0 and finally stops at FF. If the entry is a directory, then every 8-byte sub-entry is of the following format:

Table 2.3
| Offset | Length | Definition       |
| :----: | :----: | :--------:       |
| 0      | 6      | Sector           |
| 6      | 2      | Offset in sector |

<span style="color:red"> This is the location of a file in the directory. The 8-byte location marks the entry of the file.

<span style="color:orange"> The set of FFs followed by a zero. This marks the end of ranges.

<span style="color:green"> The green value is actually a null-terminated ASCII string. It must have a length that is a multiple of 16. If it is not a multiple of 16 it must be padded with null-bytes at the end until it is a multiple of 16. The string is the name of the file. In this case it is ```/system/kernel```. The entry's end is marked by the last** 16-byte part of the string. If a 16-byte part contains the null-terminator then it is the last** part. The name parts must not have any other data than the name, this is guaranteed to be true because of the structure of the entry.

<span style="color:#D95030"> This is the size of the file in bytes (64727). This is because sectors aren't as precise as bytes and software may read too little or too many bytes.

The white part is the time of creation as a signed 64-bit unix timestamp. (23/7/2005 15:53:34 GMT)

<span style="color:#828282"> The gray part is the time of last modification as a signed 64-bit unix timestamp. (12/11/2013 7:13:49)

<span style="color:#70c910"> The light green part is the parent directory entry as described in table 2.3.

<span style="color:#00ffff"> The light blue part is padding. Must be zero or else the file is unusable and accessing it will cause undefined results. This may be omitted if the entry is already a multiple of 16-bytes. It is not present in this example

Here are C declarations for use by the kernel.
```
enum FRLTAPIpermission {
	Execute = 1,
	Read    = 2,
	Write   = 4
}

struct FRLTAPIentry_o {
	uint64_t   metadata;
	uint64_t[] sectors;
	uint64_t   sectors_end = 0xFFFFFFFFFFFFFF00;
	char[]     name;
	uint64_t   bytesize;
	uint64_t   timeCreation;
	uint64_t   timeLastChange;
	uint64_t   padding = 0;
}

int FRLTAPIchangePermission(
	int permission,
	int holder,
	bool losePermission,
	FRLTAPIentry_o* entry);
// 0 = success
// 1 = fail

int FRLTAPIchangeName(
	char* name,
	FRLTAPIentry_o* entry);
// 0 = success
// 1 = fail

int FRLTAPIchangeSectors(
	uint64_t* sectors,
	uint64_t amount,
	FRLTAPIentry_o* entry);
// 0 = success
// 1 = fail

uint64_t* FRLTAPIgetSectors(
	FRLTAPIentry_o* entry);
// 0 = fail
// r = address of sectors

uint64_t* FRLTAPIgetTimeCreation(
	FRLTAPIentry_o* entry);
// 0 = fail
// r = address of time of creation

uint64_t* FRLTAPIgetTimeLastChange(
	FRLTAPIentry_o* entry);
// 0 = fail
// r = address time of last change

uint64_t* FRLTAPIgetByteSize(
	FRLTAPIentry_o* entry);
// 0 = fail
// r = address of byte size

uint64_t* FRLTAPIgetMetadata(
	FRLTAPIentry_o* entry);
// 0 = fail
// r = address of metadata

int FRLTAPIwrite(char* content,
	uint64_t amount,
	entry_o* entry,
	bool updateTime);
// 0 = success
// 1 = fail

int FRLTAPIcreate(char* name,
	bool updateTime);
// 0 = success
// 1 = fail

FRLTAPIentry_o* getEntryFromPath(
	const char* path);
// 0 = fail
// r = address of entry

const char* getPathFromEntry(
	FRLTAPIentry_o* entry);
// 0 = fail
// r = c-style string, name of file represented by entry

int FRLTAPIverify(const FRLTAPIentry_o* entry)
// -4 = ECC not enabled
// -3 = access fail
// -2 = irrecoverable damage, unable to offline file
// -1 = irrecoverable damage, file is now offline
// 0  = all good
// 1  = successful repair
```

<br><br><br>

Any set of sectors can be included in a file including the boot sector, kernel and the table itself.

If multiple files have overlapping sectors, modifying one modifies all other files. Deleting one of the files must not clear or modify any sectors that are used in other files, but it can modify or clear sectors only used in the file.

<br/><br/><br/><br/><br/><br/><br/><br/><br/>

&#42; +006 has the owner's user id as a 16-bit unsigned number. +001-+005's more significant half may have error correction data. +001 has the most significant bit and +005 has the least. +001's most significant bit has the overall parity bit. +001's second most significant bit is the toggle for overall parity (recommended). If ECC is enabled, the upper halves store the raw bits, otherwise they're all ones.

&#42;&#42; The following 32 bytes are the last parts.
