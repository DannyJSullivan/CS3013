#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define SIZE 64
#define MAXPROC 4
#define MAXPAGE 4

unsigned char memory[SIZE];

int pids[MAXPROC];

int freeList[MAXPROC];

int perm[MAXPROC][MAXPAGE];

int isValid[MAXPROC][MAXPAGE];

int isOnDisk[MAXPROC][MAXPAGE + 1];

FILE *disk;

int prevEvict = 0;

int getPage(int addr);
int getAddr(int page);
int toMem(int begin, char* val);
int toDisk(char page[16]);
int fromMem(int begin);
int fromDisk(char (*pageHolder)[16], int fromLine);
int convert(int pid, int virtAddr);
int makeTable(int pid);
int map(int pid, int virtAddr, int val);
int store(int pid, int virtAddr, int val);
int load(int pid, int virtAddr);
int evict(int pid);
int remap(int pid, int virtPage, int physPage);
int replace_page(int pid, int virtPage);
int swap(int page, int fromLine);
char** splitInput(char* string, const char toSplit);
int cmp(char a[], char b[]);


//getPage gets page from virtual address
int getPage(int addr){
    if(addr>=0 && addr<16){
        return 0;
    }
    else if(addr<32){
        return 1;
    }
    else if(addr<48){
        return 2;
    }
    else if(addr<64){
        return 3;
    }
    else{
        return -1;
    }
}

//getAddress gets address from page
int getAddr(int page){
    if(page==0){
        return 0;
    }
    else if(page==1){
        return 16;
    }
    else if(page==2){
        return 32;
    }
    else if(page==3){
        return 48;
    }
    else{
        return -1;
    }
}

//toMem writes to memory
int toMem(int begin, char* val){
    int bytes=strlen(val);
    int isRoom=begin%16;
    int count=0;

    if (isRoom+bytes >= 15){
        return -1;
    }
    for(int i=begin; i<16+begin; i++){
        if(val[i-begin] != 0){
            memory[i] = val[i - begin];
            count=i;
        }
        else{
            break;
        }
    }

    return (count - begin);
}

// Puts page in disk
int toDisk(char page[16])
{
    int line = -1;
    char currChar;
    int count = 0;

    disk = fopen("disk.txt", "r+");
    if(disk == NULL)
    {
        printf("ERROR: Cannot open disk in toDisk.\n");
        return -1;
    }

    do
    {
        currChar = fgetc(disk);
            count++;

        if(feof(disk))
        {
        	line++;
            for(int i = 0; i < 16; i++)
            {
                fputc(page[i], disk);
            }
            fputc('\n', disk);
            break;
        }
        else
        {
            if(currChar == '!' && count == 16)
            {
            	line++;
                fseek(disk, -16, SEEK_CUR);
                for(int i = 0; i < 16; i++)
                {
                    fputc(page[i], disk);
                }
                break;
            } else if(count == 16) {
                line++;
                count = 0;
            }
        }
    }
    while(currChar != EOF);

    fclose(disk);
    return line;
}

//fromMem reads from memory
int fromMem(int begin){
    int val;
    char buf[4];

    for(int i= 0; i<4; i++){
        if(memory[begin+i]!='*'&&i!=3){
            buf[i]=memory[begin+i];
        }
        else{
            buf[i]='\0';
            break;
        }
    }
    if(buf[0]=='\0'){
        return -1;
    }
    else{
        sscanf(buf, "%d", &val);
        return val;
    }
}

// Gets page from disk
int fromDisk(char (*pageHolder)[16], int fromLine)
{
    int line = -1;
    char currChar;
    int count = 0;

    disk = fopen("disk.txt", "r+");
    if(disk == NULL)
    {
    printf("ERROR: Cannot open disk in fromDisk.\n");
        return -1;
    }

    do
    {
        currChar = fgetc(disk);
            count++;

        if(feof(disk) && line == -1)
        {
            printf("ERROR: Cannot get page from empty disk.\n");

            fclose(disk);
            return -1;
        }
        else
        {
            if(count == 16)
            {
                line++;
            }
            if(line == fromLine && count == 16)
            {
                fseek(disk, -16, SEEK_CUR);
                for(int i = 0; i < 16; i++)
                {
                    currChar = fgetc(disk);
                    (*pageHolder)[i] = currChar;
                    fseek(disk, -1, SEEK_CUR);
                    fputc('!', disk);
                }
                fputc('\n', disk);

                fclose(disk);
                return 0;
            }
            else if(count > 16)
            {
                count = 0;
            }
        }
    }
    while(currChar != EOF);

    fclose(disk);
    return -1;
}

