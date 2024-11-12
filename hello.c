#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
    struct ancestor_info ancestors[10];
    int count;

    int p1 = fork();
    if(p1 > 0)
    {
        int p2 = fork();
        if(p2 > 0)
        {
            count = get_process_ancestors(ancestors, 10);


            if(count < 0) {
                printf(2, "get_process_ancestors failed\n");
                exit();
            }

            printf(1, "Process Ancestor Chain:\n");
            for(int i = 0; i < count; i++) {
                printf(1, "PID: %d, Parent PID: %d, Name: %s\n",
                    ancestors[i].pid,
                    ancestors[i].parent_pid,
                    ancestors[i].name);
            }
        }
    }
    
    
    
    exit();
}