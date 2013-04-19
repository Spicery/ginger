###############################################################################
# Install the dependencies for building Ginger.
###############################################################################

sudo apt-get update
sudo apt-get install -y make git python g++ autoconf uuid-dev libreadline-dev

# RudeCGI is part of the Ginger dependencies that needs separate download.

mkdir /tmp/rudecgi
cd /tmp/rudecgi
wget -q http://www.rudeserver.com/cgiparser/download/rudecgi-5.0.0.tar.gz
tar zxf rudecgi-5.0.0.tar.gz
cd rudecgi-5.0.0
./configure
make
sudo make install

# Now install the pre-requisites for the Ginger runtime.

sudo apt-get install -y rlwrap

###############################################################################
# And then build Ginger from the GitHub repo.
###############################################################################

SPICERY_HOME=~/Spicery
mkdir $SPICERY_HOME
cd $SPICERY_HOME

git clone https://github.com/Spicery/ginger.git
cd ginger
autoconf; ./configure; make
sudo make install-as-is

###############################################################################
# Followed by building the Gingerdocs
###############################################################################

cd $SPICERY_HOME
git clone https://github.com/Spicery/gingerdocs.git
cd gingerdocs
make html
make install-as-is

###############################################################################
# And finally constructing the tarball
###############################################################################

cd $SPICERY_HOME
cd ginger
make -f MakeTarball tarball-as-is

# Copying back out.
cp _build/ginger.tgz /vagrant


###############################################################################
