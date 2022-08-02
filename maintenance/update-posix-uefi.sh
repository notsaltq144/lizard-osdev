rm -rf uefi
git clone https://gitlab.com/bztsrc/posix-uefi
cp -r posix-uefi/uefi .
rm -rf posix-uefi
git add uefi

echo UPDATE SUCCESSFUL

