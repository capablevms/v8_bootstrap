## TW: Borrowed Graeme Jenkinson 24-02-12, when it was used to compile purecap Chromium.
##     Repurposed to compile hybrid V8.
##     WIP, though compiles with some manual intervention.
##     Originally copyrighted by Graeme under the BSD 2-clause license
##     See LICENSE-BSD-2-gjenkinson


# Before using, you'll want to do the following…:
# Install dependencies:
#   pkg64c install devel/icu
#   pkg64 install git devel/py-Jinja2 cmake llvm-morello gn python39 pkgconf glib ninja dbus atkmm nss libdrm pango mesa-libs mesa-dri protobuf protobuf-c protozero py39-types-protobuf py39-protobuf cproto prototool
# Edit /etc/make.conf to include:
#   USE_PACKAGE_DEPENDS=yes
#   USE_PACKAGE_DEPENDS_REMOTE=yes
#   USE_PACKAGE_64_DEPENDS_ONY=yes
#   OPTIONS_UNSET_FORCE+=	DOCS MANPAGES INFO NLS
# Avoid prot_max weirdnesses with mprotect:
#   sysctl vm.imply_prot_max=0
# symlink python3, protoc, c++, cc, c++, cxx from $HOME/bin to the morello toolchain (unclear whether this is still required, but I've kept it)
# Make sure $HOME/bin is in your $PATH

# `make build` to build v8
# `make helloworld` to compile a `helloworld` binary, which will be located in out/hybrid.
# `make d8` and `make content-shell` to build d8 or content-shell

# You can build things with the CHERI_HYBRID flag disabled using `make <target>-legacy`.
# Build args that differ between cheri and non-cheri builds are separated into their own lists later in this file.

# Compilation will fail after a few hundred objects with a bizarre error from protoc, compilaining that no types are defined (e.g. bool).
# Fix this with:
#   rm out/hybrid/protoc && cp `which protoc` out/hybrid/protoc
# Then `make build` to continue the compilation. Good luck...

# NB: repeatedly `make`-ing will re-apply patches to the codebase! Normal workflow is to…:
#  1) Make a change locally
#  2) rsync to remote
#     (alternative is to use your git repo, but with lots of small changes the history will get messy!)
#  3) `make configure` if GN args need to change
#  4) `make build && make helloworld`, or another second target if you're working on something specific

# If you're trying to cross-compile locally or working with non-v8 targets (e.g. v8-monolith), the configuration options you care about are likely in the `set` commands at the top of this file!



cmake_minimum_required(VERSION 3.24)
include(ExternalProject)

project(hybrid-v8)

SET(WEBASSEMBLY_ENABLED true      CACHE BOOL   "Whether to enable V8's sandbox")
SET(SANDBOX_ENABLED     true       CACHE BOOL   "Whether to enable V8's sandbox")
SET(CHERI_ENABLED       true       CACHE BOOL   "Whether CHERI is enabled, in hybrid mode")
SET(JS_SHELL            "d8"       CACHE STRING "The name of the executable for the current preferred JS shell") 
SET(gn-target           d8         CACHE STRING "Build target for ninja")

IF(${WEBASSEMBLY_ENABLED})
	SET(gn-dir-webassembly-suffix "")
ELSE()
	SET(gn-dir-webassembly-suffix "_webassembly_disabled")
ENDIF()

IF(${SANDBOX_ENABLED})
	SET(gn-dir-sandbox-suffix  "")
ELSE()
	SET(gn-dir-sandbox-suffix "_sandbox_disabled")
ENDIF()

IF(${CHERI_ENABLED})
	SET(gn-dir-cheri-suffix  "")
ELSE()
	SET(gn-dir-cheri-suffix "_legacy")
ENDIF()

SET(gn-dir out/hybrid_debug${gn-dir-cheri-suffix}${gn-dir-sandbox-suffix}${gn-dir-webassembly-suffix})

set(v8-dir ${CMAKE_SOURCE_DIR}/v8)
set(so-dir $ENV{HOME}/lib/)
set(home-bin $ENV{HOME}/bin/)
set(toolchain-syspath /usr/local64/)
set(toolchain-bin ${toolchain-syspath}bin/)
set(toolchain-lib ${toolchain-syspath}lib/)
set(toolchain-include ${toolchain-syspath}include/)
set(rustc-version "rustc 1.71.0 (7bb61853a 2024-07-10")
set(thirdparty-libcpp ${CMAKE_SOURCE_DIR}/third_party/libc++/src/include/)

find_package(Git)
find_package(
        Python3
        REQUIRED
        COMPONENTS Interpreter
)

