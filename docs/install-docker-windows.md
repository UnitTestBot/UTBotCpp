# Install Docker in WSL

Get ready for creating a Docker container:

1. Make sure you have created a WSL container with Ubuntu 20.04:
```bash
wsl --list --verbose
```
2. Navigate to the container:
```bash
wsl
```
3. In this container, install Docker and create a Docker user:
```bash
sudo apt update
sudo apt install docker.io -y
sudo usermod -aG docker $User
```
3. Start the Docker daemon:
```bash
sudo dockerd
```
