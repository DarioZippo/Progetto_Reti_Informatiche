#include "./../include/vector.h"

int vectorTotal(vector *v)
{
    int totalCount = UNDEFINE;
    if(v)
    {
        totalCount = v->vectorList.total;
    }
    return totalCount;
}

//This function allocates the memory with a new size using realloc library function 
//and updates the tracking parameter with the new value.
int vectorResize(vector *v, int capacity)
{
    int status = UNDEFINE;
    if(v)
    {
        void **items = realloc(v->vectorList.items, sizeof(void *) * capacity);
        if (items)
        {
            v->vectorList.items = items;
            v->vectorList.capacity = capacity;
            status = SUCCESS;
        }
    }
    return status;
}

//This function inserts the data at the end of the vector. 
//If sufficient memory is not available then it will resize the memory.
int vectorPushBack(vector *v, void *item)
{
    int  status = UNDEFINE;
    if(v)
    {
        if (v->vectorList.capacity == v->vectorList.total)
        {
            status = vectorResize(v, v->vectorList.capacity * 2);
            if(status != UNDEFINE)
            {
                v->vectorList.items[v->vectorList.total++] = item;
            }
        }
        else
        {
            v->vectorList.items[v->vectorList.total++] = item;
            status = SUCCESS;
        }
    }
    return status;
}

//This function sets the data at the given index if the index is valid. 
//If you will pass the invalid index then it will do nothing.
int vectorSet(vector *v, int index, void *item)
{
    int  status = UNDEFINE;
    if(v)
    {
        if ((index >= 0) && (index < v->vectorList.total))
        {
            v->vectorList.items[index] = item;
            status = SUCCESS;
        }
    }
    return status;
}

//If the index is valid then this function will return the address of the data of the given index. 
//If the index is not valid then it will return NULL (null pointer). 
//You need to typecast the memory as per the data types.
void *vectorGet(vector *v, int index)
{
    void *readData = NULL;
    if(v)
    {
        if ((index >= 0) && (index < v->vectorList.total))
        {
            readData = v->vectorList.items[index];
        }
    }
    return readData;
}

//This function assigns the NULL to the given index and shifts all elements in vector by 1.
int vectorDelete(vector *v, int index)
{
    int  status = UNDEFINE;
    int i = 0;
    if(v)
    {
        if ((index < 0) || (index >= v->vectorList.total))
            return status;
        v->vectorList.items[index] = NULL;
        for (i = index; (i < v->vectorList.total - 1); ++i)
        {
            v->vectorList.items[i] = v->vectorList.items[i + 1];
            v->vectorList.items[i + 1] = NULL;
        }
        v->vectorList.total--;
        if ((v->vectorList.total > 0) && ((v->vectorList.total) == (v->vectorList.capacity / 4)))
        {
            vectorResize(v, v->vectorList.capacity / 2);
        }
        status = SUCCESS;
    }
    return status;
}

//This function deallocates the allocated memory.
int vectorFree(vector *v)
{
    int  status = UNDEFINE;
    if(v)
    {
        free(v->vectorList.items);
        v->vectorList.items = NULL;
        status = SUCCESS;
    }
    return status;
}

//In this function, I am initializing the capacity of the vector and function pointers with the appropriate function.
void vector_init(vector *v)
{
    //init function pointers
    v->pfVectorTotal = vectorTotal;
    v->pfVectorResize = vectorResize;
    v->pfVectorAdd = vectorPushBack;
    v->pfVectorSet = vectorSet;
    v->pfVectorGet = vectorGet;
    v->pfVectorFree = vectorFree;
    v->pfVectorDelete = vectorDelete;
    //initialize the capacity and allocate the memory
    v->vectorList.capacity = VECTOR_INIT_CAPACITY;
    v->vectorList.total = 0;
    v->vectorList.items = malloc(sizeof(void *) * v->vectorList.capacity);
}

/*
int main(int argc, char *argv[])
{
    int i =0;
    //init vector
    VECTOR_INIT(v);
    //Add data in vector
    v.pfVectorAdd(&v,"aticleworld.com\n");
    v.pfVectorAdd(&v,"amlendra\n");
    v.pfVectorAdd(&v,"Pooja\n");
    //print the data and type cast it
    for (i = 0; i < v.pfVectorTotal(&v); i++)
    {
        printf("%s", (char*)v.pfVectorGet(&v, i));
    }
    //Set the data at index 0
    v.pfVectorSet(&v,0,"Apoorv\n");
    printf("\n\n\nVector list after changes\n\n\n");
    //print the data and type cast it
    for (i = 0; i < v.pfVectorTotal(&v); i++)
    {
        printf("%s", (char*)v.pfVectorGet(&v, i));
    }
    return 0;
}
*/