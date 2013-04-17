sudo apt-get update
sudo apt-get install -y make git python g++ autoconf uuid-dev libreadline-dev

mkdir /tmp/rudecgi
cd /tmp/rudecgi
wget -q http://www.rudeserver.com/cgiparser/download/rudecgi-5.0.0.tar.gz
tar zxf rudecgi-5.0.0.tar.gz
cd rudecgi-5.0.0
./configure
make
sudo make install

sudo apt-get install rlwrap
SPICERY_HOME=~/Spicery
mkdir $SPICERY_HOME
cd $SPICERY_HOME

git clone https://github.com/Spicery/ginger.git
cd ginger
autoconf; ./configure; make
sudo make install-as-is