#!/bin/bash

sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk hdimage.iso
	g # gpt
	n # new partition
	  # default number
	  # default start
	  # default end
	t # switch partition type (partition because only partition)
	1 # type 1
	w # write
EOF

