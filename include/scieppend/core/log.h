#ifndef SCIEPPEND_CORE_LOG_H
#define SCIEPPEND_CORE_LOG_H

#include "scieppend/core/bit_flags.h"

enum LogChannelID
{
    LOG_ID_MSGHIST = 0,
    LOG_ID_DEBUG,
    LOG_ID_TEST,
    LOG_ID_STDOUT
};

enum LogChannel
{
    LOG_MSGHIST = BIT_FLAG(0),
    LOG_DEBUG   = BIT_FLAG(1),
    LOG_TEST    = BIT_FLAG(2),
    LOG_STDOUT  = BIT_FLAG(3)
};
typedef unsigned int LogChannels;

/* Opens and overwrites all the log files, ready for use.
 * This should be called only once at startup.
 */
void init_logs(void);

/* Flushes and closes all log files.
 * This should be called only once at shutdown.
 */
void uninit_logs(void);

/* Writes a message to all specified log channels.
 */
void log_msg(LogChannels channels, const char* msg);

/* Writes a format string to all specified log channels.
 */
void log_format_msg(LogChannels channels, const char* format, ...);

/* Adds a level of tab indent to all specified log channels.
 */
void log_push_indent(LogChannels channels);

/* Removes a level of tab indent to all specified log channels, minimum 0.
 */
void log_pop_indent(LogChannels channels);

#endif
