FROM gitpod/workspace-full-vnc
                    
USER gitpod
# Set debconf to noninteractive mode.
RUN echo 'debconf debconf/frontend select Noninteractive' | sudo debconf-set-selections
# Install custom tools, runtime, etc. using apt-get
#
# More information: https://www.gitpod.io/docs/config-docker/
USER root
# Install dependencies.
COPY /Misc/setup-linux-dev-env.sh /tmp/setup-linux-dev-env.sh
RUN true \
  && apt-get -q update \
  && sh /tmp/setup-linux-dev-env.sh \
  && apt-get autoremove -yq

#  RUN rm -rf /var/lib/apt/lists/*

# Set debconf back to normal.
USER gitpod
RUN echo 'debconf debconf/frontend select Dialog' | sudo debconf-set-selections