#
#! /bin/bash
#
echo "Ensuring that files have no dos-ness..."
cd files
chmod +w *
dos2unix -q *
cd ..
echo "Copying files..."
cd ../../../src/dwfcore
cp -f ../../build/gnu/dwfcore/files/* .
echo "Configuring..."
rm -rf autom4te.cache/
autoreconf --force
echo "Done."

