FROM gitpod/workspace-full-vnc
                    
USER gitpod

# Install custom tools, runtime, etc. using apt-get
# For example, the command below would install "bastet" - a command line tetris clone:
#
# RUN sudo apt-get -q update && #     sudo apt-get install -yq bastet && #     sudo rm -rf /var/lib/apt/lists/*
#
# More information: https://www.gitpod.io/docs/config-docker/
RUN sudo apt-get -q update && sudo apt-get install -yq cmake fakeroot g++ gettext git libgtest-dev \
        libcurl4-openssl-dev libqrencode-dev  libssl-dev libuuid1 \
        libwxgtk3.0-gtk3-dev libxerces-c-dev libxt-dev libxtst-dev \
        libykpers-1-dev libyubikey-dev make pkg-config uuid-dev zip \
        libmagic-dev
# Set debconf back to normal.
RUN echo 'debconf debconf/frontend select Dialog' | sudo debconf-set-selections
