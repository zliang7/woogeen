#!/bin/bash

#run from webrtc/src
sudo apt-get install -y libx11-dev:i386 libxext-dev:i386 zlib1g-dev:i386 build-essential;
mkdir glib; 
sudo apt-get install -y libffi-dev:i386;
(cd glib;
apt-get source libglib2.0-dev:i386;
)
(cd glib/glib2.0-2.40.2;
if ./configure "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32"; then
    if make; then
        if sudo make install; then
            echo "successful glib2.0 install"
        else
            echo "failed to install glib2.0";
            exit
        fi
    else
        echo "failed to make glib2.0";
        exit
    fi
else
    echo "failed to configure glib2.0";
    exit
fi
)

mkdir xcomposite;
(cd xcomposite;
apt-get source libxcomposite-dev:i386;
)
(cd xcomposite/libxcomposite-0.4.4;
if ./configure "CLFAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32"; then
    if make; then
        if sudo make install; then
            echo "successfully installed libxcomposite"
        else
            echo "failed to install libxcomposite";
            exit
        fi
    else
        echo "failed to make libxcomposite";
        exit
    fi
else
    echo "failed to configure libxcomposite";
    exit
fi
)

#sudo apt-get install libxcomposite-dev:i386;
#sudo apt-get install libglib2.0-dev:i386;
sudo apt-get install -y libgtk2.0-dev;
sudo apt-get install -y mesa-common-dev:i386;
sudo apt-get install -y libgl1-mesa-dev:i386;
sudo apt-get install -y libpulse-dev libxrender-dev:i386 libexpat1-dev:i386;


sudo ln -s /usr/lib/i386-linux-gnu/libXcomposite.so.1.0.0 /usr/lib/libXcomposite.so;
sudo ln -s /usr/lib/i386-linux-gnu/libgthread-2.0.so.0.4002.0 /usr/lib/libgthread-2.0.so;
sudo ln -s /usr/lib/i386-linux-gnu/libgtk-x11-2.0.so.0.2400.23 /usr/lib/libgtk-x11-2.0.so;
sudo ln -s /usr/lib/i386-linux-gnu/libgdk-x11-2.0.so.0.2400.23 /usr/lib/libgdk-x11-2.0.so;
sudo ln -s /usr/lib/i386-linux-gnu/libatk-1.0.so.0.21009.1 /usr/lib/libatk-1.0.so;
sudo ln -s /usr/lib/i386-linux-gnu/libgio-2.0.so.0.4002.0 /usr/lib/libgio-2.0.so;
sudo ln -s /usr/lib/i386-linux-gnu/libpangoft2-1.0.so.0.3600.3 /usr/lib/libpangoft2-1.0.so;
sudo ln -s /usr/lib/i386-linux-gnu/libgdk_pixbuf-2.0.so.0.3000.7 /usr/lib/libgdk_pixbuf-2.0.so;
sudo ln -s /usr/lib/i386-linux-gnu/libpangocairo-1.0.so.0.3600.3 /usr/lib/libpangocairo-1.0.so;
sudo ln -s /usr/lib/i386-linux-gnu/libcairo.so.2.11301.0 /usr/lib/libcairo.so;
sudo ln -s /usr/lib/i386-linux-gnu/libpango-1.0.so.0.3600.3 /usr/lib/libpango-1.0.so;
sudo ln -s /usr/lib/i386-linux-gnu/libfontconfig.so.1.8.0 /usr/lib/libfontconfig.so;
sudo ln -s /usr/lib/i386-linux-gnu/libgobject-2.0.so.0.4002.0 /usr/lib/libgobject-2.0.so;
sudo ln -s /usr/lib/i386-linux-gnu/libfreetype.so.6.11.1 /usr/lib/libfreetype.so;
sudo ln -s /usr/lib/i386-linux-gnu/libXi.so.6.1.0 /usr/lib/libXi.so
sudo ln -s /usr/local/lib/libglib-2.0.so.0.4002.0 /usr/lib/libglib-2.0.so
