add_definitions(-g -O2 -fsigned-char -freg-struct-return -Wall -W -Wshadow -Wstrict-prototypes -Wpointer-arith -Wcast-qual -Winline -Werror)
execute_process(COMMAND uname -r
                OUTPUT_VARIABLE os_release
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND sh -c "uname -r | sed 's/-\\w*$//'"
                OUTPUT_VARIABLE os_release_without_type
                OUTPUT_STRIP_TRAILING_WHITESPACE)

include_directories(/usr/src/linux-3.11.10-21/include)
#include_directories(/usr/src/${os_release_without_type}/include/)
set(module_name saltfs.ko)
set(module_path /lib/modules/${os_release})
set(module_build_path ${module_path}/build)
set(module_source_dir ${CMAKE_CURRENT_LIST_DIR})
set(module_headers internal.h)
set(module_bin ${module_source_dir}/${module_name})

add_custom_command(OUTPUT ${module_bin}
                   COMMAND make -C ${module_build_path} M=`pwd` modules
                   WORKING_DIRECTORY ${module_source_dir}
                   DEPENDS super.c ${module_headers} Kbuild
                   COMMENT "Building saltfs.ko"
                  )

add_custom_target(saltfs ALL DEPENDS ${module_bin})
