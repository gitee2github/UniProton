sh ./get_openamp.sh

echo "######################### build metal #########################"
pushd .
mkdir -p libmetal
cd libmetal
mkdir -p build
cd build
rm -rf *
cmake ../../../component/libmetal -DCMAKE_TOOLCHAIN_FILE=../../../component/libmetal/cmake/platforms/uniproton_arm64_gcc.cmake -DWITH_DOC=OFF -DWITH_EXAMPLES=OFF -DWITH_TESTS=OFF -DWITH_DEFAULT_LOGGER=OFF -DWITH_SHARED_LIB=OFF
make VERBOSE=1 DESTDIR=../output install
if [ $? -ne 0 ];then
	echo "make metal failed!"
	exit 1
fi
popd

echo "######################### build openamp #########################"         
pushd .
mkdir -p open-amp
cd open-amp
mkdir -p build
cd build
rm -rf *
cmake ../../../component/open-amp -DCMAKE_TOOLCHAIN_FILE=../../../component/open-amp/cmake/platforms/uniproton_arm64_gcc.cmake
make VERBOSE=1 DESTDIR=../output install
if [ $? -ne 0 ];then
        echo "make openamp failed!"
        exit 1
fi
popd

cp ./libmetal/output/usr/local/lib/*.a ../libs
cp ./open-amp/output/usr/local/lib/*.a ../libs
