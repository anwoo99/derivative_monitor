#include "context.h"

char *whoami;

void usage(char *who)
{
    if (isatty(1))
        printf("usage : %s exchange_name\n", who);
    exit(-1);
}

int confirmAction(const char *message)
{
    char answer[32];

    printf("%s [Y/N]: ", message);
    scanf("%s", answer);
    printf("\n");

    switch (answer[0])
    {
    case 'Y':
    case 'y':
        return 1;
    case 'N':
    case 'n':
        return 0;
    default:
        printf("%s: [Y/N] 중 선택하세요.\n", whoami);
        return confirmAction(message);
    }
}

int main(int argc, char **argv)
{
    struct shmid_ds shmid_ds;
    char exnm[32];
    int shmkey, shmid;
    char inquiry[128];
	
    whoami = basename(argv[0]);

    if (argc < 2)
        usage(whoami);

    strcpy(exnm, argv[1]);

    shmkey = (int)djb2(exnm);
    shmid = shmget((key_t)shmkey, 0, 0666);

    if (shmid < 0)
    {
        printf("%s: '%s' 거래소에 할당된 공유메모리가 없습니다..\n\n", whoami, exnm);
        exit(0);
    }

    if (shmctl(shmid, IPC_STAT, &shmid_ds) == 0)
    {
        printf("\n\n--- [%s] Shared Memory Info ---\n"
               "- Segment Size: %lu bytes\n"
               "- Last Access Time: %s"
               "- Last Detach Time: %s"
               "- Last Change Time: %s"
               "- Creator Process ID: %d\n"
               "- Last Process ID: %d\n"
               "- Number of Attachments: %d\n"
               "- Key: %d\n"
               "- UID of Owner: %d\n"
               "- GID of Owner: %d\n"
               "- UID of Creator: %d\n"
               "- GID of Creator: %d\n"
               "- Permissions: %o\n"
               "- Sequence Number: %u\n"
               "-------------------------------\n",
               exnm,
               shmid_ds.shm_segsz,
               ctime(&shmid_ds.shm_atime),
               ctime(&shmid_ds.shm_dtime),
               ctime(&shmid_ds.shm_ctime),
               shmid_ds.shm_cpid,
               shmid_ds.shm_lpid,
               shmid_ds.shm_nattch,
               shmid_ds.shm_perm.__key,
               shmid_ds.shm_perm.uid,
               shmid_ds.shm_perm.gid,
               shmid_ds.shm_perm.cuid,
               shmid_ds.shm_perm.cgid,
               shmid_ds.shm_perm.mode,
               shmid_ds.shm_perm.__seq);

	sprintf(inquiry, "%s: '%s' 거래소의 공유 메모리가 삭제 됩니다. 진행하시겠습니까?", whoami, exnm);

        if (!confirmAction(inquiry))
            exit(0);
            
        if (shmctl(shmid, IPC_RMID, &shmid_ds) == 0)
            printf("%s: '%s' 거래소의 공유 메모리가 삭제되었습니다.\n", whoami, exnm);
    }

    return 0;
}
