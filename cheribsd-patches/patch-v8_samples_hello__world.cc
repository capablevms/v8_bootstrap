--- v8/samples/hello-world.cc.orig	2024-02-09 10:55:56.533951000 +0000
+++ v8/samples/hello-world.cc	2024-03-05 11:50:23.759381000 +0000
@@ -56,7 +56,8 @@
       printf("%s\n", *utf8);
     }
 
-    {
+    /* *** TW 240305 COMMENTED OUT because at the moment we don't have webassembly enabled.
+     {
       // Use the JavaScript API to generate a WebAssembly module.
       //
       // |bytes| contains the binary format for the following module:
@@ -93,6 +94,7 @@
       uint32_t number = result->Uint32Value(context).ToChecked();
       printf("3 + 4 = %u\n", number);
     }
+    */
   }
 
   // Dispose the isolate and tear down V8.
