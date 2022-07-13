#!/bin/bash

sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk hdimage.iso
	g
	n
	
		
	
	t
	1
	w
EOF

