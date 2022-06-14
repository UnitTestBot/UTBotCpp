#ifndef UNITTESTBOT_PROGRESSWRITER_H
#define UNITTESTBOT_PROGRESSWRITER_H

#include "IStreamWriter.h"

class ProgressWriter : public virtual IStreamWriter {
public:
    /**
     * @brief writes progress.
     * @param message message to show next to progress.
     * @param percent specifies how much of the task that has been completed.  It must be a valid
     * floating point number between 0 an 100.
     */
    virtual void writeProgress(const std::optional<std::string> &message,
                               double percent = 0.0,
                               bool completed = false) const = 0;
};


#endif // UNITTESTBOT_PROGRESSWRITER_H
