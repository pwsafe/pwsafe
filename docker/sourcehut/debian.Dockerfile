FROM debian

RUN true \
	&& apt-get update -q \
	&& apt-get -qy \
		busybox \
		lsb-release