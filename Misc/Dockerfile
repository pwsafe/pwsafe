#
# Copyright (c) 2003-2022 Rony Shapiro <ronys@pwsafe.org>.
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

FROM ubuntu:20.04

# Build with: sudo docker build -t passwordsafe .
# --or--      sudo docker build -t passwordsafe:1.10 .
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
ARG deb_file=passwordsafe-ubuntu20-1.13-amd64.deb

LABEL "pwsafe"="$deb_file"

ADD https://github.com/pwsafe/pwsafe/releases/download/1.13.0/$deb_file \
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
# passwordsafe:1.13 .
# Sending build context to Docker daemon  3.584kB
# Step 1/14 : FROM ubuntu:20.04
#  ---> 7e0aa2d69a15
# Step 2/14 : LABEL "maintainer"="The Passwordsafe Project <Passwordsafe-linux@lists.sourceforge.net>"
#  ---> Using cache
#  ---> f73b08debbee
# Step 3/14 : ARG DEBIAN_FRONTEND=noninteractive
#  ---> Using cache
#  ---> 87c43ae97eab
# Step 4/14 : RUN apt-get update      && apt-get install -qy libwxgtk3.0-gtk3-0v5 libxtst6 libxerces-c3.2 libykpers-1-1      libqrencode4 libcurl4 libmagic1 locales xdg-utils      && rm -rf /var/lib/apt/lists/*
#  ---> Running in da96e7b04f12
# Removing intermediate container da96e7b04f12
#  ---> cc4b6053ddd9
# Step 5/14 : RUN locale-gen en_US.UTF-8
#  ---> Running in 02f5d5470f45
# Removing intermediate container 02f5d5470f45
#  ---> 8a497900a15d
# Step 6/14 : ENV LANG en_US.UTF-8
#  ---> Running in 30bf289633f9
# Removing intermediate container 30bf289633f9
#  ---> fbd278ea5c8c
# Step 7/14 : ENV LANGUAGE en_US:en
#  ---> Running in faaae12b16f8
# Removing intermediate container faaae12b16f8
#  ---> eb8d814f7443
# Step 8/14 : ENV LC_ALL en_US.UTF-8
#  ---> Running in c8737036d28c
# Removing intermediate container c8737036d28c
#  ---> 7a1d40221f82
# Step 9/14 : RUN mkdir -p /root/.local/share
#  ---> Running in b80f707744df
# Removing intermediate container b80f707744df
#  ---> 49318139ad9a
# Step 10/14 : ARG deb_file=passwordsafe-ubuntu20-1.13-amd64.deb
#  ---> Running in e3a4a58fa6d6
# Removing intermediate container e3a4a58fa6d6
#  ---> b1d8383eb7df
# Step 11/14 : LABEL "pwsafe"="$deb_file"
#  ---> Running in 301153be3dc4
# Removing intermediate container 301153be3dc4
#  ---> c2644a37972d
# Step 12/14 : ADD https://github.com/pwsafe/pwsafe/releases/download/1.13.0/$deb_file
#  ---> 807bacbac588
# Step 13/14 : RUN dpkg -i /tmp/$deb_file && rm /tmp/$deb_file
#  ---> Running in b42e203f31aa
# Removing intermediate container b42e203f31aa
#  ---> a9435c693fb5
# Step 14/14 : CMD /usr/bin/pwsafe
#  ---> Running in e1dd74b99040
# Removing intermediate container e1dd74b99040
#  ---> 28d5f8ae80b9
# Successfully built 28d5f8ae80b9
# Successfully tagged passwordsafe:1.13
#
# real	7m49.988s
# user	0m0.448s
# sys	0m0.392s
# ----
# 
# The image is not small because of the large amount of stuff that gets 
# pulled in to meet the deps:
# ----
# ~/working/docker/passwordsafe$ sudo docker image list
# REPOSITORY           TAG       IMAGE ID       CREATED          SIZE
# passwordsafe         1.13      28d5f8ae80b9   11 minutes ago   1.05GB
# ----
