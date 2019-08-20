#include "memory_manager.h"
#include "memory_info.h"
#include "utils.h"

static struct memory_info *free_list;
static uint64_t free_next_index;

static struct memory_info *used_list;
static uint64_t used_next_index;

bool init_memory_manager(size_t number_of_entries,
                         struct memory_info *meminfo)
{
    assert(free_list == NULL, "You called the init_memory_manager duplicate.");

    free_list = NULL;
    free_next_index = 0;
    used_list = NULL;
    used_next_index = 0;

    for (int i = 0; i < number_of_entries; ++i)
    {
        if (meminfo[i].type == PAGE_TYPE_FREE)
        {
            if (used_list == NULL && meminfo[i].number_of_pages >= 2)
            {
                used_list = (struct memory_info *)meminfo[i].base_address;
                used_list[0].base_address = meminfo[i].base_address;
                used_list[0].number_of_pages = 1;

                used_next_index++;

                meminfo[i].base_address += 0x1000;
                meminfo[i].number_of_pages -= 1;

                free_list = (struct memory_info *)meminfo[i].base_address;

                meminfo[i].base_address += 0x1000;
                meminfo[i].number_of_pages -= 1;
            }
        }

        if (used_list != NULL && free_list != NULL)
        {
            break;
        }
    }

    if (used_list == NULL || free_list == NULL)
    {
        return false;
    }

    for (int i = 0; i < number_of_entries; ++i)
    {
        if (meminfo[i].number_of_pages == 0)
        {
            continue;
        }

        if (meminfo[i].type == PAGE_TYPE_FREE)
        {
            free_list[free_next_index].base_address =
                meminfo[i].base_address;
            free_list[free_next_index].number_of_pages =
                meminfo[i].number_of_pages;
            free_next_index++;
        }
        else
        {
            used_list[used_next_index].base_address =
                meminfo[i].base_address;
            used_list[used_next_index].number_of_pages =
                meminfo[i].number_of_pages;
            used_next_index++;
        }
    }

    return true;
}

bool malloc_4k(uintptr_t *dest)
{
    for (int i = 0; i < free_next_index; ++i)
    {
        if (free_list[i].number_of_pages == 0)
        {
            continue;
        }
        *dest = free_list[i].base_address;

        free_list[i].base_address -= 0x1000;
        free_list[i].number_of_pages--;

        assert(free_list[i].base_address < 0xf000000000000000,
               "negatie overflow occured");
        break;
    }

    if (dest == NULL)
    {
        return false;
    }

    return true;
}
