#!/usr/bin/env bash

# This script is used to pull base docker image where klee and UTBot can be built.

CONTAINER_NAME=$USER-utbot-dev
MOUNT_NAME=$USER-utbot

read -e -p "Enter base image tag: " IMAGE_TAG
IMAGE="ghcr.io/unittestbot/utbotcpp/base_env:$IMAGE_TAG"

echo "Pulling docker image '$IMAGE'"
if ! docker pull $IMAGE ; then
  echo "Failed to fetch the image. Aborting.."
  exit 1
fi
set +e
docker exec "$CONTAINER_NAME" ls > /dev/null 2>&1
if [ $? -eq 0 ]
then
  UTBOT_SSH_PORT=`(docker port $CONTAINER_NAME | egrep '2020/tcp ->' | tail -c 5)`
  UTBOT_SERVER_PORT=`(docker port $CONTAINER_NAME | egrep '2121/tcp ->' | tail -c 5)`
  echo "Found running '$CONTAINER_NAME' container. Reusing its ssh ($UTBOT_SSH_PORT) and utbot ($UTBOT_SERVER_PORT) port mapping"
  docker stop $CONTAINER_NAME > /dev/null && docker rm $CONTAINER_NAME > /dev/null
else
  docker rm $CONTAINER_NAME > /dev/null 2>&1
  # Get ssh port
  read -e -p "Enter ssh port to listen: " -i "$UTBOT_SSH_PORT" UTBOT_SSH_PORT
  # Get UTBot server port
  read -e -p "Enter UTBot server port: " -i "$UTBOT_SERVER_PORT" UTBOT_SERVER_PORT
fi
set -e

echo "Recreating docker container..."
docker run -d --restart=unless-stopped \
 --cap-add=SYS_PTRACE --security-opt seccomp=unconfined --security-opt apparmor=unconfined \
 --name=$CONTAINER_NAME \
 -p $UTBOT_SSH_PORT:2020 \
 -p $UTBOT_SERVER_PORT:2121 \
 -v /etc/localtime:/etc/localtime:ro \
 $IMAGE > /dev/null
echo "Container '$CONTAINER_NAME' is up and running."
echo "UTBOT_SSH_PORT=$UTBOT_SSH_PORT"
echo "UTBOT_SERVER_PORT=$UTBOT_SERVER_PORT"
