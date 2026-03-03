# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(icu_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(icu_FRAMEWORKS_FOUND_RELEASE "${icu_FRAMEWORKS_RELEASE}" "${icu_FRAMEWORK_DIRS_RELEASE}")

set(icu_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET icu_DEPS_TARGET)
    add_library(icu_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET icu_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${icu_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${icu_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:ICU::data;ICU::uc;ICU::i18n;ICU::tu>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### icu_DEPS_TARGET to all of them
conan_package_library_targets("${icu_LIBS_RELEASE}"    # libraries
                              "${icu_LIB_DIRS_RELEASE}" # package_libdir
                              "${icu_BIN_DIRS_RELEASE}" # package_bindir
                              "${icu_LIBRARY_TYPE_RELEASE}"
                              "${icu_IS_HOST_WINDOWS_RELEASE}"
                              icu_DEPS_TARGET
                              icu_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "icu"    # package_name
                              "${icu_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${icu_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Release ########################################

    ########## COMPONENT ICU::test #############

        set(icu_ICU_test_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(icu_ICU_test_FRAMEWORKS_FOUND_RELEASE "${icu_ICU_test_FRAMEWORKS_RELEASE}" "${icu_ICU_test_FRAMEWORK_DIRS_RELEASE}")

        set(icu_ICU_test_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET icu_ICU_test_DEPS_TARGET)
            add_library(icu_ICU_test_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET icu_ICU_test_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${icu_ICU_test_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_test_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_test_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'icu_ICU_test_DEPS_TARGET' to all of them
        conan_package_library_targets("${icu_ICU_test_LIBS_RELEASE}"
                              "${icu_ICU_test_LIB_DIRS_RELEASE}"
                              "${icu_ICU_test_BIN_DIRS_RELEASE}" # package_bindir
                              "${icu_ICU_test_LIBRARY_TYPE_RELEASE}"
                              "${icu_ICU_test_IS_HOST_WINDOWS_RELEASE}"
                              icu_ICU_test_DEPS_TARGET
                              icu_ICU_test_LIBRARIES_TARGETS
                              "_RELEASE"
                              "icu_ICU_test"
                              "${icu_ICU_test_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET ICU::test
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${icu_ICU_test_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_test_LIBRARIES_TARGETS}>
                     )

        if("${icu_ICU_test_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET ICU::test
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         icu_ICU_test_DEPS_TARGET)
        endif()

        set_property(TARGET ICU::test APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${icu_ICU_test_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET ICU::test APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${icu_ICU_test_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET ICU::test APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${icu_ICU_test_LIB_DIRS_RELEASE}>)
        set_property(TARGET ICU::test APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${icu_ICU_test_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET ICU::test APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${icu_ICU_test_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT ICU::tu #############

        set(icu_ICU_tu_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(icu_ICU_tu_FRAMEWORKS_FOUND_RELEASE "${icu_ICU_tu_FRAMEWORKS_RELEASE}" "${icu_ICU_tu_FRAMEWORK_DIRS_RELEASE}")

        set(icu_ICU_tu_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET icu_ICU_tu_DEPS_TARGET)
            add_library(icu_ICU_tu_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET icu_ICU_tu_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${icu_ICU_tu_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_tu_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_tu_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'icu_ICU_tu_DEPS_TARGET' to all of them
        conan_package_library_targets("${icu_ICU_tu_LIBS_RELEASE}"
                              "${icu_ICU_tu_LIB_DIRS_RELEASE}"
                              "${icu_ICU_tu_BIN_DIRS_RELEASE}" # package_bindir
                              "${icu_ICU_tu_LIBRARY_TYPE_RELEASE}"
                              "${icu_ICU_tu_IS_HOST_WINDOWS_RELEASE}"
                              icu_ICU_tu_DEPS_TARGET
                              icu_ICU_tu_LIBRARIES_TARGETS
                              "_RELEASE"
                              "icu_ICU_tu"
                              "${icu_ICU_tu_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET ICU::tu
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${icu_ICU_tu_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_tu_LIBRARIES_TARGETS}>
                     )

        if("${icu_ICU_tu_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET ICU::tu
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         icu_ICU_tu_DEPS_TARGET)
        endif()

        set_property(TARGET ICU::tu APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${icu_ICU_tu_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET ICU::tu APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${icu_ICU_tu_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET ICU::tu APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${icu_ICU_tu_LIB_DIRS_RELEASE}>)
        set_property(TARGET ICU::tu APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${icu_ICU_tu_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET ICU::tu APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${icu_ICU_tu_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT ICU::io #############

        set(icu_ICU_io_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(icu_ICU_io_FRAMEWORKS_FOUND_RELEASE "${icu_ICU_io_FRAMEWORKS_RELEASE}" "${icu_ICU_io_FRAMEWORK_DIRS_RELEASE}")

        set(icu_ICU_io_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET icu_ICU_io_DEPS_TARGET)
            add_library(icu_ICU_io_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET icu_ICU_io_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${icu_ICU_io_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_io_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_io_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'icu_ICU_io_DEPS_TARGET' to all of them
        conan_package_library_targets("${icu_ICU_io_LIBS_RELEASE}"
                              "${icu_ICU_io_LIB_DIRS_RELEASE}"
                              "${icu_ICU_io_BIN_DIRS_RELEASE}" # package_bindir
                              "${icu_ICU_io_LIBRARY_TYPE_RELEASE}"
                              "${icu_ICU_io_IS_HOST_WINDOWS_RELEASE}"
                              icu_ICU_io_DEPS_TARGET
                              icu_ICU_io_LIBRARIES_TARGETS
                              "_RELEASE"
                              "icu_ICU_io"
                              "${icu_ICU_io_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET ICU::io
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${icu_ICU_io_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_io_LIBRARIES_TARGETS}>
                     )

        if("${icu_ICU_io_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET ICU::io
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         icu_ICU_io_DEPS_TARGET)
        endif()

        set_property(TARGET ICU::io APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${icu_ICU_io_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET ICU::io APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${icu_ICU_io_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET ICU::io APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${icu_ICU_io_LIB_DIRS_RELEASE}>)
        set_property(TARGET ICU::io APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${icu_ICU_io_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET ICU::io APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${icu_ICU_io_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT ICU::in #############

        set(icu_ICU_in_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(icu_ICU_in_FRAMEWORKS_FOUND_RELEASE "${icu_ICU_in_FRAMEWORKS_RELEASE}" "${icu_ICU_in_FRAMEWORK_DIRS_RELEASE}")

        set(icu_ICU_in_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET icu_ICU_in_DEPS_TARGET)
            add_library(icu_ICU_in_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET icu_ICU_in_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${icu_ICU_in_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_in_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_in_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'icu_ICU_in_DEPS_TARGET' to all of them
        conan_package_library_targets("${icu_ICU_in_LIBS_RELEASE}"
                              "${icu_ICU_in_LIB_DIRS_RELEASE}"
                              "${icu_ICU_in_BIN_DIRS_RELEASE}" # package_bindir
                              "${icu_ICU_in_LIBRARY_TYPE_RELEASE}"
                              "${icu_ICU_in_IS_HOST_WINDOWS_RELEASE}"
                              icu_ICU_in_DEPS_TARGET
                              icu_ICU_in_LIBRARIES_TARGETS
                              "_RELEASE"
                              "icu_ICU_in"
                              "${icu_ICU_in_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET ICU::in
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${icu_ICU_in_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_in_LIBRARIES_TARGETS}>
                     )

        if("${icu_ICU_in_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET ICU::in
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         icu_ICU_in_DEPS_TARGET)
        endif()

        set_property(TARGET ICU::in APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${icu_ICU_in_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET ICU::in APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${icu_ICU_in_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET ICU::in APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${icu_ICU_in_LIB_DIRS_RELEASE}>)
        set_property(TARGET ICU::in APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${icu_ICU_in_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET ICU::in APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${icu_ICU_in_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT ICU::i18n #############

        set(icu_ICU_i18n_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(icu_ICU_i18n_FRAMEWORKS_FOUND_RELEASE "${icu_ICU_i18n_FRAMEWORKS_RELEASE}" "${icu_ICU_i18n_FRAMEWORK_DIRS_RELEASE}")

        set(icu_ICU_i18n_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET icu_ICU_i18n_DEPS_TARGET)
            add_library(icu_ICU_i18n_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET icu_ICU_i18n_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${icu_ICU_i18n_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_i18n_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_i18n_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'icu_ICU_i18n_DEPS_TARGET' to all of them
        conan_package_library_targets("${icu_ICU_i18n_LIBS_RELEASE}"
                              "${icu_ICU_i18n_LIB_DIRS_RELEASE}"
                              "${icu_ICU_i18n_BIN_DIRS_RELEASE}" # package_bindir
                              "${icu_ICU_i18n_LIBRARY_TYPE_RELEASE}"
                              "${icu_ICU_i18n_IS_HOST_WINDOWS_RELEASE}"
                              icu_ICU_i18n_DEPS_TARGET
                              icu_ICU_i18n_LIBRARIES_TARGETS
                              "_RELEASE"
                              "icu_ICU_i18n"
                              "${icu_ICU_i18n_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET ICU::i18n
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${icu_ICU_i18n_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_i18n_LIBRARIES_TARGETS}>
                     )

        if("${icu_ICU_i18n_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET ICU::i18n
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         icu_ICU_i18n_DEPS_TARGET)
        endif()

        set_property(TARGET ICU::i18n APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${icu_ICU_i18n_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET ICU::i18n APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${icu_ICU_i18n_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET ICU::i18n APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${icu_ICU_i18n_LIB_DIRS_RELEASE}>)
        set_property(TARGET ICU::i18n APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${icu_ICU_i18n_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET ICU::i18n APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${icu_ICU_i18n_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT ICU::uc #############

        set(icu_ICU_uc_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(icu_ICU_uc_FRAMEWORKS_FOUND_RELEASE "${icu_ICU_uc_FRAMEWORKS_RELEASE}" "${icu_ICU_uc_FRAMEWORK_DIRS_RELEASE}")

        set(icu_ICU_uc_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET icu_ICU_uc_DEPS_TARGET)
            add_library(icu_ICU_uc_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET icu_ICU_uc_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${icu_ICU_uc_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_uc_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_uc_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'icu_ICU_uc_DEPS_TARGET' to all of them
        conan_package_library_targets("${icu_ICU_uc_LIBS_RELEASE}"
                              "${icu_ICU_uc_LIB_DIRS_RELEASE}"
                              "${icu_ICU_uc_BIN_DIRS_RELEASE}" # package_bindir
                              "${icu_ICU_uc_LIBRARY_TYPE_RELEASE}"
                              "${icu_ICU_uc_IS_HOST_WINDOWS_RELEASE}"
                              icu_ICU_uc_DEPS_TARGET
                              icu_ICU_uc_LIBRARIES_TARGETS
                              "_RELEASE"
                              "icu_ICU_uc"
                              "${icu_ICU_uc_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET ICU::uc
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${icu_ICU_uc_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_uc_LIBRARIES_TARGETS}>
                     )

        if("${icu_ICU_uc_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET ICU::uc
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         icu_ICU_uc_DEPS_TARGET)
        endif()

        set_property(TARGET ICU::uc APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${icu_ICU_uc_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET ICU::uc APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${icu_ICU_uc_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET ICU::uc APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${icu_ICU_uc_LIB_DIRS_RELEASE}>)
        set_property(TARGET ICU::uc APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${icu_ICU_uc_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET ICU::uc APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${icu_ICU_uc_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT ICU::dt #############

        set(icu_ICU_dt_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(icu_ICU_dt_FRAMEWORKS_FOUND_RELEASE "${icu_ICU_dt_FRAMEWORKS_RELEASE}" "${icu_ICU_dt_FRAMEWORK_DIRS_RELEASE}")

        set(icu_ICU_dt_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET icu_ICU_dt_DEPS_TARGET)
            add_library(icu_ICU_dt_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET icu_ICU_dt_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${icu_ICU_dt_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_dt_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_dt_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'icu_ICU_dt_DEPS_TARGET' to all of them
        conan_package_library_targets("${icu_ICU_dt_LIBS_RELEASE}"
                              "${icu_ICU_dt_LIB_DIRS_RELEASE}"
                              "${icu_ICU_dt_BIN_DIRS_RELEASE}" # package_bindir
                              "${icu_ICU_dt_LIBRARY_TYPE_RELEASE}"
                              "${icu_ICU_dt_IS_HOST_WINDOWS_RELEASE}"
                              icu_ICU_dt_DEPS_TARGET
                              icu_ICU_dt_LIBRARIES_TARGETS
                              "_RELEASE"
                              "icu_ICU_dt"
                              "${icu_ICU_dt_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET ICU::dt
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${icu_ICU_dt_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_dt_LIBRARIES_TARGETS}>
                     )

        if("${icu_ICU_dt_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET ICU::dt
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         icu_ICU_dt_DEPS_TARGET)
        endif()

        set_property(TARGET ICU::dt APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${icu_ICU_dt_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET ICU::dt APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${icu_ICU_dt_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET ICU::dt APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${icu_ICU_dt_LIB_DIRS_RELEASE}>)
        set_property(TARGET ICU::dt APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${icu_ICU_dt_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET ICU::dt APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${icu_ICU_dt_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT ICU::data #############

        set(icu_ICU_data_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(icu_ICU_data_FRAMEWORKS_FOUND_RELEASE "${icu_ICU_data_FRAMEWORKS_RELEASE}" "${icu_ICU_data_FRAMEWORK_DIRS_RELEASE}")

        set(icu_ICU_data_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET icu_ICU_data_DEPS_TARGET)
            add_library(icu_ICU_data_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET icu_ICU_data_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${icu_ICU_data_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_data_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_data_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'icu_ICU_data_DEPS_TARGET' to all of them
        conan_package_library_targets("${icu_ICU_data_LIBS_RELEASE}"
                              "${icu_ICU_data_LIB_DIRS_RELEASE}"
                              "${icu_ICU_data_BIN_DIRS_RELEASE}" # package_bindir
                              "${icu_ICU_data_LIBRARY_TYPE_RELEASE}"
                              "${icu_ICU_data_IS_HOST_WINDOWS_RELEASE}"
                              icu_ICU_data_DEPS_TARGET
                              icu_ICU_data_LIBRARIES_TARGETS
                              "_RELEASE"
                              "icu_ICU_data"
                              "${icu_ICU_data_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET ICU::data
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${icu_ICU_data_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${icu_ICU_data_LIBRARIES_TARGETS}>
                     )

        if("${icu_ICU_data_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET ICU::data
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         icu_ICU_data_DEPS_TARGET)
        endif()

        set_property(TARGET ICU::data APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${icu_ICU_data_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET ICU::data APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${icu_ICU_data_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET ICU::data APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${icu_ICU_data_LIB_DIRS_RELEASE}>)
        set_property(TARGET ICU::data APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${icu_ICU_data_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET ICU::data APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${icu_ICU_data_COMPILE_OPTIONS_RELEASE}>)


    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET icu::icu APPEND PROPERTY INTERFACE_LINK_LIBRARIES ICU::test)
    set_property(TARGET icu::icu APPEND PROPERTY INTERFACE_LINK_LIBRARIES ICU::tu)
    set_property(TARGET icu::icu APPEND PROPERTY INTERFACE_LINK_LIBRARIES ICU::io)
    set_property(TARGET icu::icu APPEND PROPERTY INTERFACE_LINK_LIBRARIES ICU::in)
    set_property(TARGET icu::icu APPEND PROPERTY INTERFACE_LINK_LIBRARIES ICU::i18n)
    set_property(TARGET icu::icu APPEND PROPERTY INTERFACE_LINK_LIBRARIES ICU::uc)
    set_property(TARGET icu::icu APPEND PROPERTY INTERFACE_LINK_LIBRARIES ICU::dt)
    set_property(TARGET icu::icu APPEND PROPERTY INTERFACE_LINK_LIBRARIES ICU::data)

########## For the modules (FindXXX)
set(icu_LIBRARIES_RELEASE icu::icu)
