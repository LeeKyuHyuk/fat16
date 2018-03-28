disk:
	tar -xJf prepared/disk.tar.xz
	file disk.img

clean:
	rm -v disk.img
