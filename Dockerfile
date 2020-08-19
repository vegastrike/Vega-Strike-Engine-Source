ARG  from
FROM ${from}

WORKDIR /usr/src/Vega-Strike-Engine-Source

COPY script/ script/

RUN DEBIAN_FRONTEND=noninteractive script/setup

COPY . .

ENTRYPOINT ["script/docker-entrypoint.sh"]
CMD ["-DUSE_PYTHON_3=OFF"]
