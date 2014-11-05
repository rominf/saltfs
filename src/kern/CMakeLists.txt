ADD_DEFINITIONS(-g -O2 -fsigned-char -freg-struct-return -Wall -W -Wshadow -Wstrict-prototypes -Wpointer-arith -Wcast-qual -Winline -Werror)
EXECUTE_PROCESS(COMMAND uname -r
                OUTPUT_VARIABLE os_release
                OUTPUT_STRIP_TRAILING_WHITESPACE)

set(module_name saltfs.ko)
set(module_path /lib/modules/${os_release})
set(module_build_path ${module_path}/build)
set(module_source_dir ${CMAKE_CURRENT_LIST_DIR})
set(module_headers defines.h super.h)
set(module_bin ${module_source_dir}/${module_name})

ADD_CUSTOM_COMMAND(OUTPUT ${module_bin}
                   COMMAND make -C ${module_build_path} M=`pwd` modules
                   WORKING_DIRECTORY ${module_source_dir}
                   DEPENDS super.c ${module_headers} Kbuild
                   COMMENT "Building saltfs.ko"
                  )

ADD_CUSTOM_TARGET(saltfs ALL DEPENDS ${module_bin})
