# OpSysFinal

The objective of this project is to graphically show how threads and semaphores work together.

In order to run this program you must first install the following libs:

sudo apt-get install build-essential

sudo apt-get install libsdl-image1.2 libsdl-image1.2-dev guile-1.8 \
guile-1.8-dev libsdl1.2debian libart-2.0-dev libaudiofile-dev \
libesd0-dev libdirectfb-dev libdirectfb-extra libfreetype6-dev \
libxext-dev x11proto-xext-dev libfreetype6 libaa1 libaa1-dev \
libslang2-dev libasound2 libasound2-dev

Next you must download libgraph from http://download.savannah.gnu.org/releases/libgraph/libgraph-1.0.2.tar.gz and do the following commands:

./configure
make
sudo make install
sudo cp /usr/local/lib/libgraph.* /usr/lib