//convert converts virtual address to physical address, returns physical
int convert(int pid, int virt){
    int phys=-1;
    int virtPage=getPage(virt);
    int offset=virtPage*16;
    int begin=pids[pid];
    int cur=begin;//index position

    for (int i=0; i<16; i++) {
        if(memory[cur]==','){
            if(memory[cur-1]-'0'==getPage(virt)){
                return getAddr(memory[cur+1]-'0') + virt - offset;
            }
        }
        cur++;
    }
    return phys;
}

//makeTable creates table of virtual pages
int makeTable(int pid){
    int isFree = -1;

    for(int i=0; i<4; i++){
        if (freeList[i]==-1){
            freeList[i] = 0;
            pids[pid] = getAddr(i); // Put physical address into pid register
            isFree = 1;
            printf("Put page table for PID %d into physical frame %d\n", pid, i);
            break;
        }
    }
    if(isFree==-1){
        int evicted=evict(pid);
        swap(evicted, -1);
        int physPage=prevEvict;
        pids[pid]=getAddr(physPage);
        printf("Put page table for PID %d into physical frame %d\n", pid, physPage);
    }
}

//map function
int map(int pid, int virtAddr, int val){
    int virtPage=getPage(virtAddr);
    int physPage;
    int table=pids[pid];
    int isFree=-1;
    char fullAddr[16]="";
    char newEntry[10];
    char buff[10];

    //if no page table, make one
    if (table==-1&&isOnDisk[pid][0]==-1){
        makeTable(pid);
    }

    //if page already there and needs to update
    if (isValid[pid][virtPage]==1){
        if (perm[pid][virtPage] == val){
            printf("ERROR: virtual page %d is already mapped with rw_bit=%d\n", virtPage, val);
        } 
        else {
            printf("Updating permissions for virtual page %d (frame %d)\n", virtPage, getPage(convert(pid, getAddr(virtPage)))); //TODO
            perm[pid][virtPage] = val;
        }
    }
    else{
        for(int i=0; i<4; i++){
            if (freeList[i]==-1){
                freeList[i]=0;
                perm[pid][virtPage] = val;
                isValid[pid][virtPage] = 1;
                isFree=1;
                physPage=i;
                int writeTo = pids[pid];

                for(int j=0; j<16; j++){
                    if (memory[writeTo]=='*'){
                        sprintf(buff, "%d", virtPage);
                        strcat(fullAddr, buff);
                        strcat(fullAddr, ",");
                        sprintf(buff, "%d", physPage);
                        strcat(fullAddr, buff);
                        writeTo+=toMem(writeTo, fullAddr);
                        break;
                    }
                    writeTo++;
                }
                printf("Mapped virtual address %d (page %d) into physical frame %d\n", virtAddr, virtPage, physPage);
                break;
            }
        }
        if (isFree==-1){
            int evicted=evict(pid);
            swap(evicted, -1);
            perm[pid][virtPage]=val;
            isValid[pid][virtPage]=1;
            isFree=1;
            physPage=prevEvict;
            int writeTo=pids[pid];

            for(int j=0; j<16; j++){
                if(memory[writeTo] == '*'){
                    sprintf(buff, "%d", virtPage);
                    strcat(fullAddr, buff);
                    strcat(fullAddr, ",");
                    sprintf(buff, "%d", physPage);
                    strcat(fullAddr, buff);
                    writeTo+=toMem(writeTo, fullAddr);
                    break;
                }
                writeTo++;
            }
            printf("Mapped virtual address %d (page %d) into physical frame %d\n", virtAddr, virtPage, physPage);
        }
    }

    return 0;
}

//remap changes address when address is swapped
int remap(int pid, int virtPage, int physPage){
    int isFree=-1;
    int writeTo = pids[pid];
    char fullAddr[16]="";
    char buff[10];

    for(int i=0; i<16; i++){
        if(memory[writeTo]==','){
            if(memory[writeTo-1]==virtPage){
                memory[writeTo+1]=physPage;
            }
        }
        writeTo++;
    }

    printf("Remapped virtual page %d into physical frame %d\n", virtPage, physPage);
    return 0;
}

