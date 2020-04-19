#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/* requests a new mapping of npages with ID map_id */
int get_bs(bsd_t bs_id, unsigned int npages)
{
  if(npages <=0 || npages >= 128 || !valid_bsid(bs_id) || 
     bsm_tab[bs_id].bs_access == BS_PRIVATE) 
    return SYSERR;

  if(bsm_tab[bs_id].bs_status == BSM_MAPPED) return bsm_tab[bs_id].bs_npages;
  else return npages;
}
