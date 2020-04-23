/* paging.h */

#ifndef _PAGING_H_
#define _PAGING_H_

typedef unsigned int	 bsd_t;

/* Structure for a page directory entry */

typedef struct {

  unsigned int pd_pres	: 1;		/* page table present?		*/
  unsigned int pd_write : 1;		/* page is writable?		*/
  unsigned int pd_user	: 1;		/* is use level protection?	*/
  unsigned int pd_pwt	: 1;		/* write through cachine for pt?*/
  unsigned int pd_pcd	: 1;		/* cache disable for this pt?	*/
  unsigned int pd_acc	: 1;		/* page table was accessed?	*/
  unsigned int pd_mbz	: 1;		/* must be zero			*/
  unsigned int pd_fmb	: 1;		/* four MB pages?		*/
  unsigned int pd_global: 1;		/* global (ignored)		*/
  unsigned int pd_avail : 3;		/* for programmer's use		*/
  unsigned int pd_base	: 20;		/* location of page table?	*/
} pd_t;

/* Structure for a page table entry */

typedef struct {

  unsigned int pt_pres	: 1;		/* page is present?		*/
  unsigned int pt_write : 1;		/* page is writable?		*/
  unsigned int pt_user	: 1;		/* is use level protection?	*/
  unsigned int pt_pwt	: 1;		/* write through for this page? */
  unsigned int pt_pcd	: 1;		/* cache disable for this page? */
  unsigned int pt_acc	: 1;		/* page was accessed?		*/
  unsigned int pt_dirty : 1;		/* page was written?		*/
  unsigned int pt_mbz	: 1;		/* must be zero			*/
  unsigned int pt_global: 1;		/* should be zero in 586	*/
  unsigned int pt_avail : 3;		/* for programmer's use		*/
  unsigned int pt_base	: 20;		/* location of page?		*/
} pt_t;

typedef struct{
  unsigned int pg_offset : 12;		/* page offset			*/
  unsigned int pt_offset : 10;		/* page table offset		*/
  unsigned int pd_offset : 10;		/* page directory offset	*/
} virt_addr_t;

#include <proc.h>

typedef struct{
  int bs_status;		    	/* MAPPED or UNMAPPED		*/
  int bs_vpno_map[NPROC]; /* maps procs to their staring vpno's */
  int bs_nprocs;          /* num procs with mappings to this store */
  int bs_pid;				      /* -1 or pid of bs owner if private */
  // int bs_vpno;			 	  /* starting virtual page number */
  int bs_npages;		  	  /* number of pages in the store */
  int bs_access;          /* BS_PRIVATE or  BS_SHARED  */
  int bs_sem;				      /* semaphore mechanism ?	*/
} bs_map_t;

typedef struct{
  int fr_status;			/* MAPPED or UNMAPPED		*/
  int fr_pid;				/* process id using this frame  */
  int fr_vpno;				/* corresponding virtual page no*/
  int fr_refcnt;			/* reference count		*/
  int fr_refbit;      /* reference bit for second-chance algo */
  int fr_type;				/* FR_DIR, FR_TBL, FR_PAGE	*/
  int fr_dirty;
  int fr_prev;        /* prev node in the FIFO circular llist */
  int fr_next;        /* next node in the FIFO circular llist */
}fr_map_t;

extern bs_map_t bsm_tab[];
extern fr_map_t frm_tab[];
extern int frm_fifo_head;
extern int glb_pt_fr[];

/* Prototypes for required API calls */
SYSCALL xmmap(int, bsd_t, int);
SYSCALL xunmap(int);

/* given calls for dealing with backing store */

int get_bs(bsd_t, unsigned int);
SYSCALL release_bs(bsd_t);
SYSCALL read_bs(char *, bsd_t, int);
SYSCALL write_bs(char *, bsd_t, int);

int create_pd(int);
int create_pt(int);
void insert_fifo(int);

#define NUMSTORES   16   /* num backing stores          */
#define STORESIZE   128  /* backing store size in pages */

#define NBPG		4096	/* number of bytes per page	*/
#define FRAME0		1024	/* zero-th frame		*/
#define NFRAMES 	1024	/* number of frames		*/

#define BSM_UNMAPPED	0
#define BSM_MAPPED	1

#define BS_SHARED   0
#define BS_PRIVATE  1

#define FRM_UNMAPPED	0
#define FRM_MAPPED	1

#define FR_PAGE		0
#define FR_TBL		1
#define FR_DIR		2

#define SC 3
#define LFU 4

#define BACKING_STORE_BASE	0x00800000
#define BACKING_STORE_UNIT_SIZE 0x00100000

#define valid_bsid(bsid)  (bsid >= 0 && bsid <16)
#define valid_bspage(page) (page >=0 && page < 128)

#define valid_frid(frid)  (frid >= 0 && frid < NFRAMES)

#endif