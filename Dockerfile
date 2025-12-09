ARG  from
FROM ${from}

WORKDIR /usr/local/src/Vega-Strike-Engine-Source

COPY . .

ENTRYPOINT ["script/docker-entrypoint.sh"]