//store function
int store(int pid, int virtAddr, int val){
    char buff[10] = "";

    if (isOnDisk[pid][0]!=-1){
        int evicted=evict(pid);
        swap(evicted, isOnDisk[pid][0]);
    }
    int physAddr=convert(pid, virtAddr);
    int virtPage= getPage(virtAddr);

    if (perm[pid][virtPage]==1){
        if (isValid[pid][virtPage]==1){
            if (isOnDisk[pid][virtPage+1]!=-1){
                int evicted=evict(pid);
                swap(evicted, isOnDisk[pid][virtPage+1]);
            }
            sprintf(buff, "%d", val);
            int bytesWritten=toMem(physAddr, buff);
            if(bytesWritten==-1){
                printf("ERROR: Too many bytes written, value cannot be stored.\n");
            }else{
                printf("Stored value %d at virtual address %d (physical address %d)\n", val, virtAddr, physAddr);
            }
        }else{
            printf("ERROR: Virtual page %d has not been allocated for process %d!\n", virtPage, pid);
        }
    }else{
        printf("ERROR: Do not have permission to write on this page.\n");
    }

    return 0;
}

//load function
int load(int pid, int virtAddr) {
    int virtPage = getPage(virtAddr);
    
    //printf("On disk for pid %d: %d\n", pid, isOnDisk[pid][0]);

    if (isOnDisk[pid][0] != -1) {
        int evicted=evict(pid);
        swap(evicted, isOnDisk[pid][0]);
        isOnDisk[pid][0]=-1;
    }

    //printf("On disk for virtPage %d: %d\n", virtPage, isOnDisk[pid][virtPage + 1]);

    if(isOnDisk[pid][virtPage+1]!=-1){
        int evicted=evict(pid);
        swap(evicted, isOnDisk[pid][virtPage+1]);
    }

    int physAddr=convert(pid, virtAddr);
    int val=fromMem(physAddr);

    if(val==-1){
        printf("ERROR: No value stored at virtual address %d (physical address %d)\n", virtAddr, physAddr);
    }
    else{
        printf("The value %d is virtual address %d (physical address %d)\n", val, virtAddr, physAddr);
    }

    return 0;
}

//evicts physical page based on round robin
int evict(int pid){
    int table=getPage(pids[pid]);
    int nextEvict=prevEvict+1;

    if (nextEvict>=4){
        nextEvict=0;
    }
    if (nextEvict==table){
        nextEvict++;
        if (nextEvict>=4){
            nextEvict=0;
        }
    }
    prevEvict = nextEvict;
    return nextEvict;
}

//swap from memory to disk
int swap(int page, int fromLine){
    int begin=getAddr(page);
    char putTemp[16];
    char getTemp[16];
    int replaceMem=-1;
    int putLine=-1;
    int ptableFlag=-1;

    for(int i=0; i<MAXPROC; i++){
        if(begin==pids[i]){
            pids[i]=-1;
            ptableFlag=i;
            break;
        }
    }

    for(int i=0; i<16; i++){
        putTemp[i]=memory[begin+i];
    }

    if(fromLine != -1){
        replaceMem=fromDisk(&getTemp, fromLine);
        freeList[page] = 0;
    }

    putLine = toDisk(putTemp);

    if(putLine==-1){
        printf("ERROR: Could not put page to disk.\n");
        return -1;
    }
    else if(replaceMem!=-1) {
    	printf("Swapped disk slot %d into frame %d\n", fromLine, getPage(begin)); //TODO
        for(int i = 0; i < 16; i++){
            memory[begin+i]=getTemp[i];
        }
    }
    else{
        for(int i = 0; i < 16; i++){
            memory[begin+i]='*';
        }
    }

    if(fromLine!=-1){
        for(int i = 0; i<MAXPROC; i++){
            for(int j=0; j<MAXPROC+1; j++){
                if(isOnDisk[i][j]==fromLine){
                    isOnDisk[i][j]=-1;
                    if(j!=0){
                        remap(i, j-1, page);
                    }
                    else if(j==0&&ptableFlag==-1){
                        pids[i] = begin;
                    }
                    break;
                }
            }
        }
    }

    if(ptableFlag != -1){
        int rmPID = -1;
        int rmVirtPage = -1;

        for (int i=0; i<MAXPROC; i++){
            if(pids[i]!=-1){
                int curAddr=pids[i];
                for (int j=0; j<16; j++){
                    printf("%c", memory[curAddr]);
                    if(memory[curAddr]==','){
                        if(memory[curAddr+1]-'0'==page){
                            rmPID=i;
                            rmVirtPage=memory[curAddr-1]-'0';
                        }
                    }
                    curAddr++;
                }
            }
    	}

	    int curTable = -1;

	    if (rmPID == -1){
	        for (int i = 0; i < MAXPROC; i++){
	            curTable=isOnDisk[i][0];
	            if (curTable != -1){
	                replaceMem = fromDisk(&getTemp, curTable);

	                for (int j = 0; j < 16; j++){
	                    if (getTemp[j] == ','){
	                        if(getTemp[j + 1] - '0' == page){
	                            rmPID = j;
	                            rmVirtPage = getTemp[j - 1] - '0';
	                        }
	                    }
	                }
	            }
	            if (rmPID != -1) break;
	        }
	    }
	    if(rmPID != -1 && rmVirtPage != -1){
	        isOnDisk[rmPID][rmVirtPage + 1] = putLine;
	    }
	    else{
	        printf("ERROR: Page that is swapped out doesn't exist?\n");
	    }
    }
    else{
    isOnDisk[ptableFlag][0]=putLine;  
    }
    printf("Swapped frame %d to disk at swap slot %d\n", page, putLine);
    if (fromLine!=-1){
        printf("Swapped disk slot %d into frame %d\n", fromLine, page);
    }
    if (ptableFlag!=-1){
        printf("Put page table for PID %d into swap slot %d\n", ptableFlag, putLine);
    }
    return putLine;
}

