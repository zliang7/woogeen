/*
 * Copyright © 2016 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#include "logredirector.h"

#if defined(LOGREDIR_SINK)

#undef LOG_V
#undef LOG_E
#include "rtc_logging.h"

class LogSink_Logd : public rtc::LogSink {
public:
    LogSink_Logd(int priority, const std::string& tag):
        priority_(priority), tag_(tag) {}

private:
    void OnLogMessage(const std::string& message) override {
        yunosLogPrint(kLogIdMain, priority_, tag_.c_str(),
                      "%s", message.c_str());
    }
    int priority_;
    std::string tag_;
};

class LogSink_File : public rtc::LogSink {
public:
    LogSink_File(const std::string& path) {
        fd_ = open(path.c_str(), O_CREAT|O_WRONLY, S_IRWXU);
        assert(fd_ >= 0);
    }
    ~LogSink_File() {
        close(fd_);
    }

private:
    void OnLogMessage(const std::string& message) override {
        write(fd_, message.c_str(), message.length());
    }
    int fd_;
};

static rtc::LoggingSeverity SeverityMap[kLogPrioritySilent + 1] = {
    rtc::LS_NONE,       // kLogPriorityUnknown
    rtc::LS_NONE,       // kLogPriorityDefault
    rtc::LS_VERBOSE,    // kLogPriorityVerbose
    rtc::LS_SENSITIVE,  // kLogPriorityDebug
    rtc::LS_INFO,       // kLogPriorityInfo
    rtc::LS_WARNING,    // kLogPriorityWarn
    rtc::LS_ERROR,      // kLogPriorityError
    rtc::LS_ERROR,      // kLogPriorityFatal
    rtc::LS_NONE        // kLogPrioritySilent
};

#elif defined(LOGREDIR_PIPE)

#include <thread>

void logging_thread_main(int fd, int priority, std::string tag) {
    char buf[4096];
    FILE* fp = fdopen(fd, "r");

    if (fp == NULL) return;
    while (fgets(buf, sizeof(buf), fp)) {
        yunosLogPrint(kLogIdMain, priority, tag.c_str(), "%s", buf);
    }
    fclose(fp);
}

#endif

void RedirectWoogeenLog(const std::string& path_or_tag, int priority) {
    if (path_or_tag.empty() ||
        priority > kLogPrioritySilent || priority < kLogPriorityVerbose)
        return;

#if defined(LOGREDIR_SINK)
    rtc::LogSink* sink;
    if (path_or_tag.front() == '/') {
        sink = new LogSink_File(path_or_tag);
    } else {
        sink = new LogSink_Logd(priority, path_or_tag);
    }
    rtc::LogMessage::AddLogToStream(sink, SeverityMap[priority]);
    rtc::LogMessage::SetLogToStderr(false);
#elif defined(LOGREDIR_PIPE)
    int fds[2], &rfd=fds[0], &wfd=fds[1];
    if (path_or_tag.front() == '/') {
        wfd = open(path_or_tag.c_str(), O_CREAT|O_WRONLY, S_IRWXU);
    } else if (!pipe(fds)) {
        std::thread thread(logging_thread_main, rfd, priority, path_or_tag);
        thread.detach();
    } else {
        wfd = -1;
    }

    if (wfd > -1) {
        close(STDERR_FILENO);
        dup2(wfd, STDERR_FILENO);
        close(wfd);
    }
#endif
}
