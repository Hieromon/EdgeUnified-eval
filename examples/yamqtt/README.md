## Yet Another mqtt.ino

### Suggestion on how to write EdgeDriver source code files as pseudo-modules

This example suggests a way to write EdgeDriver source code files as pseudo discrete modules while still following the contexts of the One Definition Rule, the C++ standard.

The advantage of EdgeUnified is that the device IO control periphery with ESP8266 or ESP32 and the AutoConnect custom web page that provides the UI for it can be handled as a single independent component.

Your own device-dependent peripheral control and AutoConnect custom web page pair can be formed in a single C++ source code file, but you must follow the rules of the C++ standard when integrating them into one. In this case, each source code file is a C++ compilation unit, so simply including it in the main sketch will not compile successfully. So you end up applying separate compilation techniques to make use of your existing sketches. As a result, you have to provide a new header file that implements the required declarations and definitions and delegates their binding to external linkage.

The *yamqtt.ino* sketch suggests a way to write EdgeDriver source code files as pseudo discrete modules while still following the contexts of the One Definition Rule, the C++ standard. Although it has the appearance of a pseudo-module, it is different from the modules implemented by scripting language processors such as JavaScript and Python.

Essentially a module is a file that can share declarations and definitions between source files but does not leak macro definitions or private implementation details, providing safety assurance while avoiding name collisions. Even if modules are imported in a different order, they are safe and do not create undefined states.

But no such processing system is provided in Xtensa gcc tools set based on C++11. Since standard C++20 commits to module implementation. An indefinite module such as the one presented by this example has the following limitations:

1. Use namespace to limit the scope of definitions and declarations.
2. Exclude from namespace the global instances referenced by EdgeDeriver such as AutoConnect, WebServer, PubSubClient, etc. Therefore, they should not be redefined in other module source files or in the main source file.
3. The location of the module.hpp included in the main source file must be guaranteed to be forward referenced.
4. Depending on the order of inclusion, undefined declarations may occur.
