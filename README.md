# Operating_System
Ubuntu 18.04

C
## Simple pstree

### Objectives
- Understand how to transfer information between the kernel and the user
space processes

- Understand the PCB in the kernel and its related macros
- Understand the linked list in the kernel

![](https://i.imgur.com/pboSlMV.png)

![](https://i.imgur.com/yN9y7cq.png)

### References

![image](https://myaut.github.io/dtrace-stap-book/images/linux/task.png)

資料來源(https://myaut.github.io/dtrace-stap-book/kernel/proc.html)

![image](http://sop.upv.es/gii-dso/en/t3-procesos-en-linux/parent_relationship.png)

資料來源(http://sop.upv.es/gii-dso/en/t3-procesos-en-linux/gen-t3-procesos-en-linux.html)

struct task_struct https://elixir.bootlin.com/linux/latest/source/include/linux/sched.h#L592

------------------------------------------------------------------

## Simple memory allocator

![](https://i.imgur.com/4Qc1b9X.png)
