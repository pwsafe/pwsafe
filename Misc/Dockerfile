#
# Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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

FROM ubuntu:22.04

# Build with: sudo docker build -t passwordsafe .
# --or--      sudo docker build -t passwordsafe:1.18.2 .
# Run with: sudo docker run --net=host --env="DISPLAY" --volume="$HOME/.Xauthority:/root/.Xauthority:rw" --volume="$HOME/.pwsafe:/root/.pwsafe:rw" passwordsafe

LABEL "maintainer"="The Passwordsafe Project <Passwordsafe-linux@lists.sourceforge.net>"

# Install deps for PWSafe (LOTS and LOTS!)
# See https://docs.docker.com/develop/develop-images/dockerfile_best-practices#apt-get

# Prevent prompting for info during package install, e.g., timezone
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
     && apt-get install -qy libwxgtk3.0-gtk3-0v5 libxtst6 libxerces-c3.2 libykpers-1-1 \
     libqrencode4 libcurl4 libmagic1 locales xdg-utils \
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
ARG deb_file=passwordsafe-ubuntu22-1.18.2-amd64.deb

LABEL "pwsafe"="$deb_file"

ADD https://github.com/pwsafe/pwsafe/releases/download/1.18.2/$deb_file \
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
# ~/working/docker/passwordsafe$ time sudo docker build -t passwordsafe:1.18.2 .
# Sending build context to Docker daemon  7.168kB
# 
# Step 1/14 : FROM ubuntu:22.04
#  ---> fd1d8f58e8ae
# Step 2/14 : LABEL "maintainer"="The Passwordsafe Project <Passwordsafe-linux@lists.sourceforge.net>"
#  ---> Running in eb506b6e0f83
# Removing intermediate container eb506b6e0f83
#  ---> f7010ca1d713
# Step 3/14 : ARG DEBIAN_FRONTEND=noninteractive
#  ---> Running in 7bfab4ac95f5
# Removing intermediate container 7bfab4ac95f5
#  ---> 849e3b9c6f81
# Step 4/14 : RUN apt-get update      && apt-get install -qy libwxgtk3.0-gtk3-0v5 libxtst6 libxerces-c3.2 libykpers-1-1      libqrencode4 libcurl4 libmagic1 locales xdg-utils      && rm -rf /var/lib/apt/lists/*
#  ---> Running in 7c8ced5bdfe6
# ...
# Removing intermediate container 7c8ced5bdfe6
 # ---> 3633843b0a5f
# Step 5/14 : RUN locale-gen en_US.UTF-8
#  ---> Running in 336486b5bb3b
# Generating locales (this might take a while)...
#   en_US.UTF-8... done
# Generation complete.
# Removing intermediate container 336486b5bb3b
#  ---> e9b98c278598
# Step 6/14 : ENV LANG en_US.UTF-8
#  ---> Running in 4a2f0074cbbf
# Removing intermediate container 4a2f0074cbbf
#  ---> 09c480bea6cb
# Step 7/14 : ENV LANGUAGE en_US:en
#  ---> Running in 0dadf09b3b5e
# Removing intermediate container 0dadf09b3b5e
#  ---> c3801b7b4a07
# Step 8/14 : ENV LC_ALL en_US.UTF-8
#  ---> Running in f73b90534c90
# Removing intermediate container f73b90534c90
#  ---> e21a133ac663
# Step 9/14 : RUN mkdir -p /root/.local/share
#  ---> Running in 4ec087679c0e
# Removing intermediate container 4ec087679c0e
#  ---> d134920fd14b
# Step 10/14 : ARG deb_file=passwordsafe-ubuntu22-1.18.2-amd64.deb
#  ---> Running in 41d28719c32d
# Removing intermediate container 41d28719c32d
#  ---> 3ab964b6ec38
# Step 11/14 : LABEL "pwsafe"="$deb_file"
#  ---> Running in db7ddded96c0
# Removing intermediate container db7ddded96c0
#  ---> b3a49a7f7cc4
# Step 12/14 : ADD https://github.com/pwsafe/pwsafe/releases/download/1.18.2/$deb_file      /tmp/$deb_file
# ...
#  ---> 5441b9460729
# Step 13/14 : RUN dpkg -i /tmp/$deb_file && rm /tmp/$deb_file
#  ---> Running in eb9e4c713920
# Selecting previously unselected package passwordsafe.
# (Reading database ... 26621 files and directories currently installed.)
# Preparing to unpack .../passwordsafe-ubuntu22-1.18.2-amd64.deb ...
# Unpacking passwordsafe (1.18.2) ...
# Setting up passwordsafe (1.18.2) ...
# Processing triggers for hicolor-icon-theme (0.17-2) ...
# Removing intermediate container eb9e4c713920
#  ---> adf837163c20
# Step 14/14 : CMD /usr/bin/pwsafe
#  ---> Running in b3ee23483787
# Removing intermediate container b3ee23483787
#  ---> 4d7a457ff26b
# Successfully built 4d7a457ff26b
# Successfully tagged passwordsafe:1.18.2
# 
# real	2m32.595s
# user	0m0.165s
# sys	0m0.192s
# ----
# 
# The image is not small because of the large amount of stuff that gets 
# pulled in to meet the deps:
# ----
# docker image list
# REPOSITORY     TAG       IMAGE ID       CREATED         SIZE
# passwordsafe   1.18.2    4d7a457ff26b   3 minutes ago   712MB
# ----
