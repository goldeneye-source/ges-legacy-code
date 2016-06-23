#!/bin/bash -e

if ! [ -d "./ges_build" ]; then
  mkdir ges_build
fi

pushd ges_build 1> /dev/null

# Only configure if we haven't done it yet
if ! [ -f "Makefile" ]; then
  LDFLAGS="-L ../Extras/lib32" ../configure --prefix=`pwd`/bin --enable-shared --build=i686-pc-linux-gnu
else
  echo "No need to configure!"
  sleep 1
fi

if ! [ -f ./libpython3.4m.a ]; then
  # Build Python
  make -j 4
  make install
else
  echo "No need to build Python!"
fi

echo "Deploying Python to build directories..."
sleep 1

set -x

# Copy pyconfig.h
cp -v pyconfig.h ../../../public/python/pyconfig.h

# Copy the shared library (eventually deploys to $GES_PATH/bin)
cp -v libpython3.4m.a ../../../lib/python/

set +x 

popd 1> /dev/null
