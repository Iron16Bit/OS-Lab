#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

/*
Implementare in C una coda di messaggi basata sulla priorità, che consenta di
aggiungere messaggi con livelli di priorità diversi e di eliminarli in base al loro livello
di priorità.
*/

struct msg_buffer
{
    long mtype;
    char mtext[100];
};

void pushMsg(int queueId, char *msg, long priority);
struct msg_buffer getHighestPriorityMsg(int queueId);
struct msg_buffer getMsgWithPriority(int queueId, long priority);

int main(void)
{
    remove("/tmp/unique"); // First delete the file to clean it. Is it actually needed?
    creat("/tmp/unique", 0777);

    key_t queue1Key = ftok("/tmp/unique", 1);          // Get queue key
    int queueId = msgget(queue1Key, 0777 | IPC_CREAT); // Create queue

    //----------------------------------------------------------------------------------

    if (fork())
    {
        // The child will push several msgs to the queue
        pushMsg(queueId, "Raffreddore", __LONG_MAX__);
        pushMsg(queueId, "Infarto", 1);
        pushMsg(queueId, "Gamba rotta", 5);
        pushMsg(queueId, "Febbre alta", 7);
    }
    else
    {
        wait(NULL);
        struct msg_buffer received_msg;
        received_msg = getMsgWithPriority(queueId, __LONG_MAX__);
        printf("\n Received: %s\n  Priority: %ld\n", received_msg.mtext, received_msg.mtype);

        int tmp = 5;
        while (tmp > 0)
        {
            received_msg = getHighestPriorityMsg(queueId);
            printf("\n Received: %s\n  Priority: %ld\n", received_msg.mtext, received_msg.mtype);
            tmp -= 1;
        }
    }

    return 0;
}

// Push msgs to queue
void pushMsg(int queueId, char *msg, long priority)
{
    struct msg_buffer tobe_pushed;
    tobe_pushed.mtype = priority;
    strcpy(tobe_pushed.mtext, msg);
    int esito = msgsnd(queueId, &tobe_pushed, sizeof(tobe_pushed.mtext), 0);
}

// Get msg with highest priority
struct msg_buffer getHighestPriorityMsg(int queueId)
{
    struct msg_buffer received_msg;
    int esito = msgrcv(queueId, &received_msg, sizeof(received_msg.mtext), 0, 0);
    return received_msg;
}

// Gets msg with specified priority
struct msg_buffer getMsgWithPriority(int queueId, long priority)
{
    struct msg_buffer received_msg;
    int esito = msgrcv(queueId, &received_msg, sizeof(received_msg.mtext), priority, 0);
    return received_msg;
}