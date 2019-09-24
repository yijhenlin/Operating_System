#include "lib/hw_malloc.h"
#include "hw4_mm_test.h"


int main(int argc, char *argv[])
{

    while(scanf("%s",opt)!=EOF) {
        if(!strcmp(opt,"alloc")) {
            scanf("%d",&bytes);
            mem = hw_malloc(bytes);
            if(mem == NULL)printf("malloc failed\n");
            else if(bytes + 24 <= mmap_threshold)printf("%014p\n",mem-get_start_sbrk());
            else printf("%014p\n",mem);
        } else if(!strcmp(opt,"free")) {
            scanf("%p",&mem);
            if((long long int)mem < 64*1024) {
                mem = mem+(long long int)get_start_sbrk()-24;
            } else mem = mem-24;
            printf("%s\n",hw_free(mem) == 1 ?"success" : "fail");
        } else if(!strcmp(opt,"print")) {
            getchar();
            if(getchar() == 'b') {
                scanf("in[%d]",&num);
                if(num >= 0 && num <= 10)print_bin(num);
            } else if(getchar() == 'l') {
                scanf("%s",printwho);
                print_mmap(0);
            } else {
                scanf("%s",printwho);
                print_mmap(1);
            }
        } else if(!strcmp(opt,"exit")) {
            return 0;
        } else {
            printf("input error\n");
        }
    }
    return 0;
}
