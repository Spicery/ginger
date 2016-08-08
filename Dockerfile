FROM ubuntu:14.04
MAINTAINER Stephen Leach "sfkleach@gmail.com"
ENV REFRESHED_AT 2016-08-08,07:48
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y make git python3 g++ autoconf uuid-dev libreadline-dev wget rlwrap python-nose guile-2.0
RUN mkdir -p /tmp/rudecgi
WORKDIR /tmp/rudecgi
RUN wget -q http://www.rudeserver.com/cgiparser/download/rudecgi-5.0.0.tar.gz
RUN tar zxf rudecgi-5.0.0.tar.gz
WORKDIR rudecgi-5.0.0
RUN ./configure
RUN make
RUN sudo make install
WORKDIR /tmp
RUN git clone https://github.com/Spicery/ginger.git
WORKDIR /tmp/ginger
RUN autoconf
RUN ./configure
RUN make all
RUN make install-as-is
WORKDIR /
RUN rm -rf /tmp/rudecgi /tmp/ginger
CMD /usr/local/bin/ginger
