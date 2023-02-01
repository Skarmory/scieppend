#include "scieppend/core/log.h"

#include <stdarg.h>
#include <stdio.h>

// CONSTS

#define C_LOG_CHANNELS_MAX 32

static const char* C_MSGHIST_FNAME = "msghist.log";
static const char* C_DEBUGLOG_FNAME = "debug.log";
static const char* C_TESTLOG_FNAME = "test.log";

// STRUCTS

struct _LogChannel
{
    FILE* file;
    int indent;
};

// VARS

static struct _LogChannel s_log_channels[C_LOG_CHANNELS_MAX];

// INTERNAL FUNCS

static void _log_msg(struct _LogChannel* channel, const char* msg)
{
    for(int i = 0; i < channel->indent; ++i)
    {
        fprintf(channel->file, "\t");
    }

    fprintf(channel->file, "%s\n", msg);
    fflush(channel->file);
}

// EXTERNAL FUNCS

void init_logs(void)
{
    s_log_channels[LOG_ID_MSGHIST].file = fopen(C_MSGHIST_FNAME, "w");
    s_log_channels[LOG_ID_MSGHIST].indent = 0;

    s_log_channels[LOG_ID_DEBUG].file = fopen(C_DEBUGLOG_FNAME, "w");
    s_log_channels[LOG_ID_DEBUG].indent = 0;

    s_log_channels[LOG_ID_TEST].file = fopen(C_TESTLOG_FNAME, "w");
    s_log_channels[LOG_ID_TEST].indent = 0;

    s_log_channels[LOG_ID_STDOUT].file = stdout;
    s_log_channels[LOG_ID_STDOUT].indent = 0;
}

void log_msg(LogChannels channels, const char* msg)
{
    if(bit_flags_has_flags(channels, LOG_MSGHIST))
    {
        _log_msg(&s_log_channels[LOG_ID_MSGHIST], msg);
    }

    if(bit_flags_has_flags(channels, LOG_DEBUG))
    {
        _log_msg(&s_log_channels[LOG_ID_DEBUG], msg);
    }

    if(bit_flags_has_flags(channels, LOG_TEST))
    {
        _log_msg(&s_log_channels[LOG_ID_TEST], msg);
    }

    if(bit_flags_has_flags(channels, LOG_STDOUT))
    {
        _log_msg(&s_log_channels[LOG_ID_STDOUT], msg);
    }
}

void log_format_msg(LogChannels channels, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    char msg[256];
    vsnprintf(msg, 256, format, args);
    log_msg(channels, msg);

    va_end(args);
}

void uninit_logs(void)
{
    fflush(s_log_channels[LOG_ID_MSGHIST].file);
    fclose(s_log_channels[LOG_ID_MSGHIST].file);

    fflush(s_log_channels[LOG_ID_DEBUG].file);
    fclose(s_log_channels[LOG_ID_DEBUG].file);

    fflush(s_log_channels[LOG_ID_TEST].file);
    fclose(s_log_channels[LOG_ID_TEST].file);
}

void log_push_indent(LogChannels channels)
{
    if(bit_flags_has_flags(channels, LOG_MSGHIST))
    {
        ++s_log_channels[LOG_ID_MSGHIST].indent;
    }

    if(bit_flags_has_flags(channels, LOG_DEBUG))
    {
        ++s_log_channels[LOG_ID_DEBUG].indent;
    }

    if(bit_flags_has_flags(channels, LOG_TEST))
    {
        ++s_log_channels[LOG_ID_TEST].indent;
    }

    if(bit_flags_has_flags(channels, LOG_STDOUT))
    {
        ++s_log_channels[LOG_ID_STDOUT].indent;
    }
}

void log_pop_indent(LogChannels channels)
{
    if(bit_flags_has_flags(channels, LOG_MSGHIST))
    {
        if(s_log_channels[LOG_ID_MSGHIST].indent > 0)
        {
            --s_log_channels[LOG_ID_MSGHIST].indent;
        }
    }

    if(bit_flags_has_flags(channels, LOG_DEBUG))
    {
        if(s_log_channels[LOG_ID_DEBUG].indent > 0)
        {
            --s_log_channels[LOG_ID_DEBUG].indent;
        }
    }

    if(bit_flags_has_flags(channels, LOG_TEST))
    {
        if(s_log_channels[LOG_ID_TEST].indent > 0)
        {
            --s_log_channels[LOG_ID_TEST].indent;
        }
    }

    if(bit_flags_has_flags(channels, LOG_STDOUT))
    {
        if(s_log_channels[LOG_ID_STDOUT].indent > 0)
        {
            --s_log_channels[LOG_ID_STDOUT].indent;
        }
    }
}
