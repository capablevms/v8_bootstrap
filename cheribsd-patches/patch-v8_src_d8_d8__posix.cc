--- v8/src/d8/d8-posix.cc.orig	2024-02-09 10:55:56.804993000 +0000
+++ v8/src/d8/d8-posix.cc	2024-02-26 12:37:00.177453000 +0000
@@ -2,8 +2,12 @@
 // Use of this source code is governed by a BSD-style license that can be
 // found in the LICENSE file.
 
+#include <sys/types.h>
 #include <errno.h>
 #include <fcntl.h>
+#ifdef __FreeBSD__
+#include <netinet/in.h>
+#endif
 #include <netinet/ip.h>
 #include <signal.h>
 #include <stdlib.h>
@@ -12,7 +16,6 @@
 #include <sys/socket.h>
 #include <sys/stat.h>
 #include <sys/time.h>
-#include <sys/types.h>
 #include <sys/wait.h>
 #include <unistd.h>
 
