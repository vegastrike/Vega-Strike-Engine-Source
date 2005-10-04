AC_DEFUN([VS_WITH_COLLIDE_MAP], 
[dnl
#--------------------------------------------------------------------------
#Choosing list or set based collision testing
#--------------------------------------------------------------------------
AC_ARG_WITH(collision-map, AC_HELP_STRING([--with-collision-map], [TYPES are list (default) and set]))
case "$with_collision_map" in
    "set")
        AC_DEFINE([VS_ENABLE_COLLIDE_KEY], , [Enables use of std::set for collision testing])
        AC_MSG_NOTICE([Using std::set for basis of collision testing (experimental)])
    ;;
    "list" | *)
        AC_MSG_NOTICE([Using std::list for basis of collision testing])
    ;;
esac])
