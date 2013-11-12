#ifndef SLEEPER_THREAD_H
#define SLEEPER_THREAD_H
#include <QThread>

class SleeperThread : public QThread
{
public:
    static void msleep(unsigned long msecs)
    {
        QThread::msleep(msecs);
    }
};

#endif // #ifndef SLEEPER_THREAD_H

