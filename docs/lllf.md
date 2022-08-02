# Version 1
## Introduction
This document describes the `lizard loadable & linkable` (`L3`) file format
## File format
All values are in little endian. Text is not.

Table 1.  Main header. At start of file
| Offset | Size | Description                                       | Code name |
| :----- | :--- | :---------:                                       | :-------- |
| 0      | 4    | L3 Identification. Must be "L3FF" in UTF-8        | h_ident   |
| 4      | 4    | Version. Must be 1.                               | h_version |
| 8      | 8    | Flags. See Table 2                                | h_flags   |
| 16     | 8    | `CRC-64` of file. Non reflective input and output | h_crc     |
| 24     | 8    | Polynomial of h_crc.                              | h_poly    |
| 32     | 8    | File size. Used by a program loader               | h_filesz  |
| 40     | 8    | Amount of memory to allocate. Used by a program loader | h_memsz |
| 48     | 8    | Address where to load this program.               | h_addr    |
| 56     | 2    | Size of header                                    | h_hdrsz   |
| 58     | 8    | Program entrypoint. File offset                   | h_entry   |
| 66     | 8    | Address of first segment separator. File offset   | h_fssoff  |
| 74     | 6    | Padding.                                          | h_padd    |

Table 2. h_flags
| Bitmask               | Description           | Code name |
| :-------------------- | :-------------------: | :-------- |
| 0x0000_0000_0000_0001 | CRC Enabled           | hf_crc    |
| 0x0000_0000_0000_0002 | Is object file        | hf_obj    |
| 0x0000_0000_0000_0004 | Loads/is dynamic file | hf_dyn    |