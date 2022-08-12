# Title

The title of this document is "lizard operating system ABI for amd64 compatible processors, version 1 draft 1 work in progress, main document version 1 draft 1 work in progress"

# License Notice
    Copyright (C) 2022 saltq.
    Permission is granted to copy, distribute and/or modify this document
    under the terms of the GNU Free Documentation License, Version 1.3
    or any later version published by the Free Software Foundation;
    with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
    A copy of the license is included in the section entitled "GNU
    Free Documentation License".


# Terminology

The terminology of this document is not [RFC 2119](https://www.ietf.org/rfc/rfc2119.txt)

Should: A requirement for all implementations.

Recommended: Not required for implementations. It can be expected to exist when using major implementations.

# Data encoding

## Endianness

All data is little endian unless specified otherwise.

## Data type sizes

Here is a table of data sizes

| Data type | Size in bytes | Signed-ness |
| :-------- | :------------ | :---------- |
| char      | 1             | signed      |
| short     | 2             | signed      |
| int       | 4             | signed      |
| float     | 4             | signed      |
| double    | 8             | signed      |
| long      | 8             | signed      |
| pointer   | 8             | signed      |

A data type of `(u/)int(num)_t` is signed if the u is missing, otherwise unsigned. It has a bit size of num, and num should be a multiple of 8 (`num % 8 == 0`). A programming language may specify otherwise, in which case the ABI should be considered correct. A programming language may specify other types, in which case the programming language is correct.

## Programming language syntaxes

Assembly: All assembly code is in `nasm` format.

# Function calls

## Parameters

The first 12 parameters are passed in the following registers, with the first register being the leftmost argument: `rax`, `rbx`, `rcx`, `rdx`, `rsi`, `rdi`, `r8`, `r9`, `r10`, `r11`, `r12`, `r13`. If there are 13 parameters, the 13th parameter is in `r14`. If there are 14 or more parameters, `r14` is a pointer to a structure with the remaining parameters. The structure here is represented as a C-ish `struct`:
```
struct *r14 {
	uint64_t arg13;
	uint64_t arg14;
	uint64_t arg15;
	uint64_t arg16;
	uint64_t arg17;
	...
	...
	uint64_t arg4000;
}
```
Here you can see that the corresponding function has 4000 arguments, which is unrealistic, but it is a good example

If an argument is bigger than 64 bits, then the corresponding arg(x) is a pointer to the actual argument. If an argument is smaller than 64 bits, then the corresponding arg(x) is truncated.

## Return value

The return value of a function is stored in `rax`. If the return value is bigger than 64 bits, rax contains a pointer to the actual return value.

If a dynamic amount of memory is to be returned, a pointer to the data should be returned. It should be allocated using the operating system defined way.

If there is no way to do that, but a dynamic amount of memory is to be returned, it should be returned on the stack.

If a function implements both the operating system defined and stack return, the return value should be a pointer to the allocation or if the stack is used, the return value should be an immediate value that cannot be a normal allocation (0, 1, pointer to kernel memory, etc).

## Preserved registers

Here are all of the general-ish purpose registers, their usage and if they are preserved.

| Register | Usage | Preserved |
| :------- | :---- | :-------- |
| rax | Return value | No |
| rbx | General Purpose | No |
| rcx | General Purpose | No |
| rdx | General Purpose | Yes |
| rsi | Source Pointer | Yes |
| rdi | Destination Pointer | Yes |
| rbp | Frame base pointer | Yes |
| rsp | Stack pointer | Yes |
| rip | Instruction pointer | 1 |
| r8-r11 | General purpose | Yes |
| r12-r14 | General purpose | No |
| r15 | Reserved for future ABI use | 2 |
| EFLAGS | Flags | No |
| GDTR, IDTR, K0-K8, CR0-CR8, etc | Non-general purpose | 3 |
| Other | Other | Yes, 4 |

1. Address of instruction after call instruction (duh).
2. Should not be modified, unless specified by a future expansion of the ABI.
3. Should not be modified, unless appropriate or allowed by a future expansion of the ABI. Should not be relied on.
4. These might include things such as XMM0-7, YMM0-15, ZMM0-31 and more.

## Stack

Every function should start by saving the `rbp` register (and used preserved registers), and setting the `rbp` register to the `rsp` register, in order to setup the stack frame.
Additionally, when a function gets called, the stack pointer should be aligned on 16 bytes (`rsp % 16 == 0`), **before** the call instruction is executed. As `rbp` is pushed, the stack gets realigned. If there are additional preserved registers, the amount of bits should be 16-byte aligned (`rbp` does not count) (`preserved_registers_total_bit_count % 128 == 0`). If it is not, a `sub rsp, 8` should be executed **before** a `mov rbp, rsp`.
```
_func:
	push rbp
	push rdx
	push rsi
	push r9
	sub rsp, 8
	mov rbp, rsp
```


A function using local variables should allocate space on the stack for them. The sample code assumes a total variable size of 16 bytes.
```
	sub rsp, 16
```
In order to access a local variable, the function should use the `rbp` register plus an offset.
If there are two 64-bit variables (a, b), the following will access both of them:
```
	mov rax, [rbp+0] ; access a
	mov rbx, [rbp+8] ; access b
```
A function should end by first undoing their reservation:
```
	add rsp, 16
```
Then restoring `rbp` (and used preserved registers):
```
	pop rsi
	pop rdx
	pop rbp
```
Followed by a `ret`:
```
	ret
```

As such, a full function could be the following (assuming a C signature of `int _func(int a, int b)`):
```
_func:
	; preserve regs
	push rbp
	push rsi
	push r9
	sub rsp, 8
	mov rbp, rsp

	; allocate space for vars
	sub rsp, 8

	; do things
	mov [rbp+0], rax
	add [rbp+0], rbx
	mov rsi, [rbp+0]
	mov r9, rsi
	rol rsi, 9
	rol r9, 3
	sub rsi, r9
	add [rbp+0], rsi

	; set return value
	mov rax, [rbp+0]

	; unallocate vars
	add rsp, 8

	; get regs back
	pop r9
	pop rsi
	pop rbp

	; return back
	ret
```
This function was designed as an example of the ABI, not of an optimizing compiler or smart programmer. As such, it attempts to showcase every requirement.

# System calls

## Parameters

System call parameters are passed in the same way as normal functions. The first argument should be the system call number, that is what function the system call does (not distinction between printing an error or normal message, but whether to open a file or exit the process). Other arguments are defined by the operating system. There should not be more than 13 arguments.

## Return value
The location of the return value is as normal.
Here is a table of what each range of return value means.

| Return value | Meaning |
| :----------- | :------ |
| >0 | Error |
| 0 | OK |
| <0 | Should not be returned, 1 |

1. This usually would indicate a problem with the system call. If this is returned, it is recommended to immediately go to an infinite loop that does not perform a system call, in order to prevent further problems. Without performing a system call before the loop.

Note: If a system call needs to return an actual value, it should return a pointer to it, or 0 if there was a problem

Note: If a system call returns a pointer to dynamically allocated memory, the caller is responsible for freeing it, using the operating system defined way to do that. If it cannot allocate memory, it should return it on the stack and set the return value to 1.

## Preserved registers

**All** registers except the return value register is preserved. `rip` is obviously set accordingly.

## Stack

When a system call is performed, the stack should be aligned on a 16-byte boundary (`rsp % 16 == 0`). Inside the call, the stack is handled in the same way as a normal function. After the system call, the stack should be equivalent to before system call.

## Calling system calls

System calls are called with `syscall` instruction.

# Dynamic libraries

## Loading system call

The loading system call, loads a dynamic library. This is often useful to do anything useful.

A dynamic library can be loaded using a system call with arg1 equal to 0.

arg1: System call number (0).

arg2: Version of system call (0).

arg3: A pointer to a string that contains the full path of the library file.

return value: A pointer to a structure that has the address of every single variable and function in the dynamic library. Or if there is no way to allocate memory, it should be passed on the stack with fields in reverse order, and type set to 1. The normal location of the return value should contain 1.

The format of the library file is operating system defined.

The return value structure is the following:
```
struct syscall0_1_ret {
	uint64_t struct_size;
	uint64_t type;
	void* func1;
	void* func2;
	void* func3;
	...
	...
	uint64_t seperator = 0;
	void* var1;
	void* var2;
	void* var3;
	...
}
```
The seperator is used by interpreters of the return value. And so is the struct_size. And so is the type. It is recommended to use the operating system defined way to free the allocation of the struct, when it is unused. The system call should use the operating system defined way to allocate memory. If there is no way to do that, the type must be set to 1, otherwise 0. The stack pointer should be reduced according to the struct_size variable, when the stack is returned to normal for a function exit. If there are multiple loads, they should be unallocated one by one in order

A dynamic library cannot be loaded more than once.

## Unloading system call

The unloading system call, unloads a dynamic library. This is often useful to avoid memory leaks.

A dynamic library can be unloaded using a system call with arg1 equal to 1.

arg1: Number of system call (0)

arg2: Version of system call (0)

arg3: The type of input (0/1)

arg4: If arg3 is 0, this is a pointer to a `syscall0_1_ret` structure. If arg3 is 1, this is a pointer to a string that is the full path of the library (should be same as the corresponding load).

This system call returns no value, and as such uses standard returning.

# Application entrypoint.

## Registers

When an application starts, **all** registers except `rsp` and `rip` are zero. The stack should have at least 12MB available. The value of `rsp` and `rip` is unspecified

## Stack

The stack should have at least 12MB available. It is recommended for the entrypoint to immediately stabilize the stack:
```
sub rsp, 16
and rsp, 0xfffffffffffffff0
```
WARNING: This approach will increase `rsp` and as a result, you will have to 
assume 12MB-16B of stack space.

# In case of dead links

In order to avoid problems, here is a list of wayback machine pages for all pages linked:

[RFC 2119](https://web.archive.org/web/20220707154548/https://www.ietf.org/rfc/rfc2119.txt)

# License of this document

The license of this document is GFDL v1.3

### GNU Free Documentation License

Version 1.3, 3 November 2008

Copyright (C) 2000, 2001, 2002, 2007, 2008 Free Software Foundation,
Inc. <https://fsf.org/>

Everyone is permitted to copy and distribute verbatim copies of this
license document, but changing it is not allowed.

#### 0. PREAMBLE

The purpose of this License is to make a manual, textbook, or other
functional and useful document "free" in the sense of freedom: to
assure everyone the effective freedom to copy and redistribute it,
with or without modifying it, either commercially or noncommercially.
Secondarily, this License preserves for the author and publisher a way
to get credit for their work, while not being considered responsible
for modifications made by others.

This License is a kind of "copyleft", which means that derivative
works of the document must themselves be free in the same sense. It
complements the GNU General Public License, which is a copyleft
license designed for free software.

We have designed this License in order to use it for manuals for free
software, because free software needs free documentation: a free
program should come with manuals providing the same freedoms that the
software does. But this License is not limited to software manuals; it
can be used for any textual work, regardless of subject matter or
whether it is published as a printed book. We recommend this License
principally for works whose purpose is instruction or reference.

#### 1. APPLICABILITY AND DEFINITIONS

This License applies to any manual or other work, in any medium, that
contains a notice placed by the copyright holder saying it can be
distributed under the terms of this License. Such a notice grants a
world-wide, royalty-free license, unlimited in duration, to use that
work under the conditions stated herein. The "Document", below, refers
to any such manual or work. Any member of the public is a licensee,
and is addressed as "you". You accept the license if you copy, modify
or distribute the work in a way requiring permission under copyright
law.

A "Modified Version" of the Document means any work containing the
Document or a portion of it, either copied verbatim, or with
modifications and/or translated into another language.

A "Secondary Section" is a named appendix or a front-matter section of
the Document that deals exclusively with the relationship of the
publishers or authors of the Document to the Document's overall
subject (or to related matters) and contains nothing that could fall
directly within that overall subject. (Thus, if the Document is in
part a textbook of mathematics, a Secondary Section may not explain
any mathematics.) The relationship could be a matter of historical
connection with the subject or with related matters, or of legal,
commercial, philosophical, ethical or political position regarding
them.

The "Invariant Sections" are certain Secondary Sections whose titles
are designated, as being those of Invariant Sections, in the notice
that says that the Document is released under this License. If a
section does not fit the above definition of Secondary then it is not
allowed to be designated as Invariant. The Document may contain zero
Invariant Sections. If the Document does not identify any Invariant
Sections then there are none.

The "Cover Texts" are certain short passages of text that are listed,
as Front-Cover Texts or Back-Cover Texts, in the notice that says that
the Document is released under this License. A Front-Cover Text may be
at most 5 words, and a Back-Cover Text may be at most 25 words.

A "Transparent" copy of the Document means a machine-readable copy,
represented in a format whose specification is available to the
general public, that is suitable for revising the document
straightforwardly with generic text editors or (for images composed of
pixels) generic paint programs or (for drawings) some widely available
drawing editor, and that is suitable for input to text formatters or
for automatic translation to a variety of formats suitable for input
to text formatters. A copy made in an otherwise Transparent file
format whose markup, or absence of markup, has been arranged to thwart
or discourage subsequent modification by readers is not Transparent.
An image format is not Transparent if used for any substantial amount
of text. A copy that is not "Transparent" is called "Opaque".

Examples of suitable formats for Transparent copies include plain
ASCII without markup, Texinfo input format, LaTeX input format, SGML
or XML using a publicly available DTD, and standard-conforming simple
HTML, PostScript or PDF designed for human modification. Examples of
transparent image formats include PNG, XCF and JPG. Opaque formats
include proprietary formats that can be read and edited only by
proprietary word processors, SGML or XML for which the DTD and/or
processing tools are not generally available, and the
machine-generated HTML, PostScript or PDF produced by some word
processors for output purposes only.

The "Title Page" means, for a printed book, the title page itself,
plus such following pages as are needed to hold, legibly, the material
this License requires to appear in the title page. For works in
formats which do not have any title page as such, "Title Page" means
the text near the most prominent appearance of the work's title,
preceding the beginning of the body of the text.

The "publisher" means any person or entity that distributes copies of
the Document to the public.

A section "Entitled XYZ" means a named subunit of the Document whose
title either is precisely XYZ or contains XYZ in parentheses following
text that translates XYZ in another language. (Here XYZ stands for a
specific section name mentioned below, such as "Acknowledgements",
"Dedications", "Endorsements", or "History".) To "Preserve the Title"
of such a section when you modify the Document means that it remains a
section "Entitled XYZ" according to this definition.

The Document may include Warranty Disclaimers next to the notice which
states that this License applies to the Document. These Warranty
Disclaimers are considered to be included by reference in this
License, but only as regards disclaiming warranties: any other
implication that these Warranty Disclaimers may have is void and has
no effect on the meaning of this License.

#### 2. VERBATIM COPYING

You may copy and distribute the Document in any medium, either
commercially or noncommercially, provided that this License, the
copyright notices, and the license notice saying this License applies
to the Document are reproduced in all copies, and that you add no
other conditions whatsoever to those of this License. You may not use
technical measures to obstruct or control the reading or further
copying of the copies you make or distribute. However, you may accept
compensation in exchange for copies. If you distribute a large enough
number of copies you must also follow the conditions in section 3.

You may also lend copies, under the same conditions stated above, and
you may publicly display copies.

#### 3. COPYING IN QUANTITY

If you publish printed copies (or copies in media that commonly have
printed covers) of the Document, numbering more than 100, and the
Document's license notice requires Cover Texts, you must enclose the
copies in covers that carry, clearly and legibly, all these Cover
Texts: Front-Cover Texts on the front cover, and Back-Cover Texts on
the back cover. Both covers must also clearly and legibly identify you
as the publisher of these copies. The front cover must present the
full title with all words of the title equally prominent and visible.
You may add other material on the covers in addition. Copying with
changes limited to the covers, as long as they preserve the title of
the Document and satisfy these conditions, can be treated as verbatim
copying in other respects.

If the required texts for either cover are too voluminous to fit
legibly, you should put the first ones listed (as many as fit
reasonably) on the actual cover, and continue the rest onto adjacent
pages.

If you publish or distribute Opaque copies of the Document numbering
more than 100, you must either include a machine-readable Transparent
copy along with each Opaque copy, or state in or with each Opaque copy
a computer-network location from which the general network-using
public has access to download using public-standard network protocols
a complete Transparent copy of the Document, free of added material.
If you use the latter option, you must take reasonably prudent steps,
when you begin distribution of Opaque copies in quantity, to ensure
that this Transparent copy will remain thus accessible at the stated
location until at least one year after the last time you distribute an
Opaque copy (directly or through your agents or retailers) of that
edition to the public.

It is requested, but not required, that you contact the authors of the
Document well before redistributing any large number of copies, to
give them a chance to provide you with an updated version of the
Document.

#### 4. MODIFICATIONS

You may copy and distribute a Modified Version of the Document under
the conditions of sections 2 and 3 above, provided that you release
the Modified Version under precisely this License, with the Modified
Version filling the role of the Document, thus licensing distribution
and modification of the Modified Version to whoever possesses a copy
of it. In addition, you must do these things in the Modified Version:

-   A. Use in the Title Page (and on the covers, if any) a title
    distinct from that of the Document, and from those of previous
    versions (which should, if there were any, be listed in the
    History section of the Document). You may use the same title as a
    previous version if the original publisher of that version
    gives permission.
-   B. List on the Title Page, as authors, one or more persons or
    entities responsible for authorship of the modifications in the
    Modified Version, together with at least five of the principal
    authors of the Document (all of its principal authors, if it has
    fewer than five), unless they release you from this requirement.
-   C. State on the Title page the name of the publisher of the
    Modified Version, as the publisher.
-   D. Preserve all the copyright notices of the Document.
-   E. Add an appropriate copyright notice for your modifications
    adjacent to the other copyright notices.
-   F. Include, immediately after the copyright notices, a license
    notice giving the public permission to use the Modified Version
    under the terms of this License, in the form shown in the
    Addendum below.
-   G. Preserve in that license notice the full lists of Invariant
    Sections and required Cover Texts given in the Document's
    license notice.
-   H. Include an unaltered copy of this License.
-   I. Preserve the section Entitled "History", Preserve its Title,
    and add to it an item stating at least the title, year, new
    authors, and publisher of the Modified Version as given on the
    Title Page. If there is no section Entitled "History" in the
    Document, create one stating the title, year, authors, and
    publisher of the Document as given on its Title Page, then add an
    item describing the Modified Version as stated in the
    previous sentence.
-   J. Preserve the network location, if any, given in the Document
    for public access to a Transparent copy of the Document, and
    likewise the network locations given in the Document for previous
    versions it was based on. These may be placed in the "History"
    section. You may omit a network location for a work that was
    published at least four years before the Document itself, or if
    the original publisher of the version it refers to
    gives permission.
-   K. For any section Entitled "Acknowledgements" or "Dedications",
    Preserve the Title of the section, and preserve in the section all
    the substance and tone of each of the contributor acknowledgements
    and/or dedications given therein.
-   L. Preserve all the Invariant Sections of the Document, unaltered
    in their text and in their titles. Section numbers or the
    equivalent are not considered part of the section titles.
-   M. Delete any section Entitled "Endorsements". Such a section may
    not be included in the Modified Version.
-   N. Do not retitle any existing section to be Entitled
    "Endorsements" or to conflict in title with any Invariant Section.
-   O. Preserve any Warranty Disclaimers.

If the Modified Version includes new front-matter sections or
appendices that qualify as Secondary Sections and contain no material
copied from the Document, you may at your option designate some or all
of these sections as invariant. To do this, add their titles to the
list of Invariant Sections in the Modified Version's license notice.
These titles must be distinct from any other section titles.

You may add a section Entitled "Endorsements", provided it contains
nothing but endorsements of your Modified Version by various
parties—for example, statements of peer review or that the text has
been approved by an organization as the authoritative definition of a
standard.

You may add a passage of up to five words as a Front-Cover Text, and a
passage of up to 25 words as a Back-Cover Text, to the end of the list
of Cover Texts in the Modified Version. Only one passage of
Front-Cover Text and one of Back-Cover Text may be added by (or
through arrangements made by) any one entity. If the Document already
includes a cover text for the same cover, previously added by you or
by arrangement made by the same entity you are acting on behalf of,
you may not add another; but you may replace the old one, on explicit
permission from the previous publisher that added the old one.

The author(s) and publisher(s) of the Document do not by this License
give permission to use their names for publicity for or to assert or
imply endorsement of any Modified Version.

#### 5. COMBINING DOCUMENTS

You may combine the Document with other documents released under this
License, under the terms defined in section 4 above for modified
versions, provided that you include in the combination all of the
Invariant Sections of all of the original documents, unmodified, and
list them all as Invariant Sections of your combined work in its
license notice, and that you preserve all their Warranty Disclaimers.

The combined work need only contain one copy of this License, and
multiple identical Invariant Sections may be replaced with a single
copy. If there are multiple Invariant Sections with the same name but
different contents, make the title of each such section unique by
adding at the end of it, in parentheses, the name of the original
author or publisher of that section if known, or else a unique number.
Make the same adjustment to the section titles in the list of
Invariant Sections in the license notice of the combined work.

In the combination, you must combine any sections Entitled "History"
in the various original documents, forming one section Entitled
"History"; likewise combine any sections Entitled "Acknowledgements",
and any sections Entitled "Dedications". You must delete all sections
Entitled "Endorsements".

#### 6. COLLECTIONS OF DOCUMENTS

You may make a collection consisting of the Document and other
documents released under this License, and replace the individual
copies of this License in the various documents with a single copy
that is included in the collection, provided that you follow the rules
of this License for verbatim copying of each of the documents in all
other respects.

You may extract a single document from such a collection, and
distribute it individually under this License, provided you insert a
copy of this License into the extracted document, and follow this
License in all other respects regarding verbatim copying of that
document.

#### 7. AGGREGATION WITH INDEPENDENT WORKS

A compilation of the Document or its derivatives with other separate
and independent documents or works, in or on a volume of a storage or
distribution medium, is called an "aggregate" if the copyright
resulting from the compilation is not used to limit the legal rights
of the compilation's users beyond what the individual works permit.
When the Document is included in an aggregate, this License does not
apply to the other works in the aggregate which are not themselves
derivative works of the Document.

If the Cover Text requirement of section 3 is applicable to these
copies of the Document, then if the Document is less than one half of
the entire aggregate, the Document's Cover Texts may be placed on
covers that bracket the Document within the aggregate, or the
electronic equivalent of covers if the Document is in electronic form.
Otherwise they must appear on printed covers that bracket the whole
aggregate.

#### 8. TRANSLATION

Translation is considered a kind of modification, so you may
distribute translations of the Document under the terms of section 4.
Replacing Invariant Sections with translations requires special
permission from their copyright holders, but you may include
translations of some or all Invariant Sections in addition to the
original versions of these Invariant Sections. You may include a
translation of this License, and all the license notices in the
Document, and any Warranty Disclaimers, provided that you also include
the original English version of this License and the original versions
of those notices and disclaimers. In case of a disagreement between
the translation and the original version of this License or a notice
or disclaimer, the original version will prevail.

If a section in the Document is Entitled "Acknowledgements",
"Dedications", or "History", the requirement (section 4) to Preserve
its Title (section 1) will typically require changing the actual
title.

#### 9. TERMINATION

You may not copy, modify, sublicense, or distribute the Document
except as expressly provided under this License. Any attempt otherwise
to copy, modify, sublicense, or distribute it is void, and will
automatically terminate your rights under this License.

However, if you cease all violation of this License, then your license
from a particular copyright holder is reinstated (a) provisionally,
unless and until the copyright holder explicitly and finally
terminates your license, and (b) permanently, if the copyright holder
fails to notify you of the violation by some reasonable means prior to
60 days after the cessation.

Moreover, your license from a particular copyright holder is
reinstated permanently if the copyright holder notifies you of the
violation by some reasonable means, this is the first time you have
received notice of violation of this License (for any work) from that
copyright holder, and you cure the violation prior to 30 days after
your receipt of the notice.

Termination of your rights under this section does not terminate the
licenses of parties who have received copies or rights from you under
this License. If your rights have been terminated and not permanently
reinstated, receipt of a copy of some or all of the same material does
not give you any rights to use it.

#### 10. FUTURE REVISIONS OF THIS LICENSE

The Free Software Foundation may publish new, revised versions of the
GNU Free Documentation License from time to time. Such new versions
will be similar in spirit to the present version, but may differ in
detail to address new problems or concerns. See
<https://www.gnu.org/licenses/>.

Each version of the License is given a distinguishing version number.
If the Document specifies that a particular numbered version of this
License "or any later version" applies to it, you have the option of
following the terms and conditions either of that specified version or
of any later version that has been published (not as a draft) by the
Free Software Foundation. If the Document does not specify a version
number of this License, you may choose any version ever published (not
as a draft) by the Free Software Foundation. If the Document specifies
that a proxy can decide which future versions of this License can be
used, that proxy's public statement of acceptance of a version
permanently authorizes you to choose that version for the Document.

#### 11. RELICENSING

"Massive Multiauthor Collaboration Site" (or "MMC Site") means any
World Wide Web server that publishes copyrightable works and also
provides prominent facilities for anybody to edit those works. A
public wiki that anybody can edit is an example of such a server. A
"Massive Multiauthor Collaboration" (or "MMC") contained in the site
means any set of copyrightable works thus published on the MMC site.

"CC-BY-SA" means the Creative Commons Attribution-Share Alike 3.0
license published by Creative Commons Corporation, a not-for-profit
corporation with a principal place of business in San Francisco,
California, as well as future copyleft versions of that license
published by that same organization.

"Incorporate" means to publish or republish a Document, in whole or in
part, as part of another Document.

An MMC is "eligible for relicensing" if it is licensed under this
License, and if all works that were first published under this License
somewhere other than this MMC, and subsequently incorporated in whole
or in part into the MMC, (1) had no cover texts or invariant sections,
and (2) were thus incorporated prior to November 1, 2008.

The operator of an MMC Site may republish an MMC contained in the site
under CC-BY-SA on the same site at any time before August 1, 2009,
provided the MMC is eligible for relicensing.

### ADDENDUM: How to use this License for your documents

To use this License in a document you have written, include a copy of
the License in the document and put the following copyright and
license notices just after the title page:

        Copyright (C)  YEAR  YOUR NAME.
        Permission is granted to copy, distribute and/or modify this document
        under the terms of the GNU Free Documentation License, Version 1.3
        or any later version published by the Free Software Foundation;
        with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
        A copy of the license is included in the section entitled "GNU
        Free Documentation License".

If you have Invariant Sections, Front-Cover Texts and Back-Cover
Texts, replace the "with … Texts." line with this:

        with the Invariant Sections being LIST THEIR TITLES, with the
        Front-Cover Texts being LIST, and with the Back-Cover Texts being LIST.

If you have Invariant Sections without Cover Texts, or some other
combination of the three, merge those two alternatives to suit the
situation.

If your document contains nontrivial examples of program code, we
recommend releasing these examples in parallel under your choice of
free software license, such as the GNU General Public License, to
permit their use in free software.
