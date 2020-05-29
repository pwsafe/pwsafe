FROM gitpod/workspace-full-vnc
                    
USER gitpod
# Set debconf to noninteractive mode.
RUN echo 'debconf debconf/frontend select Noninteractive' | sudo debconf-set-selections
# Install custom tools, runtime, etc. using apt-get
# For example, the command below would install "bastet" - a command line tetris clone:
#
# RUN sudo apt-get -q update && #     sudo apt-get install -yq bastet && #     sudo rm -rf /var/lib/apt/lists/*
#
# More information: https://www.gitpod.io/docs/config-docker/
USER root
# Install dependencies.
COPY /Misc/setup-deb-dev-env.sh /tmp/setup-deb-dev-env.sh
RUN true \
  && apt-get -q update \
  && sh /tmp/setup-deb-dev-env.sh \
  && apt-get autoremove -yq \
  && rm -rf /var/lib/apt/lists/*

# Set debconf back to normal.
USER gitpod
RUN echo 'debconf debconf/frontend select Dialog' | sudo debconf-set-selections
