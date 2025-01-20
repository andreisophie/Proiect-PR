#!/bin/bash

docker swarm init
docker build . -t scd3_adapter
docker stack deploy -c stack.yml scd3