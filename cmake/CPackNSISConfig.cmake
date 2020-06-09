# fastgrep core
set(CPACK_GENERATOR "NSIS")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL OFF)
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})
set(CPACK_NSIS_DISPLAY_NAME ${PROJECT_NAME})
set(CPACK_NSIS_UNINSTALL_NAME uninstall)
set(CPACK_NSIS_CONTACT ${CPACK_PACKAGE_CONTACT})

# use large strings build of nsis: https://nsis.sourceforge.io/Special_Builds#Large_strings
set(CPACK_NSIS_MODIFY_PATH ON)

set(CPACK_ALL_INSTALL_TYPES Full)
set(CPACK_COMPONENT_CORE_DISPLAY_NAME "${PROJECT_NAME} core")
set(CPACK_COMPONENT_CORE_DESCRIPTION "${PROJECT_DESCRIPTION}")
set(CPACK_COMPONENT_CORE_REQUIRED 1)
set(CPACK_COMPONENT_CORE_TYPES Full)

set(CPACK_COMPONENTS_ALL core)