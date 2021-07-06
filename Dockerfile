ARG  from
FROM ${from}

WORKDIR /usr/src/Vega-Strike-Engine-Source

COPY . .

ENTRYPOINT ["script/docker-entrypoint.sh"]
CMD [""]
