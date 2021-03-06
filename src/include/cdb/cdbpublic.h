/*-------------------------------------------------------------------------
 *
 * cdbpublic.c
 *
 *		Greenplum specific structures and typedefs exposed to the
 *      outside world.
 *
 *      This is a non-ideal solution, consider alternatives before
 *      adding to this file.
 *
 * Copyright (c) 2008-2008, Greenplum inc.
 *
 *-------------------------------------------------------------------------
 */

/* This file should not include any other files under "cdb" */

#ifndef CDBPUBLIC_H
#define CDBPUBLIC_H

#include "c.h"         /* DistributedTransactionId */

/* Things defined in this header */
typedef struct TMGXACT_LOG           TMGXACT_LOG;
typedef struct CdbExplain_Agg        CdbExplain_Agg;

/* From "cdb/cdbtm.h" */
struct TMGXACT_LOG
{
	char						gid[TMGIDSIZE];
	DistributedTransactionId	gxid;
};



typedef struct TMGXACT_CHECKPOINT
{
	int						committedCount;
    /* Array [0..committedCount-1] of TMGXACT_LOG structs begins here */
	TMGXACT_LOG				committedGxactArray[1];
}	TMGXACT_CHECKPOINT;

#define TMGXACT_CHECKPOINT_BYTES(committedCount) \
	(offsetof(TMGXACT_CHECKPOINT, committedGxactArray) + sizeof(TMGXACT_LOG) * (committedCount))


/* From "cdb/cdbexplain.h" */
struct CdbExplain_Agg
{
    double      vmax;           /* maximum value of statistic */
    double      vsum;           /* sum of values */
    int         vcnt;           /* count of values > 0 */
    int         imax;           /* id of 1st observation having maximum value */
};

#endif