//function to split user input
char** splitInput(char* string, const char toSplit){
    char** final=0;
    size_t size=0;
    char* temp=string;
    char* prevComma=0;
    char delim[1];
    delim[0]=toSplit;

    while(*temp){
        if(toSplit==*temp){
            size++;
            prevComma=temp;
        }
        temp++;
    }

    size+=prevComma<(string+strlen(string)-1);

    size++;

    final=malloc(sizeof(char*)*size);

    if(final){
        size_t i=0;
        char* token=strtok(string, delim);

        while(token){
            assert(i<size);
            *(final+(i++))=strdup(token);
            token=strtok(0,delim);
        }
        assert(i==size-1);
        *(final+i)=0;
    }
    return final;
}

//compare two strings
int cmp(char a[], char b[]){
    for(int i=0; a[i]!='\0'||b[i]!='\0'; i++){//compare elements if not NULL
        if(a[i]!=b[i]){//if chars don't match, set false
            return 0;
        }
    }
    return 1;
}


// Main
int main(int argc, char *argv[])
{
    //create temporary array for stripped input
    char** temp;

    //create strings for user input
    char* input=(char*)malloc(sizeof(char*)); //gets user input
    char* instruct=(char*)malloc(sizeof(char*)); //function chosen by user

    int pid = 0; // Process ID
    int inst_type = 0; // Instruction type
    int virtAddr = 0; // Virtual address
    //int input = 0; // Value
    int is_end = 0; // Boolean for ending simulation

    char buff[20]; // Holds stdin buff
    char cmd_seq[20]; // The command sequence read from stdin
    char* cmd_array[4]; // Holds the commands read from file
    char* token;

    // Clean disk
    disk = fopen("disk.txt", "w");
    if(disk == NULL)
    {
    printf("ERROR: Cannot open disk in main.");
    return -1;
    }
    else
        fclose(disk);

    // Initialize ptable, free and write lists
    for (int i = 0; i < MAXPROC; i++)
    {
        pids[i] = -1;
        freeList[i] = -1;
        for (int j = 0; j < MAXPAGE + 1; j++)
        {
            if (j < 4)
            {
                perm[i][j] = 0;
                isValid[i][j] = 0;
            }
            isOnDisk[i][j] = -1;
        }
    }

    // Initialize physical memory
    for (int i = 0; i < SIZE; i++)
    {
        memory[i] = '*';
    }

    while(1){
        //prompt instructions
        printf("Instruction? "); 

        //get instructions
        if(fgets(input, 15, stdin)==NULL) {
            printf("End of file. Exiting.\n");
            exit(1);
        }
        printf("\n");

        //split input at every comma
        temp=splitInput(input, ',');

        //assign input to vars
        int pid=atoi(temp[0]); //process id user input in range [0,3]
        instruct=temp[1]; //instruction from user input
        int virt=atoi(temp[2]); //virtual address user input in range [0,63]
        int value=atoi(temp[3]); //depends on instruct, int in range [0,255]

        //print out what is being done
        if(cmp("map", instruct)){
            map(pid, virt, value);
            printf("\n");
        }
        else if(cmp("store", instruct)){
            store(pid, virt, value);
            printf("\n");
        }
        else if(cmp("load", instruct)){
            load(pid, virt);
            printf("\n");
        }
    }
}