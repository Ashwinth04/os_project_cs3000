typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef uint pde_t;
struct ancestor_info {
    int pid;           // Process ID
    int parent_pid;    // Parent Process ID
    char name[16];     // Process name (optional)
};