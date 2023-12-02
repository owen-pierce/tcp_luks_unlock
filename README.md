# TCP LUKS Decryptor
A TCP Server / Client model written in C, built to provide an automatic unlock to LUKS encrypted drives through the use of initramfs via Dropbear SSH.

Requirements to Build:
- libsodium-dev
- make
- gcc

Compiling:
- make

Client Requirements:
- dropbear-initramfs

Dropbear SSH prereq:
- Have your authorized keys appended to /etc/dropbear/initramfs/authorized_keys

Generating a key with encryptor:
- on the client system run: ./encryptor ./ YOURPASSWORDHERE

Unpacking / Packing initramfs set the following in initramfs_packing_tool.sh before running:

- initramfs_location=/boot : The location of your initramfs image
- initramfs_name=initrd.img-6.1.0-10-amd64 : The image name
- zst_output_arch_name=root_fs_archive : The name of the output archive
- block_size=13976 : The blocksize
- tmp_file_sys_name=tmp_rootfs : The tmp file system name
- root_file_sys_name=rootfs : The root file system name

To have the client load on startup:
- install the compiled client to tmp_rootfs/bin
- create the data directory and install the key file given by encryptor
- copy "tcprun" to scripts/local-top/
- Now repack the image using the pack function of initramfs_packing_tool.sh

tcp_client

On the Server:
- ./tcp_server

Now when the client boots it will send a beacon to the server which will reply, then receive the encrypted key to unlock the client
which the server will then decrypt and use SSH to connect to the client and unlock it.