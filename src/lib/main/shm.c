#include "context.h"

#define MAX_KEY 1000000 // Maximum value for key, adjust as needed
#define MAGIC_NUMBER 5381

unsigned long djb2(const char *str)
{
    unsigned long hash = MAGIC_NUMBER;
    int c;
    while ((c = *str++))
    {
        hash = (((hash << 5) + hash) + c) % MAX_KEY;
    }
    return hash % MAX_KEY;
}

int createSharedMemory(int shmkey, int shmsz)
{
    int shmid = shmget((key_t)shmkey, shmsz, (0666 | IPC_CREAT));
    if (shmid < 0)
    {
        return -1;
    }
    return shmid;
}

void *attachSharedMemory(int shmid, int readOnly)
{
    return shmat(shmid, NULL, readOnly ? SHM_RDONLY : 0);
}

int detachSharedMemory(void *shmad)
{
    return shmdt(shmad);
}

int removeSharedMemory(int shmid)
{
    return shmctl(shmid, IPC_RMID, NULL);
}

int resizeSharedMemory(int shmid, int newShmsz, void *oldShmad)
{
    void *temp_buff = malloc(newShmsz);
    if (!temp_buff)
    {
        return -1;
    }

    memcpy(temp_buff, oldShmad, newShmsz);

    detachSharedMemory(oldShmad);
    removeSharedMemory(shmid);

    int newShmid = createSharedMemory(shmid, newShmsz);
    if (newShmid < 0)
    {
        free(temp_buff);
        return -1;
    }

    void *newShmad = attachSharedMemory(newShmid, 0);
    memcpy(newShmad, temp_buff, newShmsz);

    free(temp_buff);
    return newShmid;
}

int fep_shminit(FEP *fep)
{
    int shmkey;
    int shmid, shmsz;
    char *shmad;
    struct shmid_ds shmid_ds;
    MDARCH *arch;

    shmkey = (int)djb2(fep->exnm);

    switch (fep->whoami)
    {
    case MD_RDONLY:
        shmid = shmget((key_t)shmkey, 0, 0666);
        if (shmid < 0)
        {
            return -1;
        }
        shmad = attachSharedMemory(shmid, 1);
        break;

    case MD_RDWR:
        shmsz = sizeof(MDARCH) + (sizeof(FOLDER) * fep->config.settings.room);

        shmid = shmget((key_t)shmkey, 0, 0666);

        if (shmid < 0) // NEW
        {
            shmid = createSharedMemory(shmkey, shmsz);
            shmad = attachSharedMemory(shmid, 0);
            memset(shmad, 0x00, shmsz);
        }
        else
        {
            shmad = attachSharedMemory(shmid, 0);
            shmctl(shmid, IPC_STAT, &shmid_ds);

            if (shmid_ds.shm_segsz != shmsz) // RESIZE
            {
                shmid = resizeSharedMemory(shmid, shmsz, shmad);
                shmad = attachSharedMemory(shmid, 0);
            }
        }
        break;

    default:
        return -1;
    }

    fep->arch = shmad;
    fep->fold = &shmad[sizeof(MDARCH)];

    arch = (MDARCH *)fep->arch;

    arch->mrec = fep->config.settings.room;
    memcpy(&arch->config, &fep->config, sizeof(CONFIG));

    fep_log(fep, FL_MUST, GET_CALLER_FUNCTION(),
                "\n\n--- [%s] Shared Memory Info ---\n"
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
                fep->exnm,
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
                shmid_ds.shm_perm.__seq
        );

    return 0;
}
