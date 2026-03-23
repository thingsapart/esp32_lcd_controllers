#ifndef LCD_CTRL_PERF_TRACE_H
#define LCD_CTRL_PERF_TRACE_H

/**
 * @file perf_trace.h
 * @brief No-op performance tracing stubs.
 *
 * The full implementation in cnc_interface emits per-phase timing data via
 * the serial port.  This stub version silently compiles away all PERF_*
 * macros so the driver files that include perf_trace.h compile cleanly
 * without the full tracing infrastructure.
 *
 * To enable real tracing, define PERF_TRACE_ENABLED=1 in your build_flags
 * and provide your own perf_trace.h before this file is included.
 */

#define PERF_BEGIN(name)                    do {} while (0)
#define PERF_END(tag, name)                 do {} while (0)
#define PERF_END_D(tag, name)               do {} while (0)
#define PERF_SLOWLOG(tag, name, thresh_us)  do {} while (0)

#define TRACE_TASK_WAKE(tag)                do {} while (0)
#define TRACE_TASK_SLEEP(tag, ms)           do {} while (0)

#endif /* LCD_CTRL_PERF_TRACE_H */
