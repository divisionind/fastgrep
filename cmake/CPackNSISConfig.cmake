# fastgrep - a multi-threaded tool to search for files containing a pattern
# Copyright (C) 2020, Andrew Howard, <divisionind.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# fastgrep core
set(CPACK_GENERATOR "NSIS")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL OFF)
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})
set(CPACK_NSIS_DISPLAY_NAME ${PROJECT_NAME})
set(CPACK_NSIS_UNINSTALL_NAME uninstall)
set(CPACK_NSIS_CONTACT ${CPACK_PACKAGE_CONTACT})
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

# use large strings build of nsis: https://nsis.sourceforge.io/Special_Builds#Large_strings
set(CPACK_NSIS_MODIFY_PATH ON)

set(CPACK_ALL_INSTALL_TYPES Full)
set(CPACK_COMPONENT_CORE_DISPLAY_NAME "${PROJECT_NAME} core")
set(CPACK_COMPONENT_CORE_DESCRIPTION "${PROJECT_DESCRIPTION}")
set(CPACK_COMPONENT_CORE_REQUIRED 1)
set(CPACK_COMPONENT_CORE_TYPES Full)

set(CPACK_COMPONENTS_ALL core)