# WebOS CMake Functions
# Functions for packaging re3 as a WebOS PDK IPK

set(CMAKE_EXECUTABLE_SUFFIX "")

# Function to generate appinfo.json manifest
function(webos_generate_appinfo TARGET)
    cmake_parse_arguments(APPINFO "" "NAME;ID;VERSION;VENDOR;ICON;MEMORY" "" ${ARGN})

    if(NOT APPINFO_NAME)
        set(APPINFO_NAME "${TARGET}")
    endif()

    if(NOT APPINFO_ID)
        set(APPINFO_ID "com.re3.${TARGET}")
    endif()

    if(NOT APPINFO_VERSION)
        set(APPINFO_VERSION "1.0.0")
    endif()

    if(NOT APPINFO_VENDOR)
        set(APPINFO_VENDOR "re3 Team")
    endif()

    if(NOT APPINFO_ICON)
        set(APPINFO_ICON "icon.png")
    endif()

    if(NOT APPINFO_MEMORY)
        set(APPINFO_MEMORY 300)
    endif()

    set(APPINFO_JSON "{
    \"title\": \"${APPINFO_NAME}\",
    \"type\": \"pdk\",
    \"main\": \"${TARGET}\",
    \"icon\": \"${APPINFO_ICON}\",
    \"id\": \"${APPINFO_ID}\",
    \"version\": \"${APPINFO_VERSION}\",
    \"vendor\": \"${APPINFO_VENDOR}\",
    \"requiredMemory\": ${APPINFO_MEMORY},
    \"orientation\": \"landscape\"
}")

    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_appinfo.json" "${APPINFO_JSON}")
    message(STATUS "Generated appinfo.json for ${TARGET}")
endfunction()

# Function to create IPK package
function(webos_create_ipk TARGET)
    cmake_parse_arguments(IPK "INSTALL" "ICON" "EXTRA_FILES" ${ARGN})

    get_target_property(TARGET_TYPE "${TARGET}" TYPE)
    if(NOT TARGET_TYPE STREQUAL "EXECUTABLE")
        return()
    endif()

    # Create staging directory
    set(STAGING_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_ipk_staging")

    # Custom target to build IPK
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.ipk"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${STAGING_DIR}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${STAGING_DIR}"
        # Strip binary and copy to staging
        COMMAND ${CMAKE_COMMAND} -E echo "Stripping debug symbols from ${TARGET}..."
        COMMAND ${CMAKE_STRIP} --strip-all "$<TARGET_FILE:${TARGET}>" -o "${STAGING_DIR}/$<TARGET_FILE_NAME:${TARGET}>"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_appinfo.json" "${STAGING_DIR}/appinfo.json"
        COMMAND ${CMAKE_COMMAND} -E echo "filemode.755=$<TARGET_FILE_NAME:${TARGET}>" > "${STAGING_DIR}/package.properties"
        # Create install scripts
        COMMAND ${CMAKE_COMMAND} -E echo "#!/bin/sh\nexit 0" > "${STAGING_DIR}/pmPostInstall.script"
        COMMAND ${CMAKE_COMMAND} -E echo "#!/bin/sh\nexit 0" > "${STAGING_DIR}/pmPreRemove.script"
        COMMAND chmod +x "${STAGING_DIR}/pmPostInstall.script" "${STAGING_DIR}/pmPreRemove.script"
        COMMENT "Packaging ${TARGET} as IPK"
        DEPENDS ${TARGET}
        VERBATIM
    )

    # Add icon if provided
    if(IPK_ICON AND EXISTS "${IPK_ICON}")
        add_custom_command(
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.ipk"
            APPEND
            COMMAND ${CMAKE_COMMAND} -E copy "${IPK_ICON}" "${STAGING_DIR}/"
        )
    endif()

    # Copy extra files if provided
    foreach(EXTRA_FILE ${IPK_EXTRA_FILES})
        if(EXISTS "${EXTRA_FILE}")
            add_custom_command(
                OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.ipk"
                APPEND
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${EXTRA_FILE}" "${STAGING_DIR}/"
            )
        endif()
    endforeach()

    # Call palm-package tool (note: requires palm-package in PATH or SDK bin)
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.ipk"
        APPEND
        COMMAND palm-package "${STAGING_DIR}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )

    # Create custom target for IPK
    add_custom_target(${TARGET}_ipk ALL
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.ipk"
    )

    if(IPK_INSTALL)
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.ipk"
            DESTINATION "."
        )
    endif()

    message(STATUS "IPK package target created: ${TARGET}_ipk")
endfunction()

# Main platform target function (called from CMakeLists.txt)
function(re3_platform_target TARGET)
    cmake_parse_arguments(RPT "INSTALL" "" "" ${ARGN})

    get_target_property(TARGET_TYPE "${TARGET}" TYPE)
    if(TARGET_TYPE STREQUAL "EXECUTABLE")
        # Generate appinfo.json
        webos_generate_appinfo(${TARGET}
            NAME "GTA III re3"
            ID "com.re3.gta3"
            VERSION "1.0.0"
            VENDOR "re3 Team"
            ICON "icon.png"
            MEMORY 512
        )

        # Create IPK package
        webos_create_ipk(${TARGET}
            INSTALL
        )

        message(STATUS "WebOS platform target configured for ${TARGET}")
    endif()
endfunction()
