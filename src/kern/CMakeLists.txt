#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/data/romas/bmstu/term_project/07_operating_systems/bin")

ADD_DEFINITIONS(-g -O2 -fsigned-char -freg-struct-return -Wall -W -Wshadow -Wstrict-prototypes -Wpointer-arith -Wcast-qual -Winline -Werror)
EXECUTE_PROCESS(COMMAND uname -r
                OUTPUT_VARIABLE os_release
                OUTPUT_STRIP_TRAILING_WHITESPACE)

set(module_path /lib/modules/${os_release})
set(module_build_path ${module_path}/build)
set(module_source_dir ${PROJECT_SOURCE_DIR}/src/kern)
set(module_name saltfs.ko)
set(module_bin_dir ${PROJECT_SOURCE_DIR}/bin/)

ADD_CUSTOM_COMMAND(OUTPUT ${module_name}
                   COMMAND make -C ${module_build_path} M=`pwd` modules
                   COMMAND cp -f ${module_name} ${module_bin_dir}
                   WORKING_DIRECTORY ${module_source_dir}
                   DEPENDS super.c Kbuild
                   COMMENT "Building saltfs.ko"
                  )

ADD_CUSTOM_TARGET(saltfs ALL DEPENDS ${module_name})

# Installation of the module
#SET(module_install_path ${module_path}/kernel)
#INSTALL(FILES udpc.ko
#        DESTINATION ${module_install_path}
#        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)