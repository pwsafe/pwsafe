FROM gitpod/workspace-full-vnc

USER root

# Get dependencies
COPY Misc/name-me.sh /usr/bin/name-me.sh
RUN true "replace" \
	&& apt-get install -qy busybox lsb-release \
	&& chmod +x /usr/bin/name-me.sh \
	&& busybox sh /usr/bin/name-me.sh || printf '\033[31m\033[1mBUG:\033[0m %s\n' "Script Misc/'name-me.sh' failed, check logs at $HOME/.name-me.log"

USER gitpod