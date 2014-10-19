wget http://kozos.jp/books/makeos/binutils-2.19.1.tar.gz
tar zxvf binutils-2.19.1.tar.gz
cd binutils-2.19.1
./configure --target=h8300-elf --disable-nls --disable-werror
make -j 8
sudo make install
cd ..

wget http://kozos.jp/books/makeos/gcc-3.4.6.tar.gz
tar zxvf gcc-3.4.6.tar.gz
patch gcc-3.4.6/gcc/config/h8300/h8300.c < ./h8_64.patch
cd gcc-3.4.6

./configure --target=h8300-elf --disable-nls--disable-threads \
  --disable-shared --enable-languages=c --disable-werror
make -j 8
sudo make install
cd ..

rm gcc-*
rm binutils-*
