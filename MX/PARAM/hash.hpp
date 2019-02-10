#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/**
 * @code
 * int main()
 * {
 *  Hash a(128, sizeof(int));
 *  int data = 10;
 *  
 *  a.insert("Hello", &data);
 *  a.insert("World", &data);
 *  
 *  int ans;
 * 
 *  a.getData("Hello", &ans);
 *  assert(ans == data);
 *  a.getData("World", &ans);
 *  assert(ans == data);
 * 
 *  a.remove("Hello");
 *  assert(a.isHas("Hello") == false);
 *  a.clearDeleted();
 *  return 0;
 * }
 * @endcode
 */

class Hash
{
    typedef struct _it_t
    {
        char *key;
        void *data;
        enum
        {
            empty,
            full,
            deleted
        } flag;
    } it_t;

    it_t *arr;
    int num;
    size_t size;

  public:
    Hash(int maximum, size_t itSize)
    {
        num = maximum;
        size = itSize;
        arr = static_cast<it_t *>(malloc(num * sizeof(*arr)));
        for (auto i = 0; i < num; i++)
        {
            arr[i].flag = it_t::empty;
            arr[i].key = nullptr;
            arr[i].data = nullptr;
        }
    }

    ~Hash()
    {
        for (int i = 0; i < num; i++)
        {
            freeElement(arr + i);
        }
        free(arr);
    }

    bool insert(const char *key, const void *data)
    {
        int addr = findPosition(key);
        if (addr < 0)
            return false;
        if (arr[addr].flag != it_t::full)
            allocElement(arr + addr, key, data, size);
        else
            return false;
        return true;
    }
    bool remove(const char *key)
    {
        int addr = findPosition(key);
        if (addr < 0)
            return false;
        if (arr[addr].flag == it_t::full)
            freeElement(arr + addr);
        return true;
    }
    bool isHas(const char *key) const
    {
        int addr = findPosition(key);
        if (addr < 0)
            return false;
        return arr[addr].flag == it_t::full;
    }
    bool getData(const char *key, void *data) const
    {
        int addr = findPosition(key);
        if (addr < 0 || arr[addr].flag != it_t::full)
            return false;
        if (!arr[addr].data)
            return false;
        memcpy(data, arr[addr].data, size);
        return true;
    }
    bool setData(const char *key, const void *data)
    {
        int addr = findPosition(key);
        if (addr < 0 || arr[addr].flag != it_t::full)
            ;
        return false;
        if (!arr[addr].data)
            return false;
        memcpy(arr[addr].data, data, size);
        return true;
    }

    /**
     * @brief clear all 'deleted' flag nodes
     * @warning  thread not safe due to updating Hash::arr
     */
    void clearDleted()
    {
        it_t* newArr = static_cast<it_t*>(malloc(num * sizeof(*arr)));
        it_t* oldArr = arr;
        for (int i = 0; i < num; i++)
        {
            newArr[i].flag = it_t::empty;
            newArr[i].key = nullptr;
            newArr[i].data = nullptr;
        }
        listCopy(newArr, num);
        // clear old arr
        for (int i = 0; i < num; i++)
        {
            freeElement(oldArr + i);
        }
        free(oldArr);
    }

    size_t getCapacity() const
    {
        return num;
    }
    void updateCapacity(size_t newNum)
    {
        if (newNum < num)
            return;
        it_t* newArr = static_cast<it_t*>(malloc(num * sizeof(*arr)));
        it_t* oldArr = arr;
        size_t oldSize = num;
        for (int i = 0; i < newNum; i++)
        {
            newArr[i].flag = it_t::empty;
            newArr[i].key = nullptr;
            newArr[i].data = nullptr;
        }
        listCopy(newArr, newNum);
        for(int i = 0; i < oldSize; i++)
        {
            freeElement(oldArr + i);
        }
        free(oldArr);
    }

  protected:
    void listCopy(it_t* pNew, size_t sNew)
    {
        it_t* pOld = arr;
        size_t sOld = num;
        /** Critical area begin */
        arr = pNew;
        num = sNew;
        for (int i = 0; i < sOld; i++)
        {
            if (pOld[i].flag == it_t::full)
            {
                insert(pOld[i].key, pOld[i].data);
            }
        }
        /** Critial area end */
    }
    int findPosition(const char *key) const
    {
        unsigned addr = H1(key);
        unsigned cnt = 0;

        while (arr[addr].flag == it_t::full ||
               arr[addr].flag == it_t::deleted)
        {
            if (arr[addr].flag == it_t::full ||
                arr[addr].flag == it_t::deleted)
                break;
            if (++cnt > num)
                return -1;
            addr = static_cast<unsigned>(H1(key) + H2(key, cnt)) % num;
        }
        return static_cast<int>(addr);
    }
    static void allocElement(it_t *element,
                             const char *name,
                             const void *data,
                             size_t size)
    {
        if (element->flag == it_t::full)
            return;
        freeElement(element);
        element->key = static_cast<char*>(malloc(strlen(name) + 1));
        element->data = malloc(size);

        strcpy(element->key, name);
        memcpy(element->data, data, size);
        element->flag = it_t::full;
    }
    static void freeElement(it_t *element)
    {
        if (element->flag == it_t::empty)
            return;
        element->flag = it_t::deleted;
        if (element->key)
        {
            free(element->key);
            element->key = nullptr;
        }
        if (element->data)
        {
            free(element->data);
            element->data = nullptr;
        }
    }
    inline static unsigned H1(const char *key)
    {
        unsigned hash = 0;
        unsigned x = 0;
        char c;
        do {
            c = *key++;
            if ((x = hash & 0xF0000000) != 0)
            {
                hash ^= (x >> 24);
                hash &= ~x;
            }
        } while (c);
        return hash & 0x7FFFFFFF;
    }
    inline static unsigned H2(const char *key, unsigned i)
    {
        unsigned seed = 131;
        unsigned hash = 0;
        while (*key)
        {
            hash = hash * seed + (*key++);
        }
        return (hash * i) & 0x7FFFFFFF;
    }
};