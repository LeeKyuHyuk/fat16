all:
	gcc -O3 src/main.c -o fat16

run:
	./fat16

disk:
	tar -xJf prepared/disk.tar.xz
	file disk.img

clean:
	rm -v disk.img fat16
