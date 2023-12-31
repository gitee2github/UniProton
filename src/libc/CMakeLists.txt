if(NOT ${CONFIG_OS_OPTION_POSIX})
    RETURN()
endif()

message(${CMAKE_SYSTEM_PROCESSOR})

# 所有litelibc中适配的.c文件
file(GLOB_RECURSE LITELIBC_SOURCE litelibc/src/*.c litelibc/internal/*.c)

# 需要编译的musl的.c文件
set(MULS_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/unistd/sleep.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/locale/c_locale.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/locale/__lctrans.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/locale/langinfo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/internal/libc.c
    )

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/musl/arch/${CMAKE_SYSTEM_PROCESSOR}/arch.mak)
list(APPEND  MULS_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/clock_getres_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/clock_gettime32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/clock_nanosleep_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/clock_settime32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/ctime32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/difftime32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/ftime32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/gettimeofday_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/gmtime32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/localtime32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/mktime32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/nanosleep_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/pthread_mutex_timedlock_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/pthread_rwlock_timedrdlock_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/pthread_rwlock_timedwrlock_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/timer_gettime32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/timer_settime32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/timespec_get_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/utime_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/sem_timedwait_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/ctime32_r.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/localtime32_r.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/gmtime32_r.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/thrd_sleep_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/pthread_timedjoin_np_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/pthread_cond_timedwait_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/cnd_timedwait_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/mtx_timedlock_time32.c
)
endif()

file(GLOB_RECURSE MUSL_TIME_SOURCE musl/src/time/*.c)
file(GLOB MUSL_THREAD_SOURCE musl/src/thread/*.c)

# 当前不支持功能，不提供该接口
list(REMOVE_ITEM MUSL_TIME_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/time/__map_file.c
    # 当前不支持文件系统
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/time/utime.c
    # 当前不支持文件系统、环境变量获取
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/time/getdate.c
)
list(REMOVE_ITEM MUSL_THREAD_SOURCE
    # 当前不支持syscall、tls、vmclock
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/clone.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/synccall.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/syscall_cp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/tls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/vmlock.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/__set_thread_area.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/__syscall_cp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/__timedwait.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/__tls_get_addr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/__unmapself.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/__wait.c
    )
list(APPEND MULS_SOURCE
    ${MUSL_TIME_SOURCE}
    ${MUSL_THREAD_SOURCE})

add_library(musllibc OBJECT ${MULS_SOURCE} ${LITELIBC_SOURCE})
target_compile_options(musllibc PUBLIC 
        -Wno-bad-function-cast
        -Wno-unused-value
        -Wno-switch-default
        -Wno-float-equal
        -Wno-unused-but-set-variable
        -Wno-discarded-qualifiers
        -Wno-implicit-fallthrough
        -Wno-cast-function-type
        )
if (${OS_POSIX_SET_TZDST})
    target_compile_options(musllibc PUBLIC -DPOSIX_SET_TZDST)
endif()

target_include_directories(musllibc PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/internal/include
        ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/include
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/include
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/arch/${CMAKE_SYSTEM_PROCESSOR}
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/arch/generic
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/internal
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS musllibc)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

file(GLOB_RECURSE LITELIBC_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/include/*.h)
STRING(REPLACE 
       "${CMAKE_CURRENT_SOURCE_DIR}/litelibc/"
       "${CMAKE_CURRENT_SOURCE_DIR}/musl/" 
       DEL_INCLUDE
       "${LITELIBC_INCLUDE}")

add_custom_target(del_include_file ALL)
add_custom_command(TARGET del_include_file
                   POST_BUILD
                   COMMAND rm -rf ${DEL_INCLUDE}
                   DEPENDS ${DEL_INCLUDE}
)

install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/include
    ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/include
    DESTINATION libc/
)