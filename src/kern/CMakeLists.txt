ADD_DEFINITIONS(-g -O2 -fsigned-char -freg-struct-return -Wall -W -Wshadow -Wstrict-prototypes -Wpointer-arith -Wcast-qual -Winline -Werror)
EXECUTE_PROCESS(COMMAND uname -r
                OUTPUT_VARIABLE os_release
                OUTPUT_STRIP_TRAILING_WHITESPACE)

set(module_name saltfs.ko)
set(module_path /lib/modules/${os_release})
set(module_build_path ${module_path}/build)
set(module_source_dir ${CMAKE_CURRENT_LIST_DIR})
set(module_bin_dir ${PROJECT_SOURCE_DIR}/bin/)

ADD_CUSTOM_COMMAND(OUTPUT ${module_name}
                   COMMAND make -C ${module_build_path} M=`pwd` modules
                   COMMAND cp -f ${module_name} ${module_bin_dir}/${module_name}
                   WORKING_DIRECTORY ${module_source_dir}
                   DEPENDS super.c Kbuild
                   COMMENT "Building saltfs.ko"
                  )

ADD_CUSTOM_TARGET(saltfs ALL DEPENDS ${module_name})
