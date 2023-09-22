<!---
name: Create release
route: /docs/cpp/develop/testing-release-utbotcpp-in-docker.md
parent: Documentation
menu: Develop
description: Step by step instruction for create docker for testing UTBotCpp release image
--->

## Create docker container for tests

Create container
```bash
docker container create -i -t -p 5522:22 -p 5521:2121 --name utbot-test ubuntu:20.04 #5522 ssh port, 5521 utbot port
docker start utbot-test
```

Go into container 
```bash
docker exec -it utbot-test /bin/bash
```

Install dependency
```bash
apt update && apt install sudo vim openssh-server unzip gcc-9 g++-9 make -y
```

Create user
```bash
useradd -m utbot
usermod -aG sudo utbot
```

Set user password
```bash
passwd utbot
```

Set up ssh
```bash
service ssh start
```

Exit from docker
```bash
exit
```

## Setup UTBotCpp

Download archive from result of [github publish UTBot as an archive](https://github.com/UnitTestBot/UTBotCpp/actions/workflows/publish-utbot.yml)

Copy archive to docker
```bash
scp -P 5522 utbot-dev-2022.7.198.zip utbot@127.0.0.1:/home/utbot/
```

ssh into docker
```bash
ssh utbot@127.0.0.1 -p 5522
```

Setup UTBotCpp
```bash
chsh -s /bin/bash  #change sh to bash
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 60 --slave /usr/bin/g++ g++ /usr/bin/g++-9
sudo update-alternatives --config gcc
unzip utbot-dev-2022.7.198.zip
chmod +x unpack_and_run_utbot.sh
./unpack_and_run_utbot.sh
```

## Next same as [step-by-step](https://github.com/UnitTestBot/UTBotCpp/wiki/step-by-step)