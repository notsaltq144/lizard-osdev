rm -rf uefi
git clone https://gitlab.com/bztsrc/posix-uefi
cp -r posix-uefi/uefi .
cp posix-uefi/LICENSE uefi/
cp posix-uefi/README.md uefi/
rm -rf posix-uefi
sed -i '1s;^;#define UEFI_NO_TRACK_ALLOC\n;' uefi/uefi.h
git add uefi

echo UPDATE SUCCESSFUL

