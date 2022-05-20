#include <iostream>
#include <windows.h>

void addrInput(unsigned char* *lowAddr, unsigned char* *highAddr, const unsigned char* lowBound, const unsigned char* highBound);

unsigned char* addrRoundDown (unsigned char * addr, DWORD pageSize);

void currentPageInfo (MEMORY_BASIC_INFORMATION memBasInfo);

int main() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    auto pageSize = sysInfo.dwPageSize;
    auto lowBound = sysInfo.lpMinimumApplicationAddress;
    auto highBound = sysInfo.lpMaximumApplicationAddress;

    unsigned char* lowAddr;
    unsigned char* highAddr;
    addrInput(&lowAddr, &highAddr, (unsigned char*)lowBound, (unsigned char*)highBound);
    lowAddr = addrRoundDown(lowAddr, pageSize);

    MEMORY_BASIC_INFORMATION memBasInfo{};

    printf("Legend:\n"
           "        State:  Free, Reserved, Committed;\n"
           "        Access: Readable, Writable, eXecutable, Copy-on-write, Guard,\n"
           "                No cache, write combine (Q), no access (-------);\n"
           "                empty space indicates no access to page protection info.\n\n");
    printf("Address \t   Size\tUsed\tState\tAccess\tType\n");

    for (unsigned char* currentAddr = lowAddr; currentAddr < highAddr; currentAddr+=memBasInfo.RegionSize)
    {
        VirtualQuery((void*)currentAddr, &memBasInfo, sizeof memBasInfo);
        currentPageInfo(memBasInfo);
    }
    system("pause");
    return 0;
}

void addrInput(unsigned char* *lowAddr, unsigned char* *highAddr, const unsigned char* lowBound, const unsigned char* highBound)
{
    while (true)
    {
        printf("Enter address range boundaries (acceptable range of values [0x%x; 0x%x]): ", lowBound, highBound);
        if (scanf("%x %x", lowAddr, highAddr) != 2)
        {
            while (getchar() != '\n')
                continue;
            printf("Wrong input! The input must contain 2 hexidecimal addresses in range [0x%x; 0x%x]!\n", lowBound, highBound);
        }
        else if (*lowAddr < lowBound)
        {
            printf("Address range lower boundary is too small! It must be at least 0x%x!\n", lowBound);
        }
        else if (*lowAddr > highBound)
        {
            printf("Address range lower boundary is too high! It must be less than 0x%x!\n", highBound);
        }
        else if (*highAddr < lowBound)
        {
            printf("Address range higher boundary is too small! It must be more than 0x%x!\n", lowBound);
        }
        else if (*highAddr > highBound)
        {
            printf("Address range higher boundary is too high! It must be less or equal 0x%x!\n", highBound);
        }
        else if (*lowAddr > *highAddr)
        {
            printf("Address range lower boundary must be less than higher boundary!\n");
        }
        else
        {
            break;
        }

        printf("Try again!\n\n");
    }
}

unsigned char* addrRoundDown (unsigned char * addr, DWORD pageSize)
{
    return (unsigned char*)((unsigned long)addr & ~(pageSize - 1));
}

void currentPageInfo (MEMORY_BASIC_INFORMATION memBasInfo)
{
    printf("0x%-10x %10ld\t", memBasInfo.BaseAddress, memBasInfo.RegionSize/1024);
    printf("%3s\t  ", memBasInfo.State==MEM_FREE ? "no" : "yes");
    char stateRes;
    switch (memBasInfo.State)
    {
        case MEM_FREE:
            stateRes = 'F';
            break;
        case MEM_COMMIT:
            stateRes = 'C';
            break;
        case MEM_RESERVE:
            stateRes = 'R';
            break;
        default:
            stateRes = '-';
    }
    printf("%c\t", stateRes);

    bool noAccess = false;
    char protectRes[7];
    switch (memBasInfo.AllocationProtect)
    {
        case PAGE_EXECUTE:
            strcpy(protectRes, "--X-");
            break;
        case PAGE_EXECUTE_READ:
            strcpy(protectRes, "R-X-");
            break;
        case PAGE_EXECUTE_READWRITE:
            strcpy(protectRes, "RWX-");
            break;
        case PAGE_EXECUTE_WRITECOPY:
            strcpy(protectRes, "R-XC");
            break;
        case PAGE_NOACCESS:
            strcpy(protectRes, "----");
            break;
        case PAGE_READONLY:
            strcpy(protectRes, "R---");
            break;
        case PAGE_READWRITE:
            strcpy(protectRes, "RW--");
            break;
        case PAGE_WRITECOPY:
            strcpy(protectRes, "R--C");
            break;
        default:
            strcpy(protectRes, "    ");
            noAccess = true;
            break;
    }

    if (!noAccess)
    {
        strcat(protectRes, (memBasInfo.Protect & PAGE_GUARD) ? "G" : "-");
        strcat(protectRes, (memBasInfo.Protect & PAGE_NOCACHE) ? "N" : "-");
        strcat(protectRes, (memBasInfo.Protect & PAGE_WRITECOMBINE) ? "Q" : "-");
    }
    else
    {
        strcat(protectRes, "   ");
    }
    printf("%s\t", protectRes);

    char typeRes[7];
    switch (memBasInfo.Type)
    {
        case MEM_IMAGE:
            strcat(typeRes, "image");
            break;
        case MEM_MAPPED:
            strcat(typeRes, "mapped");
            break;
        case MEM_PRIVATE:
            strcat(typeRes, "private");
            break;
        default:
            strcat(typeRes, "unused");
            break;
    }
    printf("%s\n", typeRes);
}
