// https://www.boost.org/doc/libs/1_65_1/doc/html/stacktrace/getting_started.html#stacktrace.getting_started.handle_terminates_aborts_and_seg

#include "app/stacktrace.h"

#include <boost/stacktrace.hpp>

#include <sstream>

#include <signal.h>

#include <QFile>
#include <QtGlobal>

#ifdef Q_OS_WIN
#   include <qt_windows.h>
#endif

namespace {

QByteArray backtracePath()
{
    return qgetenv("COPYQ_BACKTRACE");
}

void dumpStackTrace()
{
    const auto path = backtracePath();
    if ( !path.isEmpty() )
        boost::stacktrace::safe_dump_to(path.data());
}

#ifdef Q_OS_WIN

long WINAPI unhandledExceptionFilter(EXCEPTION_POINTERS *)
{
    dumpStackTrace();
    return EXCEPTION_CONTINUE_SEARCH;
}

void registerUnhandledExceptionFilter()
{
    SetUnhandledExceptionFilter(unhandledExceptionFilter);
}

#else // Q_OS_WIN

void registerUnhandledExceptionFilter()
{
}

#endif // Q_OS_WIN

void crashHandler(int signum)
{
    ::signal(signum, SIG_DFL);
    dumpStackTrace();
    ::raise(SIGABRT);
}

} // namespace

void registerCrashHandler()
{
    ::signal(SIGSEGV, &crashHandler);
    ::signal(SIGABRT, &crashHandler);
    registerUnhandledExceptionFilter();
}

int printLastStackTrace()
{
    const auto bt = lastStackTrace();
    if ( bt.isEmpty() )
        return 0;

    QFile f;
    f.open(stderr, QIODevice::WriteOnly);
    f.write("ERROR: Found stacktrace of last crash:\n");
    f.write(bt);
    f.write("\n");
    f.close();

    return 1;
}

QByteArray lastStackTrace()
{
    const auto path = backtracePath();
    if ( path.isEmpty() )
        return QByteArray();

    QFile backtrace(path);
    if ( !backtrace.exists() )
        return QByteArray();

    if ( !backtrace.open(QIODevice::ReadOnly) )
        return QByteArray();

    const QByteArray data = backtrace.readAll();

    const auto bt = boost::stacktrace::stacktrace::from_dump(data.data(), data.size() * sizeof(char));

    std::stringstream stream;
    stream << bt;
    const auto text = stream.str();

    backtrace.remove();

    return QByteArray(text.data(), text.size());
}
