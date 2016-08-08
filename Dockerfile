FROM ubuntu:14.04
MAINTAINER Stephen Leach "sfkleach@gmail.com"
ENV REFRESHED_AT 2015-03-15
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y make git python g++ autoconf uuid-dev libreadline-dev wget rlwrap
COPY apps/ /tmp/ginger/apps/
COPY projects/ /tmp/ginger/projects/
COPY instruction_set/ /tmp/ginger/instruction_set/
COPY config* /tmp/ginger/
COPY docker-scripts/ /tmp/ginger/docker-scripts/
COPY appginger.png /tmp/ginger/
COPY Makefile.in /tmp/ginger/
COPY autodocs/ /tmp/ginger/autodocs/
COPY COPYING /tmp/ginger/
RUN [ "/bin/dash", "/tmp/ginger/docker-scripts/install-rudecgi" ]
RUN [ "/bin/dash", "/tmp/ginger/docker-scripts/build-ginger" ]
CMD /usr/local/bin/ginger
