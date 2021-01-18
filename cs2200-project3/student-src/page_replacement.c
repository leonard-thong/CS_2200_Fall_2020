#include "types.h"
#include "pagesim.h"
#include "paging.h"
#include "swapops.h"
#include "stats.h"
#include "util.h"

pfn_t select_victim_frame(void);


/*  --------------------------------- PROBLEM 7 --------------------------------------
    Checkout PDF section 7 for this problem
    
    Make a free frame for the system to use.

    You will first call the page replacement algorithm to identify an
    "available" frame in the system.

    In some cases, the replacement algorithm will return a frame that
    is in use by another page mapping. In these cases, you must "evict"
    the frame by using the frame table to find the original mapping and
    setting it to invalid. If the frame is dirty, write its data to swap!
 * ----------------------------------------------------------------------------------
 */
pfn_t free_frame(void) {
    pfn_t victim_pfn;

    /* Call your function to find a frame to use, either one that is
       unused or has been selected as a "victim" to take from another
       mapping. */
    victim_pfn = select_victim_frame();

    /*
     * If victim frame is currently mapped, we must evict it:
     *
     * 1) Look up the corresponding page table entry
     * 2) If the entry is dirty, write it to disk with swap_write()
     * 3) Mark the original page table entry as invalid
     * 4) Unmap the corresponding frame table entry
     *
     */
    if (frame_table[victim_pfn].mapped) {
        vpn_t victim_vpn = frame_table[victim_pfn].vpn;
        pte_t *page_table = (pte_t *)(frame_table[victim_pfn].process -> saved_ptbr * PAGE_SIZE + mem);
        pte_t *victim_pte = &page_table[victim_vpn];
        if (victim_pte -> dirty == 1) {
            swap_write(victim_pte, (uint8_t*) (victim_pfn * PAGE_SIZE + mem));
            stats.writebacks++;
        }
        victim_pte -> dirty = 0;
        victim_pte -> valid = 0;
    }
    frame_table[victim_pfn].mapped = 0;
    /* Return the pfn */
    return victim_pfn;
}



/*  --------------------------------- PROBLEM 9 --------------------------------------
    Finds a free physical frame. If none are available, uses either a
    randomized or counter clock sweep algorithm to find a used frame for
    eviction.

    When implementing counter clock sweep, be sure to go in a backwards circular 
    fashion where you start at the bottom of the frame table, make your way to the 
    top, and circle back around. Also make sure to set the reference bits to 0 for 
    each frame that had its referenced bit set. Your counter clock sweep should remember
    the index at which it leaves off and resume at the same place for each run (you may 
    need to add a global or static variable for this).  

    Return:
        The physical frame number of a free (or evictable) frame.

    HINTS: Use the global variables MEM_SIZE and PAGE_SIZE to calculate
    the number of entries in the frame table.
    ----------------------------------------------------------------------------------
*/
pfn_t select_victim_frame() {
    static pfn_t pointer = NUM_FRAMES - 1;
    /* See if there are any free frames first */
    size_t num_entries = MEM_SIZE / PAGE_SIZE;
    for (size_t i = 0; i < num_entries; i++) {
        if (!frame_table[i].protected && !frame_table[i].mapped) {
            return i;
        }
    }

    if (replacement == RANDOM) {
        /* Play Russian Roulette to decide which frame to evict */
        pfn_t last_unprotected = NUM_FRAMES;
        for (pfn_t i = 0; i < num_entries; i++) {
            if (!frame_table[i].protected) {
                last_unprotected = i;
                if (prng_rand() % 2) {
                    return i;
                }
            }
        }
        /* If no victim found yet take the last unprotected frame
           seen */
        if (last_unprotected < NUM_FRAMES) {
            return last_unprotected;
        }
    } else if (replacement == COUNTERCLOCKSWEEP) {
        /* Implement an COUNTERCLOCKSWEEP page replacement algorithm here */
        for (pfn_t i = pointer; i >= 0; i--) {
            if (frame_table[i].referenced == 1) {
                frame_table[i].referenced = 0;
            } else if (frame_table[i].protected == 0) {
                if (i == 0) {
                    pointer = NUM_FRAMES - 1;
                } else {
                    pointer = i - 1;
                }
                return i;
            }
            if (i == 0) {
                i = NUM_FRAMES;
            }
        }
        /*
        for (pfn_t i = pointer; i >= 0; i--) {
            if (frame_table[i].protected == 0) {
                if (frame_table[i].referenced == 1) {
                    frame_table[i].referenced = 0;
                } else {
                    if (i == 0) {
                        pointer = NUM_FRAMES - 1;
                    } else {
                        pointer = i - 1;
                    }
                    return i;
                }
            } 
            if (i == 0) {
                i = NUM_FRAMES;
            }
        }
        */
    }

    /* If every frame is protected, give up. This should never happen
       on the traces we provide you. */
    panic("System ran out of memory\n");
    exit(1);
}
