#!/bin/bash
# shellcheck disable=SC1117
# Copyright 2019 Jacob Hrbek <kreyren@rixotstudio.cz>
# Distributed under the terms of the GNU General Public License v3 (https://www.gnu.org/licenses/gpl-3.0.en.html) or later
# Based in part upon 'before-install' from rsplib	(https://raw.githubusercontent.com/dreibh/rsplib/master/ci/before-install), which is:
# 		Copyright (C) 2018-2019 by Thomas Dreibholz <dreibh@iem.uni-due.de> as GPLv3 or any other GPL at your option

# shellcheck source=QA/travis-ci/travis-common.sh
. "QA/travis-ci/travis-common.sh"

# shellcheck source=QA/travis-ci/get-container.sh
. "QA/travis-ci/get-container.sh"

fixme "Travis - before-install.sh disables SC1117 as hotfix"

# Linux as-is
if [ "$TRAVIS_OS_NAME" = linux ] && [ -z "$DOCKER" ] && [ -z "$QEMU" ]; then

	# Nothing else to do
	true

# Linux via Docker
elif [ "$TRAVIS_OS_NAME" = linux ] && [ -n "$DOCKER" ] && [ -z "$QEMU" ]; then
	# The build machine therefore needs a sufficient amount of RAM!
	[ -e "$CONTAINER" ] && sudo docker rm -f "$CONTAINER"
	sudo docker run -d \
		--name "$CONTAINER" \
		--tmpfs /var/cache/pbuilder:rw,exec,dev,size=4g \
		--tmpfs /var/lib/mock:rw,exec,dev,size=4g \
		--cap-add=SYS_ADMIN --cap-add=NET_ADMIN \
		--security-opt apparmor:unconfined \
		-v "$(pwd):/travis" -w /travis \
	"$DOCKER" tail -f /dev/null
	sudo docker ps

# Exherbo via Docker
elif [ "$TRAVIS_OS_NAME" = 'Exherbo Linux' ] && [ -n "$DOCKER" ] && [ -z "$QEMU" ]; then
	# The build machine therefore needs a sufficient amount of RAM!
	[ -e "$CONTAINER" ] && sudo docker rm -f "$CONTAINER"

    # Run docker
	sudo docker run -d \
		--name "$CONTAINER" \
		--cap-add=SYS_ADMIN --cap-add=NET_ADMIN \
		--security-opt apparmor:unconfined \
		-v "$(pwd):/travis" -w /travis \