# Chromium version 128.0.6613.114
set(FREEBSD_PORTS_REVISION_FOR_CHROMIUM_VERSION 4b35fdbd99dd03b971c420a62bd44b48c98a224f)
ExternalProject_Add(
  freebsd-ports 
  SOURCE_DIR     freebsd-ports
  GIT_REPOSITORY https://github.com/freebsd/freebsd-ports.git
  GIT_SHALLOW    false
  GIT_TAG        ${FREEBSD_PORTS_REVISION_FOR_CHROMIUM_VERSION}
  GIT_PROGRESS   true
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
set_target_properties(freebsd-ports PROPERTIES EXCLUDE_FROM_ALL TRUE)
 
## Install dependencies (DEPS file)
#
#set(CHROMIUM_GIT https://chromium.googlesource.com)
#
## set(ANGLE_REVISION 1cedd773c14a6afc7223fb12426f9b575c8ae4c8)
## ExternalProject_Add(
##   angle
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/angle
##   GIT_REPOSITORY ${CHROMIUM_GIT}/angle/angle.git
##   GIT_TAG        ${ANGLE_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
#
## set(FLAC_REVISION 689da3a7ed50af7448c3f1961d1791c7c1d9c85c)
## ExternalProject_Add(
##   flac
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/flac
##   GIT_REPOSITORY ${CHROMIUM_GIT}/chromium/deps/flac.git
##   GIT_TAG        ${FLAC_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(BORINGSSL_GIT https://boringssl.googlesource.com)
## set(BORINGSSL_REVISION 689da3a7ed50af7448c3f1961d1791c7c1d9c85c)
## ExternalProject_Add(
##   boringssl
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/boringssl/src
##   GIT_REPOSITORY ${BORINGSSL_GIT}/boringssl.git
##   GIT_TAG        ${BORING_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(CATAPULT_REVISION 4f43388d908cb2c5e3cf788a9018be512b6c7653)
## ExternalProject_Add(
##   catapult
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/catapult
##   GIT_REPOSITORY ${CHROMIUM_GIT}/catapult.git
##   GIT_TAG        ${CATAPULT_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(CLD_3_REVISION b48dc46512566f5a2d41118c8c1116c4f96dc661)
## ExternalProject_Add(
##   cld_3
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/cld_3/src
##   GIT_REPOSITORY ${CHROMIUM_GIT}/external/github.com/google/cld_3.git
##   GIT_TAG        ${CLD_3_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(DAWN_GIT https://dawn.googlesource.com)
## set(DAWN_REVISION 53c41d10d68009d74e953a780befa66ef4b59498)
## ExternalProject_Add(
##   dawn
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/dawn
##   GIT_REPOSITORY ${DAWN_GIT}/dawn.git
##   GIT_TAG        ${DAWN_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
#
#set(DEVTOOLS_FRONTEND_REVISION 47f04a536265028feb1254a3663853e0ce526455)
#ExternalProject_Add(
#  devtools-frontend
#  SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/devtools-frontend/src
#  GIT_REPOSITORY ${CHROMIUM_GIT}/devtools/devtools-frontend.git
#  GIT_TAG        ${DEVTOOLS_FRONTEND_REVISION}
#  GIT_PROGRESS   true
#  CONFIGURE_COMMAND ""
#  BUILD_COMMAND ""
#  INSTALL_COMMAND ""
#  )
#
## 
## set(FFMPEG_REVISION 8d21d41d8bec5c0b266ee305d1a708dc5c23b594)
## ExternalProject_Add(
##   ffmpeg
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/ffmpeg
##   GIT_REPOSITORY ${CHROMIUM_GIT}/chromium/third_party/ffmpeg.git
##   GIT_TAG        ${FFMPEG_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(FONTCONFIG_REVISION 06929a556fdc39c8fe12965b69070c8df520a33e)
##   ExternalProject_Add(
##   fontconfig
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/fontconfig/src
##   GIT_REPOSITORY ${CHROMIUM_GIT}/external/fontconfig.git
##   GIT_TAG        ${FONTCONFIG_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
#
#set(ICU_REVISION 9408c6fd4a39e6fef0e1c4077602e1c83b15f3fb)
#  ExternalProject_Add(
#  icu
#  SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/icu
#  GIT_REPOSITORY ${CHROMIUM_GIT}/chromium/deps/icu.git
#  GIT_TAG        ${ICU_REVISION}
#  GIT_PROGRESS   true
#  CONFIGURE_COMMAND ""
#  BUILD_COMMAND ""
#  INSTALL_COMMAND ""
#  )
#
## set(LIBJPEG_TURBO_REVISION aa4075f116e4312537d0d3e9dbd5e31096539f94)
## ExternalProject_Add(
##   libjpeg_turbo
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/libjpeg_turbo
##   GIT_REPOSITORY ${CHROMIUM_GIT}/chromium/deps/libjpeg_turbo.git
##   GIT_TAG        ${LIBJPEG_TURBO_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(LIBPHONENUMBER_REVISION 140dfeb81b753388e8a672900fb7a971e9a0d362)
## ExternalProject_Add(
##   libphonenumber
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/libphonenumber/dist
##   GIT_REPOSITORY ${CHROMIUM_GIT}/external/libphonenumber.git
##   GIT_TAG        ${LIBPHONENUMBER_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(LIBSYNC_REVISION f4f4387b6bf2387efbcfd1453af4892e8982faf6)
## ExternalProject_Add(
##   libsync
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/libsync/src
##   GIT_REPOSITORY ${CHROMIUM_GIT}/aosp/platform/system/core/libsync.git
##   GIT_TAG        ${LIBSYNC_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(LIBWEBP_REVISION fd7b5d48464475408d32d2611bdb6947d4246b97)
## ExternalProject_Add(
##   libwebp
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/libwebp/src
##   GIT_REPOSITORY ${CHROMIUM_GIT}/webm/libwebp.git
##   GIT_TAG        ${LIBWEBP_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(LIBYUV_REVISION dcbe0820709c274beb4624588d1aa14b25557d62)
## ExternalProject_Add(
##   libyuv
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/libyuv
##   GIT_REPOSITORY ${CHROMIUM_GIT}/libyuv/libyuv.git
##   GIT_TAG        ${LIBYUV_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(NASM_REVISION 7fc833e889d1afda72c06220e5bed8fb43b2e5ce)
## ExternalProject_Add(
##   nasm
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/nasm
##   GIT_REPOSITORY ${CHROMIUM_GIT}/chromium/deps/nasm.git
##   GIT_TAG        ${NASM_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(OPENSCREEN_REVISION d052a5556a78aa4463852e29919d616b47237842)
## ExternalProject_Add(
##   openscreen 
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/openscreen/src
##   GIT_REPOSITORY ${CHROMIUM_GIT}/openscreen
##   GIT_TAG        ${OPENSCREEN_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(PDFIUM_GIT https://pdfium.googlesource.com)
## set(PDFIUM_REVISION c739696a25a3b515e208cafa2e944d377ee37307)
## ExternalProject_Add(
##   pdfium
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/pdfium
##   GIT_REPOSITORY ${PDFIUM_GIT}/pdfium.git
##   GIT_TAG        ${PDFIUM_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
#
#set(ANDROID_GIT https://android.googlesource.com)
#set(PERFETTO_REVISION 077d742d9f9738a0453e408560a283a7514c9e8e)
#ExternalProject_Add(
#  perfetto
#  SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/perfetto
#  GIT_REPOSITORY ${ANDROID_GIT}/platform/external/perfetto.git
#  GIT_TAG        ${PERFETTO_REVISION}
#  GIT_PROGRESS   true
#  CONFIGURE_COMMAND ""
#  BUILD_COMMAND ""
#  INSTALL_COMMAND ""
#  )
#
#set(SKIA_GIT https://skia.googlesource.com)
#set(SKIA_REVISION 8bd493b850f1a75482af8f30cb492cd70645498c)
#ExternalProject_Add(
#  skia
#  SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/skia
#  GIT_REPOSITORY ${SKIA_GIT}/skia.git
#  GIT_TAG        ${SKIA_REVISION}
#  GIT_PROGRESS   true
#  CONFIGURE_COMMAND ""
#  BUILD_COMMAND ""
#  INSTALL_COMMAND ""
#  )
#
## # skipped patch: tools/gn/build_gen.py
## # skipped patch: tools/gn/src/gn_version.h
## 
## set(SQLITE_REVISION f6752b7ed1fe3cc1491c0c47ec5804ee2bd0e59b)
## ExternalProject_Add(
##   sqlite
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/sqlite/src
##   GIT_REPOSITORY ${CHROMIUM_GIT}/chromium/deps/sqlite.git
##   GIT_TAG        ${SQLITE_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(SWIFTSHADER_GIT https://swiftshader.googlesource.com)
## set(SWIFTSHADER_REVISION f549d5e6c6635ec8b75fb544a6bdc9f48bfb1dd3)
## ExternalProject_Add(
##   swiftshader
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/swiftshader
##   GIT_REPOSITORY ${SWIFTSHADER_GIT}/SwiftShader.git
##   GIT_TAG        ${SWIFTSHADER_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(VULKANDEPS_REVISION 94574e5c2e14614aae7251f26a5492fec966abf0)
## ExternalProject_Add(
##   vulkan-deps
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/vulkan-deps
##   GIT_REPOSITORY ${CHROMIUM_GIT}/vulkan-deps
##   GIT_TAG        ${VULKANDEPS_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(GLSLANG_REVISION a5bf69936ddebd45370c2e2c392b4b6f26296bad)
## ExternalProject_Add(
##   glslang
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/vulkan-deps/glslang/src
##   GIT_REPOSITORY ${CHROMIUM_GIT}/external/github.com/KhronosGroup/glslang
##   GIT_TAG        ${GLSLANG_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(SPIRV_CROSS_REVISION 12542fc6fc05000e04742daf93892a0b10edbe80)
## ExternalProject_Add(
##   spirv-cross
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/vulkan-deps/spirv-cross/src
##   GIT_REPOSITORY ${CHROMIUM_GIT}/external/github.com/KhronosGroup/SPIRV-Cross
##   GIT_TAG        ${SPIRV_CROSS_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(SPIRV_HEADERS_REVISION bdbfd019be6952fd8fa9bd5606a8798a7530c853)
## ExternalProject_Add(
##   spirv-headers
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/vulkan-deps/spirv-headers/src
##   GIT_REPOSITORY ${CHROMIUM_GIT}/external/github.com/KhronosGroup/SPIRV-Headers
##   GIT_TAG        ${SPIRV_HEADERS_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(SPIRV_TOOLS_REVISION e7c6084fd1d6d6f5ac393e842728d8be309688ca)
## ExternalProject_Add(
##   spirv-tools
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/vulkan-deps/spirv-tools/src
##   GIT_REPOSITORY ${CHROMIUM_GIT}/external/github.com/KhronosGroup/SPIRV-Tools
##   GIT_TAG        ${SPIRV_TOOLS_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(VULKAN_HEADERS_REVISION 9e61870ecbd32514113b467e0a0c46f60ed222c7)
## ExternalProject_Add(
##   vulkan-headers
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/vulkan-deps/vulkan-headers/src
##   GIT_REPOSITORY ${CHROMIUM_GIT}/external/github.com/KhronosGroup/Vulkan-Headers
##   GIT_TAG        ${VULKAN_HEADERS_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(VULKAN_LOADER_REVISION b95d065ec567f862ab5baa85ab99ecb1f8979fbc)
## ExternalProject_Add(
##   vulkan-loader
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/vulkan-deps/vulkan-loader/src
##   GIT_REPOSITORY ${CHROMIUM_GIT}/external/github.com/KhronosGroup/Vulkan-Loader
##   GIT_TAG        ${VULKAN_LOADER_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(VULKAN_TOOLS_REVISION 80b010b1e1b08c1f092fb2bfa337faadf8ea1ba3)
## ExternalProject_Add(
##   vulkan-tools
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/vulkan-deps/vulkan-tools/src
##   GIT_REPOSITORY ${CHROMIUM_GIT}/external/github.com/KhronosGroup/Vulkan-Tools
##   GIT_TAG        ${VULKAN_TOOLS_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(VULKAN_VALIDATION_LAYERS_REVISION 414ee5c8fe6ede3ba44d9ca8832ef974f1158cd5)
## ExternalProject_Add(
##   vulkan-validation-layers
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/vulkan-deps/vulkan-validation-layers/src
##   GIT_REPOSITORY ${CHROMIUM_GIT}/external/github.com/KhronosGroup/Vulkan-ValidationLayers
##   GIT_TAG        ${VULKAN_VALIDATION_LAYERS_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
## set(VULCANMEMORYALLOCATOR_REVISION ebe84bec02c041d28f902da0214bf442743fc907)
## ExternalProject_Add(
##   vulkan_memory_allocator
##   SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/vulkan_memory_allocator
##   GIT_REPOSITORY ${CHROMIUM_GIT}/external/github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
##   GIT_TAG        ${VULCANMEMORYALLOCATOR_REVISION}
##   CONFIGURE_COMMAND ""
##   BUILD_COMMAND ""
##   INSTALL_COMMAND ""
##   )
## 
#
#set(WEBRTC_GIT https://webrtc.googlesource.com)
## WARNING!! The commit history for webrtc seems to get rewritten sometimes. Update at your peril...!
#set(WEBRTC_REVISION b47cd6fbe315690756f2f03e7658d4e26fe27b1e)
#ExternalProject_Add(
#  webrtc
#  SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/webrtc
#  GIT_REPOSITORY ${WEBRTC_GIT}/src.git
#  GIT_TAG        main
#  GIT_SHALLOW	 true
#  GIT_PROGRESS   true
#  CONFIGURE_COMMAND ""
#  BUILD_COMMAND ""
#  INSTALL_COMMAND ""
#  )
#
#set(PROBABLYTOM_V8_GIT https://github.com/probablytom/v8.git)
#set(V8_REVISION compartment_switching_around_control_flow_integrity)
#ExternalProject_Add(
#  v8
#  SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/v8
#  GIT_REPOSITORY ${PROBABLYTOM_V8_GIT}
#  GIT_TAG        ${V8_REVISION}
#  GIT_SHALLOW	 true
#  GIT_PROGRESS   true
#  CONFIGURE_COMMAND ""
#  BUILD_COMMAND ""
#  INSTALL_COMMAND ""
#  )
#
#set(XDGUTILS_REVISION cb54d9db2e535ee4ef13cc91b65a1e2741a94a44)
#ExternalProject_Add(
#  xdg-utils
#  SOURCE_DIR	 ${CMAKE_SOURCE_DIR}/third_party/xdg-utils
#  GIT_REPOSITORY ${CHROMIUM_GIT}/chromium/deps/xdg-utils.git
#  GIT_TAG        ${XDGUTILS_REVISION}
#  GIT_PROGRESS   true
#  CONFIGURE_COMMAND ""
#  BUILD_COMMAND ""
#  INSTALL_COMMAND ""
#  )


add_custom_target(apply-patches
	COMMAND find freebsd-ports/www/chromium/files/patch-* -type f -exec sed -i \"\" -e 's/\.{LOCALBASE}/\\\/usr\\\/local64/g' {} '\;'
	COMMAND find ${CMAKE_BINARY_DIR}/freebsd-ports/www/chromium/files -type f -name 'patch-*' -print0 | xargs -t -0 -n 1 patch -p0 -u -N -f -d ${CMAKE_SOURCE_DIR} -i || true
	COMMAND find ${CMAKE_BINARY_DIR}/cheribsd-patches -type f -name 'patch-*' -print0 | xargs -t -0 -n 1 patch -p0 -u -N -d ${CMAKE_SOURCE_DIR} -i || true
)

add_custom_target(apply-v8-patches
	COMMAND find ${CMAKE_BINARY_DIR}/freebsd-ports/www/chromium/files -type f -name 'patch-v8_*' -print0 | xargs -t -0 -n 1 patch -p0 -u -N -f -d ${CMAKE_SOURCE_DIR} -i || true
	COMMAND find ${CMAKE_BINARY_DIR}/cheribsd-patches -type f -name 'patch-v8_*' -print0 | xargs -t -0 -n 1 patch -p0 -u -N -d ${CMAKE_SOURCE_DIR} -i || true
)

add_custom_target(undo-patches
	COMMAND find ${CMAKE_BINARY_DIR}/freebsd-ports/www/chromium/files -type f -name 'patch-*' -print0 | xargs -t -0 -n 1 patch -R -p0 -u -N -f -d ${CMAKE_SOURCE_DIR} -i || true
	COMMAND find ${CMAKE_BINARY_DIR}/cheribsd-patches -type f -name 'patch-*' -print0 | xargs -t -0 -n 1 patch -R -p0 -u -N -d ${CMAKE_SOURCE_DIR} -i || true
)

add_custom_target(clean-orig-files
	COMMAND git submodule foreach --recursive find . -type f -name *.orig -delete
	COMMAND find . -type f -name *.orig -delete
)

add_custom_target(reset-entire-project
	COMMAND git submodule foreach --recursive \"git restore . && git clean -f\"
	COMMAND git restore .
	COMMAND git clean -f
)

  #set(ccache_distcc_configuration DISTCC_CMDLIST=/home/probablytom/.distcc/cmdlist DISTCC_VERBOSE=1 DISTCC_LOG=~/.distcc/logfile DISTCC_HOSTS=\\"@bencher15.soft-dev.org/64\\" CCACHE_PREFIX=distcc CCACHE_SLOPPINESS=time_macros ccache)
  set(ccache_distcc_configuration DISTCC_CMDLIST=/home/probablytom/.distcc/cmdlist DISTCC_VERBOSE=1 DISTCC_LOG=~/.distcc/logfile DISTCC_HOSTS=\\"@bencher15.soft-dev.org/64\\" CCACHE_SLOPPINESS=time_macros ccache)

set(GN_ARGS_BASE
  arm_use_neon=false
  cc_wrapper="${ccache_distcc_configuration}"
  clang_base_path="${toolchain-syspath}"
  clang_use_chrome_plugins=false
  clang_version="-morello"
  enable_backup_ref_ptr_support=false
  enable_base_tracing=false
  enable_dav1d_decoder=false
  enable_nacl=false
  enable_perfetto_ipc=false
  enable_perfetto_trace_processor=true
  enable_perfetto_trace_processor_httpd=false
  enable_perfetto_trace_processor_json=false
  enable_perfetto_trace_processor_sqlite=true
  enable_perfetto_system_consumer=false
  enable_perfetto_unittests=false
  enable_perfetto_zlib=false
  enable_rust=true
  enable_trace_logging=true
  extra_ldflags="-Wl,-znotext,-L${toolchain-lib},-L/usr/local/lib -g -O0"
  fatal_linker_warnings=false
  icu_use_data_file=false
  is_clang=true
  is_component_build=false
  is_debug=true
  #libyuv_build_with_neon=false
  optional_trace_events_enabled=false
  perfetto_use_system_protobuf=true
  perfetto_use_system_sqlite=true
  perfetto_use_system_zlib=true
  rtc_build_libevent=true
  rtc_build_with_neon=false
  rtc_disable_trace_events=true
  rtc_enable_google_benchmarks=false
  rtc_enable_libevent=false
  rtc_enable_protobuf=false
  rtc_exclude_audio_processing_module=true
  rust_sysroot_absolute="${toolchain-syspath}"
  rustc_version="${rustc-version}"
  symbol_level=2
  target_os="freebsd"
  target_sysroot_dir="${toolchain-sysroot}"
  target_cpu="arm64"
  treat_warnings_as_errors=false
  #use_allocator_shim=false
  use_aura=true
  use_debug_fission=true
  use_custom_libcxx=false
  #use_cxx17=true
  use_lld=true
  use_libinput=false
  #use_partition_alloc=true
  #use_partition_alloc_as_malloc=false
  #use_perfetto_client_library=true
  use_perfetto_trace_processor=false
  use_sysroot=false
  use_system_freetype=false
  use_system_harfbuzz=true
  use_system_libjpeg=true
  use_system_zlib=true
  use_udev=false
  v8_annotate_torque_ir=false
  v8_advanced_bigint_algorithms=false
  v8_disable_write_barriers=false
  v8_enable_backtrace=true
  #v8_enable_conservative_stack_scanning=true
  #v8_enable_direct_handle=true
  v8_enable_external_code_space=true
  v8_enable_gdbjit=false # not supported on aarch64!
  v8_enable_fast_torque=false
  v8_enable_i18n_support=false
  v8_enable_maglev=true
  v8_enable_memory_corruption_api=${SANDBOX_ENABLED}
  v8_enable_object_print=true
  v8_enable_sandbox=${SANDBOX_ENABLED}
  v8_enable_single_generation=false
  v8_enable_sparkplug=true
  v8_enable_trace_ignition=true
  v8_enable_turbofan=true
  v8_enable_verify_heap=true
  v8_enable_webassembly=${WEBASSEMBLY_ENABLED}
  v8_monolithic=false
  v8_optimized_debug=false
  v8_use_external_startup_data=false
  v8_use_perfetto=false
  v8_use_zlib=false
  )

IF(${CHERI_ENABLED})
	# CHERI-SPECIFIC
	set(GN_ARGS ${GN_ARGS_BASE}
	  custom_toolchain="//v8/tools/toolchain:morello_llvm_hybrid_native"
	  extra_cflags="-mabi=aapcs -march=morello -Xclang -morello-vararg=new -I/usr/local/include -I${toolchain-lib} -I/usr/include -I${CMAKE_SOURCE_DIR}/chromium/third_party/perfetto -D_BSD_SOURCE --include sys/types.h -g -O0 -DCHERI_HYBRID -DCHERI_PROTECT_GENERATED_CODE -Wno-unknown-warning-option -Wno-unused-command-line-argument -Wno-implicit-int-conversion -Wno-thread-safety-analysis -Wno-c++98-compat-extra-semi"
	  extra_cppflags="-mabi=aapcs -march=morello -Xclang -morello-vararg=new -I/usr/local/include -I${toolchain-lib} -I/usr/include -I${CMAKE_SOURCE_DIR}/chromium/third_party/perfetto -D_BSD_SOURCE --include sys/types.h -g -O0 -DCHERI_HYBRID -DCHERI_PROTECT_GENERATED_CODE -Wno-unknown-warning-option -Wno-unused-command-line-argument -Wno-implicit-int-conversion -Wno-thread-safety-analysis -Wno-c++98-compat-extra-semi"
	  extra_cxxflags="-mabi=aapcs -march=morello -Xclang -morello-vararg=new -I/usr/local/include -I${toolchain-lib} -I/usr/include -I${CMAKE_SOURCE_DIR}/chromium/third_party/perfetto -D_BSD_SOURCE --include sys/types.h -g -O0 -DCHERI_HYBRID -DCHERI_PROTECT_GENERATED_CODE -Wno-unknown-warning-option -Wno-unused-command-line-argument -Wno-implicit-int-conversion -Wno-thread-safety-analysis -Wno-c++98-compat-extra-semi"
	  host_toolchain="//v8/tools/toolchain:morello_llvm_hybrid_native"
	  #host_system_allocator_toolchain="//v8/tools/toolchain:morello_llvm_hybrid_native"
	  #v8_snapshot_toolchain="//v8/tools/toolchain:morello_llvm_hybrid_native"
	  )
ELSE()
	# LEGACY, NON-CHERI ARGS
	set(GN_ARGS_LEGACY ${GN_ARGS_BASE}
	  custom_toolchain="//v8/tools/toolchain:morello_llvm_legacy_native"
	  extra_cflags="-mabi=aapcs -march=morello -Xclang -morello-vararg=new -I/usr/local/include -I${toolchain-lib} -I/usr/include -I${CMAKE_SOURCE_DIR}/chromium/third_party/perfetto -D_BSD_SOURCE --include sys/types.h -g -O0 -Wno-unknown-warning-option -Wno-unused-command-line-argument -Wno-implicit-int-conversion -Wno-thread-safety-analysis -Wno-c++98-compat-extra-semi"
	  extra_cppflags="-mabi=aapcs -march=morello -Xclang -morello-vararg=new -I/usr/local/include -I${toolchain-lib} -I/usr/include -I${CMAKE_SOURCE_DIR}/chromium/third_party/perfetto -D_BSD_SOURCE --include sys/types.h -g -O0 -Wno-unknown-warning-option -Wno-unused-command-line-argument -Wno-implicit-int-conversion -Wno-thread-safety-analysis -Wno-c++98-compat-extra-semi"
	  extra_cxxflags="-mabi=aapcs -march=morello -Xclang -morello-vararg=new -I/usr/local/include -I${toolchain-lib} -I/usr/include -I${CMAKE_SOURCE_DIR}/chromium/third_party/perfetto -D_BSD_SOURCE --include sys/types.h -g -O0 -Wno-unknown-warning-option -Wno-unused-command-line-argument -Wno-implicit-int-conversion -Wno-thread-safety-analysis -Wno-c++98-compat-extra-semi"
	  host_toolchain="//v8/tools/toolchain:morello_llvm_legacy_native"
	  #host_system_allocator_toolchain="//v8/tools/toolchain:morello_llvm_legacy_native"
	  #v8_snapshot_toolchain="//v8/tools/toolchain:morello_llvm_legacy_native"
	  )
ENDIF()



add_custom_target(hooks ALL
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/build/util/lastchange.py -o ${CMAKE_SOURCE_DIR}/build/util/LASTCHANGE
  )

set(DOLLAR "$")
add_custom_target(pre-configure
  #COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/gn/bootstrap/bootstrap.py --debug --no-clean --no-rebuild --skip-generate-buildfiles
  COMMAND echo "checkout_google_benchmark = false" > build/config/gclient_args.gni
  COMMAND sed -i "" -e 's/\\\/usr\\\/local[^6]\\/\/usr\\\/local64/g' ${CMAKE_SOURCE_DIR}/base/base_paths_posix.cc
  COMMAND sed -i "" -e 's/\\\/usr\\\/local[^6]/\\\/usr\\\/local64/g' ${CMAKE_SOURCE_DIR}/chrome/common/chrome_paths.cc
  COMMAND sed -i "" -e 's/\\\/usr\\\/local[^6]/\\\/usr\\\/local64/g' ${CMAKE_SOURCE_DIR}/third_party/fontconfig/include/config.h
  COMMAND sed -i "" -e 's/\\\/usr\\\/local[^6]/\\\/usr\\\/local64/g' ${CMAKE_SOURCE_DIR}/third_party/test_fonts/fontconfig/BUILD.gn
  COMMAND sed -i "" -e 's/\\\/usr\\\/local[^6]/\\\/usr\\\/local64/g' ${CMAKE_SOURCE_DIR}/third_party/vulkan-deps/vulkan-loader/src/BUILD.gn
  COMMAND sed -i "" -e 's/\\\/usr\\\/local[^6]/\\\/usr\\\/local64/g' ${CMAKE_SOURCE_DIR}/third_party/vulkan-deps/vulkan-loader/src/loader/vk_loader_platform.h
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/build/linux/unbundle/replace_gn_files.py --system-libraries dav1d flac fontconfig freetype harfbuzz-ng icu libdrm libevent libpng libusb libwebp libxml libxslt openh264 opus snappy || false
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/third_party/ffmpeg/chromium/config/Chrome/linux/ ${CMAKE_SOURCE_DIR}/third_party/ffmpeg/chromium/config/Chrome/freebsd
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/third_party/ffmpeg/chromium/config/Chromium/linux/ ${CMAKE_SOURCE_DIR}/third_party/ffmpeg/chromium/config/Chromium/freesbd
  #COMMAND sed -i "" -e 's/\\\/usr\\\/local/\\\/usr\\\/local64/g' ${CMAKE_SOURCE_DIR}/third_party/wayland/features.gni
)

add_custom_target(configure-all
	COMMAND cmake . -DWEBASSEMBLY_ENABLED=true -DSANDBOX_ENABLED=true -DCHERI_ENABLED=true
	COMMAND make configure
	COMMAND cmake . -DWEBASSEMBLY_ENABLED=true -DSANDBOX_ENABLED=true -DCHERI_ENABLED=false
	COMMAND make configure
	COMMAND cmake . -DWEBASSEMBLY_ENABLED=true -DSANDBOX_ENABLED=false -DCHERI_ENABLED=true
	COMMAND make configure
	COMMAND cmake . -DWEBASSEMBLY_ENABLED=true -DSANDBOX_ENABLED=false -DCHERI_ENABLED=false
	COMMAND make configure
	COMMAND cmake . -DWEBASSEMBLY_ENABLED=false -DSANDBOX_ENABLED=true -DCHERI_ENABLED=true
	COMMAND make configure
	COMMAND cmake . -DWEBASSEMBLY_ENABLED=false -DSANDBOX_ENABLED=true -DCHERI_ENABLED=false
	COMMAND make configure
	COMMAND cmake . -DWEBASSEMBLY_ENABLED=false -DSANDBOX_ENABLED=false -DCHERI_ENABLED=true
	COMMAND make configure
	COMMAND cmake . -DWEBASSEMBLY_ENABLED=false -DSANDBOX_ENABLED=false -DCHERI_ENABLED=false
	COMMAND make configure
	# Restore cmake generated files from current configuration for next `make` command
	COMMAND cmake . -DWEBASSEMBLY_ENABLED="${WEBASSEMBLY_ENABLED}" -DSANDBOX_ENABLED="${SANDBOX_ENABLED}" -DCHERI_ENABLED="${CHERI_ENABLED}" 
)

IF(NOT ${CHERI_ENABLED})
	MESSAGE(NOTICE "Can't build with CHERI disabled --- there's an issue with rust versions. To fix. Sorry, future dev, if you're seeing this...!" )
	MESSAGE(NOTICE "" )
	MESSAGE(NOTICE "Build target `configure` will do nothing with CHERI disabled." )
	MESSAGE(NOTICE "" )
	add_custom_target(configure ALL
		COMMAND echo
		COMMAND echo "Cannot build with CHERI disabled --- we have some issue with rust versions. To fix. Sorry, future dev, if you see this...!"
		COMMAND echo
	)
ELSE()
	add_custom_target(configure ALL
		COMMAND if ! grep -q "morello_llvm_hybrid_native" ${v8-dir}/tools/toolchain/BUILD.gn$<SEMICOLON> then printf "\"gcc_toolchain(\\\"morello_llvm_hybrid_native\\\"){cc = \\\"${home-bin}clang-morello\\\"\\ncxx = \\\"${home-bin}clang++-morello\\\"\\nreadelf = \\\"${toolchain-bin}llvm-readelf-morello\\\"\\nnm = \\\"${toolchain-bin}llvm-nm-morello\\\"\\nar = \\\"${toolchain-bin}llvm-ar-morello\\\"\\nld = cxx\\nextra_ldflags=\\\"-Wl,-znotext,-L/lib,-L${toolchain-lib},-L/usr/local/lib -g -O0\\\"\\nextra_cflags = \\\"--target=aarch64-unknown-freebsd -mabi=aapcs -march=morello -Xclang -morello-vararg=new -I/usr/local/include -I${toolchain-include} -I/usr/include --include sys/types.h -g -O0 -DCHERI_HYBRID -DCHERI_PROTECT_GENERATED_CODE -Wno-unknown-warning-option -Wno-unused-command-line-argument -Wno-c++98-compat-extra-semi -Wno-implicit-int-conversion -Wno-thread-safety-analysis\\\"\\nextra_cxxflags = \\\"--target=aarch64-unknown-freebsd -mabi=aapcs -march=morello -Xclang -morello-vararg=new -I/usr/local/include -I${toolchain-include} -I/usr/include -I${CMAKE_SOURCE_DIR}/chromium/third_party/perfetto --include sys/types.h -g -O0 -DCHERI_HYBRID -DCHERI_PROTECT_GENERATED_CODE -Wno-unknown-warning-option -Wno-unused-command-line-argument -Wno-c++98-compat-extra-semi -Wno-implicit-int-conversion -Wno-thread-safety-analysis\\\"\\ntoolchain_args = {target_cpu = \\\"arm64\\\"\\ncurrent_cpu = \\\"arm64\\\"\\ncurrent_os = \\\"freebsd\\\"\\nis_clang = true\\nuse_lld = true\\nv8_enable_i18n_support = false\\nsimple_template_names = false}}\"" >> ${v8-dir}/tools/toolchain/BUILD.gn$<SEMICOLON> fi
		COMMAND if ! grep -q "morello_llvm_legacy_native" ${v8-dir}/tools/toolchain/BUILD.gn$<SEMICOLON> then printf "\"gcc_toolchain(\\\"morello_llvm_legacy_native\\\"){cc = \\\"${home-bin}clang-morello\\\"\\ncxx = \\\"${home-bin}clang++-morello\\\"\\nreadelf = \\\"${toolchain-bin}llvm-readelf-morello\\\"\\nnm = \\\"${toolchain-bin}llvm-nm-morello\\\"\\nar = \\\"${toolchain-bin}llvm-ar-morello\\\"\\nld = cxx\\nextra_ldflags=\\\"-Wl,-znotext,-L/lib,-L${toolchain-lib},-L/usr/local/lib -g -O0\\\"\\nextra_cflags = \\\"--target=aarch64-unknown-freebsd -mabi=aapcs -march=morello -Xclang -morello-vararg=new -I/usr/local/include -I${toolchain-include} -I/usr/include --include sys/types.h -g -O0 -Wno-unknown-warning-option -Wno-unused-command-line-argument -Wno-c++98-compat-extra-semi -Wno-implicit-int-conversion -Wno-thread-safety-analysis\\\"\\nextra_cxxflags = \\\"--target=aarch64-unknown-freebsd -mabi=aapcs -march=morello -Xclang -morello-vararg=new -I/usr/local/include -I${toolchain-include} -I/usr/include -I${CMAKE_SOURCE_DIR}/chromium/third_party/perfetto --include sys/types.h -g -O0 -Wno-unknown-warning-option -Wno-unused-command-line-argument -Wno-c++98-compat-extra-semi -Wno-implicit-int-conversion -Wno-thread-safety-analysis\\\"\\ntoolchain_args = {target_cpu = \\\"arm64\\\"\\ncurrent_cpu = \\\"arm64\\\"\\ncurrent_os = \\\"freebsd\\\"\\nis_clang = true\\nuse_lld = true\\nv8_enable_i18n_support = false\\nsimple_template_names = false}}\"" >> ${v8-dir}/tools/toolchain/BUILD.gn$<SEMICOLON> fi
	  COMMAND gn gen ${gn-dir} --args='${GN_ARGS}'
	  #COMMAND gn gen ${gn-dir}-legacy --args='${GN_ARGS_LEGACY}'
	  #COMMAND cp `which protoc` ${gn-dir}/protoc
	  #COMMAND cp `which protoc` ${gn-dir}-legacy/protoc
	  )
ENDIF()

add_custom_target(build ALL
  COMMAND ninja -C ${gn-dir} ${gn-target}
  COMMAND rm -f ${so-dir}/libv8.so
  COMMAND ln -s ${CMAKE_SOURCE_DIR}/${gn-dir}/libv8.so ${so-dir}/libv8.so
  COMMAND rm -f ${so-dir}/libv8_libbase.so
  COMMAND ln -s ${CMAKE_SOURCE_DIR}/${gn-dir}/libv8_libbase.so ${so-dir}/libv8_libbase.so
  COMMAND rm -f ${so-dir}/libv8_libplatform.so
  COMMAND ln -s ${CMAKE_SOURCE_DIR}/${gn-dir}/libv8_libplatform.so ${so-dir}/libv8_libplatform.so
)

add_custom_target(helloworld
  COMMAND clang++-morello -I. -Iv8 -Iv8/include v8/samples/hello-world.cc -o helloworld -fno-rtti -lv8 -lv8_libbase -lv8_libplatform -L${CMAKE_SOURCE_DIR}/${gn-dir}/obj/ -L${CMAKE_SOURCE_DIR}/${gn-dir} -pthread -std=c++20 -g -O0 -DV8_COMPRESS_POINTERS -DV8_ENABLE_SANDBOX
  COMMAND rm -f ${so-dir}/libv8.so
  COMMAND ln -s ${CMAKE_SOURCE_DIR}/${gn-dir}/libv8.so ${so-dir}/libv8.so
  COMMAND rm -f ${so-dir}/libv8_libbase.so
  COMMAND ln -s ${CMAKE_SOURCE_DIR}/${gn-dir}/libv8_libbase.so ${so-dir}/libv8_libbase.so
  COMMAND rm -f ${so-dir}/libv8_libplatform.so
  COMMAND ln -s ${CMAKE_SOURCE_DIR}/${gn-dir}/libv8_libplatform.so ${so-dir}/libv8_libplatform.so
)

add_custom_target(d8
  COMMAND ninja -C ${gn-dir} -v d8
  COMMAND rm -f ${so-dir}/libv8.so
  COMMAND ln -s ${CMAKE_SOURCE_DIR}/${gn-dir}/libv8.so ${so-dir}/libv8.so
  COMMAND rm -f ${so-dir}/libv8_libbase.so
  COMMAND ln -s ${CMAKE_SOURCE_DIR}/${gn-dir}/libv8_libbase.so ${so-dir}/libv8_libbase.so
  COMMAND rm -f ${so-dir}/libv8_libplatform.so
  COMMAND ln -s ${CMAKE_SOURCE_DIR}/${gn-dir}/libv8_libplatform.so ${so-dir}/libv8_libplatform.so
  COMMAND cmake . -DJS_SHELL=d8
)

add_custom_target(content-shell
  COMMAND ninja -C ${gn-dir} content-shell
  COMMAND rm -f ${so-dir}/libv8.so
  COMMAND ln -s ${CMAKE_SOURCE_DIR}/${gn-dir}/libv8.so ${so-dir}/libv8.so
  COMMAND rm -f ${so-dir}/libv8_libbase.so
  COMMAND ln -s ${CMAKE_SOURCE_DIR}/${gn-dir}/libv8_libbase.so ${so-dir}/libv8_libbase.so
  COMMAND rm -f ${so-dir}/libv8_libplatform.so
  COMMAND ln -s ${CMAKE_SOURCE_DIR}/${gn-dir}/libv8_libplatform.so ${so-dir}/libv8_libplatform.so
)

add_custom_target(v8-shell
  COMMAND ninja -C ${gn-dir} v8_shell
  COMMAND rm -f ${so-dir}/libv8.so
  COMMAND ln -s ${CMAKE_SOURCE_DIR}/${gn-dir}/libv8.so ${so-dir}/libv8.so
  COMMAND rm -f ${so-dir}/libv8_libbase.so
  COMMAND ln -s ${CMAKE_SOURCE_DIR}/${gn-dir}/libv8_libbase.so ${so-dir}/libv8_libbase.so
  COMMAND rm -f ${so-dir}/libv8_libplatform.so
  COMMAND ln -s ${CMAKE_SOURCE_DIR}/${gn-dir}/libv8_libplatform.so ${so-dir}/libv8_libplatform.so
  COMMAND cmake . -DJS_SHELL=v8_shell
)

add_custom_target(print-current-config
	COMMAND echo "==================== CONFIG"
	COMMAND echo "CHERI       ENABLED: ${CHERI_ENABLED}"
	COMMAND echo "SANDBOX     ENABLED: ${SANDBOX_ENABLED}"
	COMMAND echo "WEBASSEMBLY ENABLED: ${WEBASSEMBLY_ENABLED}"
	COMMAND echo "SHELL              : ${JS_SHELL}"
	COMMAND echo "===================="
)

add_custom_target(webassembly-off
	COMMAND cmake . -DSANDBOX_ENABLED="${SANDBOX_ENABLED}" -DCHERI_ENABLED="${CHERI_ENABLED}" -DWEBASSEMBLY_ENABLED=false
	COMMAND make print-current-config
)

add_custom_target(webassembly-on
	COMMAND cmake . -DSANDBOX_ENABLED="${SANDBOX_ENABLED}" -DCHERI_ENABLED="${CHERI_ENABLED}" -DWEBASSEMBLY_ENABLED=true
	COMMAND make print-current-config
)

add_custom_target(sandbox-off
	COMMAND cmake . -DSANDBOX_ENABLED=false -DCHERI_ENABLED="${CHERI_ENABLED}" -DWEBASSEMBLY_ENABLED="${WEBASSEMBLY_ENABLED}"
	COMMAND make print-current-config
)

add_custom_target(sandbox-on
	COMMAND cmake . -DSANDBOX_ENABLED=true -DCHERI_ENABLED="${CHERI_ENABLED}" -DWEBASSEMBLY_ENABLED="${WEBASSEMBLY_ENABLED}"
	COMMAND make print-current-config
)

add_custom_target(cheri-off
	COMMAND cmake . -DSANDBOX_ENABLED="${SANDBOX_ENABLED}" -DCHERI_ENABLED=false -DWEBASSEMBLY_ENABLED="${WEBASSEMBLY_ENABLED}"
	COMMAND make print-current-config
)

add_custom_target(cheri-on
	COMMAND cmake . -DSANDBOX_ENABLED="${SANDBOX_ENABLED}" -DCHERI_ENABLED=true -DWEBASSEMBLY_ENABLED="${WEBASSEMBLY_ENABLED}"
	COMMAND make print-current-config
)

add_custom_target(run
	COMMAND gdb --args ${gn-dir}/${JS_SHELL} --sandbox-fuzzing --no-maglev
)
