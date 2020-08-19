ARG  from
FROM ${from}

WORKDIR /usr/src/Vega-Strike-Engine-Source

COPY script/ script/
ENV UPDATE=1
ENV DEBIAN_FRONTEND=noninteractive
RUN script/setup

COPY . .

ENTRYPOINT [ "script/docker-entrypoint.sh" ]
