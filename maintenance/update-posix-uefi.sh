rm -rf uefi
git clone https://gitlab.com/bztsrc/posix-uefi
cp -r posix-uefi/uefi .
cp posix-uefi/LICENSE uefi/
cp posix-uefi/README.md uefi/
rm -rf posix-uefi
git add uefi

echo UPDATE SUCCESSFUL

