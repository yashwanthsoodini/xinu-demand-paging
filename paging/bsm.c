/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

bs_map_t bsm_tab[NUMSTORES];

void init_bsm_entry(int);
void reset_bsm_entry(int);

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
    int i;
    for(i=0; i<NUMSTORES; i++){
        init_bsm_entry(i);
    }
    return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
    int i;
    for(i=0; i<NUMSTORES; i++)
    {
        if(bsm_tab[i].bs_status == BSM_UNMAPPED)
        {
            *avail = i;
            return OK;
        }
    }
    return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
    if(valid_bsid(i) && bsm_tab[i].bs_nprocs == 0)
    {
        reset_bsm_entry(i);
        return OK;
    }
    return SYSERR;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
    int vpageno = (vaddr & 0x003ff000) >> 12;
    
    int i;
    for(i=0; i<16; i++)
    {
        if(bsm_tab[i].bs_status = BSM_MAPPED && bsm_tab[i].bs_vpno_map[pid] > 4096 &&
           vpageno >= bsm_tab[i].bs_vpno_map[pid] && 
           vpageno < bsm_tab[i].bs_vpno_map[pid] + bsm_tab[i].bs_npages)
        {
            *store = i;
            *pageth = vpageno - bsm_tab[i].bs_vpno_map[pid];
            return OK;
        }
    }
    return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
    if(valid_bsid(source) && vpno >= 4096 && npages > 0 && npages <= STORESIZE)
    {
        bsm_tab[source].bs_vpno_map[pid] = vpno;
        
        if(bsm_tab[source].bs_status == BSM_UNMAPPED)
        {
            if(npages > bsm_tab[source].bs_npages)
            {
                return SYSERR;
            }
            bsm_tab[source].bs_status = BSM_MAPPED;
            bsm_tab[source].bs_npages = npages;
        }
        bsm_tab[source].bs_nprocs++;        
        return OK;
    }
    return SYSERR;
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
    int store;
    int page;
    if(bsm_lookup(pid, vpno*NBPG, &store, &page))
    {
        bsm_tab[store].bs_vpno_map[pid] = -1;
        bsm_tab[store].bs_nprocs--;
        if(bsm_tab[store].bs_nprocs == 0) free_bsm(store);
        return OK;
    }
    return SYSERR;
}

void init_bsm_entry(int bsid)
{
    bsm_tab[bsid].bs_status = BSM_UNMAPPED;
    int i;
    for(i=0; i<NPROC; i++)
    {
        bsm_tab[bsid].bs_vpno_map[i] = -1;
    }
    bsm_tab[bsid].bs_nprocs = 0;
    bsm_tab[bsid].bs_pid = -1;
    bsm_tab[bsid].bs_npages = 0;
    bsm_tab[bsid].bs_access = BS_SHARED;
    return;
}

void reset_bsm_entry(int bsid)
{
    init_bsm_entry(bsid);
}

