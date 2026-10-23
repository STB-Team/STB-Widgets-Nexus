cmake_minimum_required(VERSION 3.21)

# ---- Cache build vars ----
option(COPY_BUILD "Copy the build output to the Skyrim directory." TRUE)
macro(set_from_environment VARIABLE)
	if (NOT DEFINED ${VARIABLE} AND DEFINED ENV{${VARIABLE}})
		set(${VARIABLE} $ENV{${VARIABLE}})
	endif ()
endmacro()

# ---- Vcpkg check ----
set_from_environment(VCPKG_ROOT)
if (NOT DEFINED VCPKG_ROOT)
	message(
		FATAL_ERROR
		"Variable VCPKG_ROOT is not set."
	)
endif ()

# ---- Include guards ----
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
	message(
		FATAL_ERROR
			"In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
	)
endif()

set(Boost_USE_STATIC_RUNTIME OFF CACHE BOOL "")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "")

# ---- Project ----

project(
	STB_Widgets
	VERSION 1.0.0
	LANGUAGES CXX
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/Version.h.in
	${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
	@ONLY
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
	@ONLY
)

# ---- Globals ----

if (MSVC)
	add_compile_definitions(
		_UNICODE
	)

	if (NOT ${CMAKE_GENERATOR} STREQUAL "Ninja")
		add_compile_options(
			/MP	# Build with Multiple Processes
		)
	endif ()
endif ()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

set(Boost_USE_STATIC_LIBS ON)

if (DEFINED UseUselessUtils AND UseUselessUtils)
	add_compile_definitions(USELESS_UTILS)
endif ()


# ---- Dependencies ----

# Use the bundled CommonLibSSE-NG (CharmedBaryon) checkout. NG ships its own
# unit tests which we don't need here, so disable them to avoid the Catch2 dep.
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(CommonLibSSE)

find_package(spdlog REQUIRED CONFIG)
find_package(glm REQUIRED CONFIG)
find_path(SIMPLEINI_INCLUDE_DIRS "SimpleIni.h" REQUIRED)

# ---- Add source files ----

set(SOURCES
	src/HudRequests.h
	src/HudRuntime.h
	src/HudRuntime.cpp
	src/main.cpp
	src/Hooks.cpp
	src/Hooks.h
	src/Serialization.cpp
	src/Serialization.h
	src/ImGuiMenu.cpp
	src/ImGuiMenu.h
	src/InputHandler.cpp
	src/InputHandler.h
	src/WidgetManager.cpp
	src/WidgetManager.h
	src/WidgetWeight.cpp
	src/WidgetWeight.h
	src/WidgetGold.cpp
	src/WidgetGold.h
	src/WidgetResist.cpp
	src/WidgetResist.h
	src/WidgetLvlWidget.cpp
	src/WidgetLvlWidget.h
	src/MenuHandler.cpp
	src/MenuHandler.h
	src/WidgetShout.cpp
	src/WidgetShout.h
	src/WidgetPlayTime.cpp
	src/WidgetPlayTime.h
	src/WidgetEquip.cpp
	src/WidgetEquip.h
	src/WidgetGameTime.cpp
	src/WidgetGameTime.h
	src/Settings.h
	src/SKSEMenuFramework.h
	src/PCH.h
)

source_group(
	TREE
		${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		${SOURCES}
)

source_group(
	TREE
		${CMAKE_CURRENT_BINARY_DIR}
	FILES
		${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
)

# ---- Create DLL ----

add_library(
	${PROJECT_NAME}
	SHARED
	${SOURCES}
	${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
	.clang-format
	.editorconfig
)

target_compile_features(
	${PROJECT_NAME}
	PRIVATE
		cxx_std_23
)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${SIMPLEINI_INCLUDE_DIRS}
)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		CommonLibSSE::CommonLibSSE
		spdlog::spdlog
		glm::glm
)

if (DEFINED UseUselessUtils AND UseUselessUtils)
	target_link_libraries(
		${PROJECT_NAME}
		PRIVATE
			UselessFenixUtils::UselessFenixUtils
	)
endif ()

if (MSVC)
	target_compile_options(
		${PROJECT_NAME}
		PRIVATE
      /std:c17
      
			/sdl	# Enable Additional Security Checks
			/utf-8	# Set Source and Executable character sets to UTF-8
			/Zi	# Debug Information Format

			/permissive-	# Standards conformance

			/Zc:alignedNew	# C++17 over-aligned allocation
			/Zc:auto	# Deduce Variable Type
			/Zc:char8_t
			/Zc:__cplusplus	# Enable updated __cplusplus macro
			/Zc:externC
			/Zc:externConstexpr	# Enable extern constexpr variables
			/Zc:forScope	# Force Conformance in for Loop Scope
			/Zc:hiddenFriend
			/Zc:implicitNoexcept	# Implicit Exception Specifiers
			/Zc:lambda
			/Zc:noexceptTypes	# C++17 noexcept rules
			/Zc:preprocessor	# Enable preprocessor conformance mode
			/Zc:referenceBinding	# Enforce reference binding rules
			/Zc:rvalueCast	# Enforce type conversion rules
			/Zc:sizedDealloc	# Enable Global Sized Deallocation Functions
			/Zc:strictStrings	# Disable string literal type conversion
			/Zc:ternary	# Enforce conditional operator rules
			/Zc:threadSafeInit	# Thread-safe Local Static Initialization
			/Zc:tlsGuards
			/Zc:trigraphs	# Trigraphs Substitution
			/Zc:wchar_t	# wchar_t Is Native Type

			/external:anglebrackets
			/external:W0

			/W4	# Warning level
			/WX	# Warning level (warnings are errors)

			"$<$<CONFIG:DEBUG>:>"
			"$<$<CONFIG:RELEASE>:/Zc:inline;/JMC-;/Ob3>"
	)

	target_link_options(
		${PROJECT_NAME}
		PRIVATE
			/WX	# Treat Linker Warnings as Errors

			"$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
			"$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
	)
endif ()

target_precompile_headers(
	${PROJECT_NAME}
	PRIVATE
		src/PCH.h
)


if (COPY_BUILD)
  set(STB_WIDGETS_DEPLOY_DIR "C:/GAMES/The Elder Scrolls V Skyrim - Special Edition/[STB] Mod Organizer/mods/STB Widgets2/SKSE/Plugins")
  add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${STB_WIDGETS_DEPLOY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${PROJECT_NAME}> "${STB_WIDGETS_DEPLOY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_PDB_FILE:${PROJECT_NAME}> "${STB_WIDGETS_DEPLOY_DIR}"
  )
endif ()