# FreeBSD via QEMU
# QA: Convert on POSIX-compatible
elif [ "$TRAVIS_OS_NAME" = linux ] && [ "$QEMU" = FreeBSD ]; then

	if [ -n "$VARIANT" ]; then
		sudo mkdir -p /vservers/qemu-freebsd
		sudo chown "$USER:$USER" /vservers/qemu-freebsd
		mkdir -p /vservers/qemu-freebsd/mnt
		cd /vservers/qemu-freebsd || die "Unable to change directory in '/vservers/qemu-freebsd'"

		# Download image
		imageName="FreeBSD-$VARIANT-amd64.raw"
		if [ -e "$imageName.xz" ] && [ ! -e "$imageName" ]; then
			printf '%s\n' "Extracting existing $imageName.xz ..."
			xz -T0 -dk "$imageName.xz"
		elif [ ! -e "$imageName.xz" ] || [ ! -e "$imageName" ]; then
			imageURL="https://download.freebsd.org/ftp/releases/VM-IMAGES/$VARIANT/amd64/Latest/$imageName.xz"
			info "Downloading FreeBSD VM image from $imageURL ..."
			curl "$imageURL" | tee "$imageName.xz.tmp" | xz -T0 -d - ">$imageName"
			mv "$imageName.xz.tmp" "$imageName.xz" || die "Unable to rename $imageName.xz.tmp in  $imageName.xz"
		fi

		# Modify FreeBSD image
		pushd /vservers/qemu-freebsd/

		# Grow image
		truncate -s 128G "$imageName"
		echo -e "w\nY\nY\n" | LANG=C gdisk "$imageName"
		parted -s "$imageName" resizepart 3 100%
		parted -s "$imageName" print

		# Mount
		ls -l "$imageName"
		mountPoint="$(pwd)/mnt"

		sudo umount "$mountPoint" 2>/dev/null || true
		LOOPDEVS="$(sudo losetup -j "$imageName" | awk '{ print $1 }' | sed -e "s/:$//g")"
		for loopdev in $LOOPDEVS ;do
			sudo losetup -d "$loopdev" || true
		done

		sudo losetup -P -f "$imageName"
		sudo losetup -j "$imageName"
		LOOPDEV="$(sudo losetup -j "$imageName" | awk '{ print $1 }' | sed -e "s/:$//g")"

		echo "sudo fuse-ufs2/fuse-ufs/fuse-ufs ${LOOPDEV}p3 $mountPoint -o rw"
		sudo fuse-ufs2/fuse-ufs/fuse-ufs "${LOOPDEV}p3" "$mountPoint" -o rw

		# Modify
		# Set up networking:
		sudo cp "$mountPoint/etc/rc.conf" .
		sudo bash -c "(
			echo \"sshd_enable=\\\"YES\\\"\"
			echo \"ifconfig_vtnet0=\\\"DHCP\\\"\"
			echo \"ifconfig_vtnet1=\\\"inet 192.168.100.100 netmask 255.255.255.0\\\"\"
			echo \"nfs_client_enable=\\\"YES\\\"\"
			echo \"rpc_lockd_enable=\\\"YES\\\"\"
			echo \"rpc_statd_enable=\\\"YES\\\"\"
		) >>$mountPoint/etc/rc.conf"

		# Make sure that FreeBSD uses the latest packages:
		sudo sed -e 's#"pkg+http://pkg.FreeBSD.org/${ABI}/quarterly"#"pkg+http://pkg.FreeBSD.org/${ABI}/latest"#g' -i "$mountPoint/etc/pkg/FreeBSD.conf"

		# Make sure that the /usr/ports directory is there.
		sudo mkdir -p "$mountPoint/usr/ports"

		# Add SSH public key authentication:
		sudo mkdir -p "$mountPoint/root/.ssh"
		sudo chmod 700 "$mountPoint/root/.ssh"
		if [ ! -e "$HOME/.ssh/id_rsa" ]; then
			ssh-keygen -t rsa -b 4096 -P "" -f "$HOME/.ssh/id_rsa"
		fi
		sudo cp "$HOME/.ssh/id_rsa.pub" "$mountPoint/root/.ssh/authorized_keys"
		sudo chmod 600 "$mountPoint/root/.ssh/authorized_keys"

		sudo bash -c "echo \"PermitRootLogin prohibit-password\" >>$mountPoint/etc/ssh/sshd_config"
		# sudo tail -n 6 ${mountPoint}/etc/ssh/sshd_config

		# Set up NFS:
		sudo mkdir -p "$mountPoint/travis"
		sudo sed -e "/^.*\/travis.*$/d" -i~ "$mountPoint/etc/fstab"
		sudo bash -c "( echo \"192.168.100.1:/travis /travis   nfs   rw,soft,async,noatime,nfsv3,rsize=65536,wsize=65536   0   0\" ; echo \"tmpfs /usr/ports tmpfs rw 0 0\" ) >>$mountPoint/etc/fstab"
		sudo cat "$mountPoint/etc/fstab"

		# Unmount
		sudo umount "$mountPoint"
		sudo losetup -d "$LOOPDEV"

		popd

		# Host-only networking
		sudo ip link add br0 type bridge || true
		sudo ip addr flush dev br0
		sudo ip addr add 192.168.100.1/24 brd 192.168.100.255 dev br0
		sudo ip tuntap add mode tap
		sudo ip link set tap0 master br0
		sudo ip link set dev br0 up
		sudo ip link set dev tap0 up
		sudo iptables -A INPUT  -i tap0 -s 192.168.100.0/24 -j ACCEPT
		sudo iptables -A OUTPUT -o tap0 -d 192.168.100.0/24 -j ACCEPT
		sudo iptables -A INPUT  -i br0  -s 192.168.100.0/24 -j ACCEPT
		sudo iptables -A OUTPUT -o br0  -d 192.168.100.0/24 -j ACCEPT
		sudo iptables -A FORWARD -s 192.168.100.0/24 -d 192.168.100.0/24
		# sudo dnsmasq --interface=br0 --bind-interfaces \
		#   --dhcp-range=192.168.100.2,192.168.100.254 || true


		# NFS
		sudo mkdir -p /travis
		sudo mount --bind "$(pwd)" /travis
		sudo bash -c "echo \"/travis 192.168.100.0/24(rw,no_root_squash)\" >/etc/exports"
		env LANG=C.UTF-8 ci/retry -t "$RETRY_MAXTRIALS" -p "$RETRY_PAUSE" -- sudo apt-get install -y qemu-kvm nfs-kernel-server
		sudo exportfs -v
		sudo service nfs-kernel-server restart

		# ====== Start VM =====================================================
		sudo killall -q qemu-system-x86_64 || true
		ssh-keygen -R "[localhost]:8829" -f ~/.ssh/known_hosts
		# Non-KVM execution: qemu-system-x86_64 \
		sudo qemu-system-x86_64 -machine type=pc,accel=kvm -nographic \
			-m 6144 -cpu host -smp "$(nproc)" \
			-drive "if=virtio,media=disk,file=/vservers/qemu-freebsd/$imageName,format=raw" \
			-netdev user,id=mynet0,hostfwd=tcp:127.0.0.1:8829-:22 -device virtio-net-pci,netdev=mynet0 \
			-netdev tap,id=network0,ifname=tap0,script=no,downscript=no -device virtio-net-pci,netdev=network0,mac=00:00:00:00:00:00 \
		&

		ready=0
		trials=20 ; sleep=15
		i=0 ; while [ "$i" -lt "$trials" ]; do
			# QA: Convert on POSIX compatible
			# shellcheck disable=2219
			let i=$i+1
			echo "$i/$trials: Waiting for VM to boot ..."
			sleep "$sleep"
			if ssh -p 8829 -oStrictHostKeyChecking=no -i "$HOME/.ssh/id_rsa" root@localhost hostname; then
				ready=1
				break
			fi
		done
			if [ $ready -eq 0 ]; then
					echo >&2 "VM did not boot properly!"
					exit 1
			fi

		# Download fuse-ufs2
		# if [ ! -d fuse-ufs2/ ]; then
		# 	info "Downloading fuse-ufs2 ..."
		# 	git clone https://github.com/dreibh/fuse-ufs2 -b dreibh/ubuntu-disco-fix
		# 	# git clone https://github.com/mkatiyar/fuse-ufs2
		# fi

		# Get repository
		ssh -p 8829 -oStrictHostKeyChecking=no -i "$HOME/.ssh/id_rsa" root@localhost \
			git clone https://github.com/Kreyrock/Kreyrock.git

		unset imageName imageURL
	fi

# MacOS X
elif [ "$TRAVIS_OS_NAME" = osx ]; then

	# Nothing to be done here.
	true

else
	die "Invalid setting of TRAVIS_OS_NAME=$TRAVIS_OS_NAME, DOCKER=$DOCKER, QEMU=$QEMU!"
fi
