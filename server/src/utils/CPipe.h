#ifndef UNITTESTBOT_CPIPE_H
#define UNITTESTBOT_CPIPE_H


class CPipe {
private:
    int fd[2]{};

public:
    CPipe();

    ~CPipe() = default;

    int readFd() const;

    int writeFd() const;

    void close();
};


#endif // UNITTESTBOT_CPIPE_H
