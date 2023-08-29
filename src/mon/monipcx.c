#include "context.h"

void usage()
{
    if (isatty(1))
        printf("usage : monipcx exchange_name\n");
    exit(-1);
}

int confirmAction(const char *message)
{
    char answer[32];

    printf("%s [Y/N]: ", message);
    scanf("%s", answer);

    switch (answer[0])
    {
        case 'Y':
        case 'y':
            return 1;
        case 'N':
        case 'n':
            return 0;
        default:
            printf("\n [Y/N] 중 선택하세요.\n");
            return confirmAction(message);
    }
}

int main(int argc, char **argv)
{
    struct shmid_ds shmid_ds;
    char exnm[32];
    int shmkey, shmid;

    if (argc < 2)
        usage();

    strcpy(exnm, argv[1]);

    if (!confirmAction("'%s' 거래소의 전체 메모리가 초기화 됩니다. 진행하시겠습니까?"))
        exit(0);

    shmkey = (int)djb2(exnm);
    shmid = shmget((key_t)shmkey, 0, 0666);

    if (shmid < 0)
    {
        printf("'%s'에 할당된 공유메모리가 없습니다..\n");
        exit(0);
    }

    if (shmctl(shmid, IPC_STAT, &shmid_ds) == 0)
    {
        if (shmctl(shmid, IPC_RMID, &shmid_ds) == 0)
            printf("'%s' 거래소의 공유 메모리가 삭제되었습니다.\n");
    }

    return 0;
}
