--- third_party/zlib/crc32_simd.c.orig	2024-12-12 14:35:33.845445000 +0000
+++ third_party/zlib/crc32_simd.c	2024-12-12 14:59:19.409647000 +0000
@@ -387,7 +387,7 @@
 #endif
 
 #if defined(__aarch64__)
-#define TARGET_ARMV8_WITH_CRC __attribute__((target("arch=armv8-a+aes+crc")))
+#define TARGET_ARMV8_WITH_CRC __attribute__((target("crc,aes")))
 #else  // !defined(__aarch64__)
 #define TARGET_ARMV8_WITH_CRC __attribute__((target("crc")))
 #endif  // defined(__aarch64__)
@@ -398,7 +398,7 @@
  */
 #include <arm_acle.h>
 #include <arm_neon.h>
-#define TARGET_ARMV8_WITH_CRC __attribute__((target("arch=armv8-a+crc")))
+#define TARGET_ARMV8_WITH_CRC __attribute__((target("arch=armv8-a+crc+crypto")))
 #else  // !defined(__GNUC__) && !defined(_aarch64__)
 #error ARM CRC32 SIMD extensions only supported for Clang and GCC
 #endif
