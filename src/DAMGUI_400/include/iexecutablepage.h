#ifndef IEXECUTABLEPAGE_H
#define IEXECUTABLEPAGE_H

class IExecutablePage
{
public:
    virtual ~IExecutablePage() = default;

    virtual void setPageEnabled(bool enabled) = 0;
    virtual void setExecEnabled(bool enabled) = 0;
    virtual void setStopEnabled(bool enabled) = 0;
};

#endif
