#
# Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
# All rights reserved. Use of the code is allowed under the
# Artistic License 2.0 terms, as specified in the LICENSE file
# distributed with this code, or available from
# http://www.opensource.org/licenses/artistic-license-2.0.php
#
# This Dockerfile written by JP Vossen and generously donated to the project.
#
# Here is how to run it. It works, but is a tad ugly, I assume due to 
# default GTK themes:
# ----
# ~/working/docker/passwordsafe$ sudo docker run --net=host 
# --env="DISPLAY" --volume="$HOME/.Xauthority:/root/.Xauthority:rw" 
# --volume="$HOME/.pwsafe:/root/.pwsafe:rw" passwordsafe
# ----
#
# See: https://hub.docker.com/_/debian and
# https://hub.docker.com/_/ubuntu

FROM ubuntu:18.04

# Build with: sudo docker build -t passwordsafe .
# --or--      sudo docker build -t passwordsafe:1.08BETA .
# Run with: sudo docker run --net=host --env="DISPLAY" --volume="$HOME/.Xauthority:/root/.Xauthority:rw" --volume="$HOME/.pwsafe:/root/.pwsafe:rw" passwordsafe

LABEL "maintainer"="The Passwordsafe Project <Passwordsafe-linux@lists.sourceforge.net>"

# Install deps for PWSafe (LOTS and LOTS!)
# See https://docs.docker.com/develop/develop-images/dockerfile_best-practices#apt-get
RUN apt-get update \
     && apt-get install -qy libwxgtk3.0-0v5 libxtst6 libxerces-c3.2 libykpers-1-1 libqrencode3 libcurl4 locales \
     && rm -rf /var/lib/apt/lists/*

# Set the locale, per http://jaredmarkell.com/docker-and-locales/
RUN locale-gen en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

# Avoid: (pwsafe:8): Gtk-WARNING **: *: Attempting to set the permissions of `/root/.local/share', but failed: No such file or directory
RUN mkdir -p /root/.local/share

# This is here to leave everything above UNCHANGED as much as possible
# to allow layer caching to work at build time!
ARG deb_file=passwordsafe-ubuntu18-1.08.1-BETA.amd64.deb

LABEL "pwsafe"="$deb_file"

ADD https://github.com/pwsafe/pwsafe/releases/download/1.08.1BETA/$deb_file \
     /tmp/$deb_file

RUN dpkg -i /tmp/$deb_file && rm /tmp/$deb_file

CMD /usr/bin/pwsafe
# ----
# 
# A non-cached build that actually has to do the APT installs takes about 
# 5 minutes, most of which is `apt-get` "Setting up...".
# 
# Build:
# ----
#
# ~/working/docker/passwordsafe$ time sudo docker build -t 
# passwordsafe:1.08BETA .
# Sending build context to Docker daemon  3.584kB
# Step 1/13 : FROM ubuntu:18.04
#   ---> 1d9c17228a9e
# Step 2/13 : LABEL "maintainer"="The Passwordsafe Project 
# <Passwordsafe-linux@lists.sourceforge.net>"
#   ---> Using cache
#   ---> 7740484d49d6
# Step 3/13 : RUN apt-get update     && apt-get install -qy 
# libwxgtk3.0-0v5 libxtst6 libxerces-c3.2 libykpers-1-1 libqrencode3 
# libcurl4 locales     && rm -rf /var/lib/apt/lists/*
#   ---> Using cache
#   ---> a27408669b39
# Step 4/13 : RUN locale-gen en_US.UTF-8
#   ---> Using cache
#   ---> 5a77b0e1353a
# Step 5/13 : ENV LANG en_US.UTF-8
#   ---> Using cache
#   ---> 2f130971ca1f
# Step 6/13 : ENV LANGUAGE en_US:en
#   ---> Using cache
#   ---> 5e3b97476bcb
# Step 7/13 : ENV LC_ALL en_US.UTF-8
#   ---> Using cache
#   ---> 3fab6d434cb1
# Step 8/13 : RUN mkdir -p /root/.local/share
#   ---> Using cache
#   ---> 853a2a59033b
# Step 9/13 : ARG deb_file=passwordsafe-ubuntu18-1.08.1-BETA.amd64.deb
#   ---> Using cache
#   ---> b8c13697e68e
# Step 10/13 : LABEL "pwsafe"="$deb_file"
#   ---> Using cache
#   ---> a04a7de3ab67
# Step 11/13 : ADD 
# https://github.com/pwsafe/pwsafe/releases/download/1.08.1BETA/$deb_file 
#   /tmp/$deb_file
# Downloading  11.26MB/11.26MB
#   ---> Using cache
#   ---> d75d25d248ca
# Step 12/13 : RUN dpkg -i /tmp/$deb_file && rm /tmp/$deb_file
#   ---> Using cache
#   ---> 5d88cddbe9b4
# Step 13/13 : CMD /usr/bin/pwsafe
#   ---> Using cache
#   ---> 00b895e642f0
# Successfully built 00b895e642f0
# Successfully tagged passwordsafe:1.08.1BETA
# 
# real    0m3.823s
# user    0m0.120s
# sys     0m0.040
# ----
# 
# The image is not small because of the large amount of stuff that gets 
# pulled in to meet the deps:
# ----
# ~/working/docker/passwordsafe$ sudo docker image list
# REPOSITORY       TAG        IMAGE ID         CREATED             SIZE
# passwordsafe     1.08BETA   00b895e642f0     2 minutes ago       359MB
# passwordsafe     latest     00b895e642f0     2 minutes ago       359MB
# <none>           <none>     852d5e907158     29 minutes ago      359MB
# <none>           <none>     9d5272a16c46     About an hour ago   359MB
# ...
# ubuntu           18.04      1d9c17228a9e     7 days ago          86.7MB
# ----
