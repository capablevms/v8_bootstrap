--- v8/include/v8config.h.orig	2024-12-12 11:50:11.493645000 +0000
+++ v8/include/v8config.h	2024-12-12 12:04:06.408765000 +0000
@@ -9,7 +9,7 @@
 // 201703L" for its experimental -std=gnu++2a config.
 // TODO(leszeks): Change to `__cplusplus <= 202002L` once we only support
 // compilers with full C++20 support.
-#if __cplusplus <= 201703L
+#if __cplusplus <= 201703L && !(V8_OS_FREEBSD || defined(CHERI_HYBRID))
 #error "C++20 or later required."
 #endif
 
