--- base/metrics/crc32.cc.orig	2024-12-12 11:09:44.248138000 +0000
+++ base/metrics/crc32.cc	2024-12-12 11:06:23.181056000 +0000
@@ -4,13 +4,13 @@
 
 #include "base/metrics/crc32.h"
 
-#if !BUILDFLAG(IS_NACL)
+#if !BUILDFLAG(IS_NACL) && !(defined(CHERI_HYBRID) || V8_OS_FREEBSD)
 #include "third_party/zlib/zlib.h"
 #endif  // !BUILDFLAG(IS_NACL)
 
 namespace base {
 
-#if !BUILDFLAG(IS_NACL)
+#if !BUILDFLAG(IS_NACL) && !(defined(CHERI_HYBRID) || V8_OS_FREEBSD)
 uint32_t Crc32(uint32_t sum, span<const uint8_t> data) {
   if (data.empty()) {
     return sum;
