AC_DEFUN([VS_WITH_NET_THREADS], 
[dnl
#--------------------------------------------------------------------------
#Choosing networking threading approach
#--------------------------------------------------------------------------
AC_ARG_WITH(net-threads, AC_HELP_STRING([[--with-net-threads[=TYPE]]], [TYPES are none (default) and posix]))
case "$with_net_threads" in
    "p" | "posix" | "yes" | "y")
        AC_MSG_NOTICE([Using Posix Threads (Experimental)])
        AC_DEFINE(USE_NET_THREAD_POSIX,1,[Use POSIX threads])
    ;;
    "n" | "no" | "none"| *)
        AC_MSG_NOTICE([No Threading Model Used])
        AC_DEFINE(USE_NET_THREAD_NONE,1,[Use no threads])
    ;;
esac])
