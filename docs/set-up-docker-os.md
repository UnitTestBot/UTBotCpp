# Set up Docker and run a container with Ubuntu 18.04

Install Docker using the instructions for your OS:

* [Linux](https://docs.docker.com/engine/install/) (external link)
* [Windows](install-docker-windows)
* [macOS](https://pilsniak.com/how-to-install-docker-on-mac-os-using-brew) (external link)

Run a Docker container with Ubuntu 18.04:

1. Create a container:
```bash
docker container create -i -t -p 5522:22 -p 5521:2121 --name utbot ubuntu:18.04
docker start utbot
```
2. Navigate to a container:
```bash
docker exec -it utbot /bin/bash
```
3. Create a user:
```bash
useradd -m utbot
usermod -aG sudo utbot
```
4. Set the user password:
```bash
passwd utbot
```
5. Install the OpenSSH server:
```bash
apt update && apt install  openssh-server sudo -y
``` 
6. Set up an SSH connection:
```bash
service ssh start
```
