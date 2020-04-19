#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/* release the backing store with ID bs_id */
SYSCALL release_bs(bsd_t bs_id) {
  if(valid_bsid(bs_id) && bsm_tab[bs_id].bs_access == BS_PRIVATE && 
     bsm_tab[bs_id].bs_pid == currpid) 
  {
    return free_bsm(bs_id);
  }

  return SYSERR;
}

