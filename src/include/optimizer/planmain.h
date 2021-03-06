/*-------------------------------------------------------------------------
 *
 * planmain.h
 *	  prototypes for various files in optimizer/plan
 *
 *
 * Portions Copyright (c) 2005-2009, Greenplum inc
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/optimizer/planmain.h,v 1.113 2008/10/04 21:56:55 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef PLANMAIN_H
#define PLANMAIN_H

#include "nodes/plannodes.h"
#include "nodes/relation.h"
#include "optimizer/clauses.h" /* AggClauseCounts */

/* GUC parameters */
#define DEFAULT_CURSOR_TUPLE_FRACTION 1.0 /* assume all rows will be fetched */
extern double cursor_tuple_fraction;

/*
 * A structure that contains information for planning GROUP BY 
 * queries.
 */
typedef struct GroupContext
{
	Path *best_path;
	Path *cheapest_path;

	/*
	 * If subplan is given, use it (including its targetlist).  
	 *
	 * If sub_tlist and no subplan is given, then use sub_tlist
	 * on the input plan. (This is intended to  assure that targets 
	 * that appear in the SortClauses of AggOrder  nodes have targets 
	 * in the subplan that match in sortgroupref.
	 *
	 * If neither subplan nor sub_tlist is given, just make a plan with
	 * a flat target list.
	 */
	Plan *subplan;
	List *sub_tlist;

	List *tlist;
	bool use_hashed_grouping;
	double tuple_fraction;
	CanonicalGroupingSets *canonical_grpsets;
	uint64 grouping;

	/*
	 * When subplan is privided, groupColIdx and distinctColIdx are also provided.
	 */
	int numGroupCols;
	AttrNumber *groupColIdx;
	Oid		   *groupOperators;
	int numDistinctCols;
	AttrNumber *distinctColIdx;

	double *p_dNumGroups;
	List **pcurrent_pathkeys;
	bool *querynode_changed;
} GroupContext;

/*
 * prototypes for plan/planmain.c
 */
extern void query_planner(PlannerInfo *root, List *tlist,
			  double tuple_fraction, double limit_tuples,
			  Path **cheapest_path, Path **sorted_path,
			  double *num_groups);

/*
 * prototypes for plan/planagg.c
 */
extern Plan *optimize_minmax_aggregates(PlannerInfo *root, List *tlist,
						   Path *best_path);

/*
 * prototype for plan/plangroupexp.c
 */
extern Plan *make_distinctaggs_for_rollup(PlannerInfo *root, bool is_agg,
										  List *tlist, bool twostage, List *sub_tlist,
										  List *qual, AggStrategy aggstrategy,
										  int numGroupCols, AttrNumber *grpColIdx, Oid *grpOperators,
										  double numGroups, int *rollup_gs_times,
										  int numAggs, int transSpace,
										  double *p_dNumGroups,
										  List **p_current_pathkeys,
										  Plan *lefttree);

/*
 * prototypes for plan/planwindow.c
 */
extern Plan *window_planner(PlannerInfo *root, double tuple_fraction, List **pathkeys_ptr);
extern RangeTblEntry *package_plan_as_rte(Query *query, Plan *plan, Alias *eref, List *pathkeys);
extern Value *get_tle_name(TargetEntry *tle, List* rtable, const char *default_name);
extern bool contain_windowref(Node *node, void *context);
extern bool window_edge_is_delayed(WindowFrameEdge *edge);
extern Plan *wrap_plan(PlannerInfo *root, Plan *plan, Query *query, List **p_pathkeys,
       const char *alias_name, List *col_names, Query **query_p);


/*
 * prototype for plan/plangroupext.c
 */
extern Plan *plan_grouping_extension(PlannerInfo *root,
									 Path *path,
									 double tuple_fraction,
									 bool use_hashed_grouping,
									 List **p_tlist, List *sub_tlist,
									 bool is_agg, bool twostage,
									 List *qual,
									 int *p_numGroupCols, AttrNumber **p_grpColIdx, Oid **p_grpOperators,
									 AggClauseCounts *agg_counts,
									 CanonicalGroupingSets *canonical_grpsets,
									 double *p_dNumGroups,
									 bool *querynode_changed,
									 List **p_current_pathkeys,
									 Plan *lefttree);
extern void free_canonical_groupingsets(CanonicalGroupingSets *canonical_grpsets);
extern Plan *add_repeat_node(Plan *result_plan, int repeat_count, uint64 grouping);

/*
 * prototypes for plan/createplan.c
 */
extern Plan *create_plan(PlannerInfo *root, Path *path);
extern SubqueryScan *make_subqueryscan(PlannerInfo *root, List *qptlist, List *qpqual,
				  Index scanrelid, Plan *subplan, List *subrtable);
extern Append *make_append(List *appendplans, bool isTarget, List *tlist);
extern RecursiveUnion *make_recursive_union(List *tlist,
			   Plan *lefttree, Plan *righttree, int wtParam);
extern Sort *make_sort_from_pathkeys(PlannerInfo *root, Plan *lefttree,
						List *pathkeys, double limit_tuples, bool add_keys_to_targetlist);
extern Sort *make_sort_from_sortclauses(PlannerInfo *root, List *sortcls,
						   Plan *lefttree);
extern Sort *make_sort_from_groupcols(PlannerInfo *root, List *groupcls,
									  AttrNumber *grpColIdx, bool appendGrouping,
									  Plan *lefttree);
extern Sort *make_sort_from_reordered_groupcols(PlannerInfo *root,
												List *groupcls,
												AttrNumber *orig_grpColIdx,
												AttrNumber *new_grpColIdx,
												TargetEntry *grouping,
												TargetEntry *groupid,
												int req_ngrpkeys,
												Plan *lefttree);
extern List *reconstruct_group_clause(List *orig_groupClause, List *tlist,
						 AttrNumber *grpColIdx, int numcols);

extern Agg *make_agg(PlannerInfo *root, List *tlist, List *qual,
					 AggStrategy aggstrategy, bool streaming,
					 int numGroupCols, AttrNumber *grpColIdx, Oid *grpOperators,
					 long numGroups, int numNullCols,
					 uint64 inputGrouping, uint64 grouping,
					 int rollupGSTimes,
					 int numAggs, int transSpace,
					 Plan *lefttree);
extern HashJoin *make_hashjoin(List *tlist,
			  List *joinclauses, List *otherclauses,
			  List *hashclauses, List *hashqualclauses,
			  Plan *lefttree, Plan *righttree,
			  JoinType jointype);
extern Hash *make_hash(Plan *lefttree);
extern NestLoop *make_nestloop(List *tlist,
							   List *joinclauses, List *otherclauses,
							   Plan *lefttree, Plan *righttree,
							   JoinType jointype);
extern MergeJoin *make_mergejoin(List *tlist,
			   List *joinclauses, List *otherclauses,
			   List *mergeclauses,
			   Oid *mergefamilies,
			   int *mergestrategies,
			   bool *mergenullsfirst,
			   Plan *lefttree, Plan *righttree,
			   JoinType jointype);
extern Window *make_window(PlannerInfo *root, List *tlist,
			int numPartCols, AttrNumber *partColIdx, Oid *partOperators,
			List *windowKeys, Plan *lefttree);
extern Material *make_material(Plan *lefttree);
extern Plan *materialize_finished_plan(PlannerInfo *root, Plan *subplan);
extern Unique *make_unique(Plan *lefttree, List *distinctList);
extern Limit *make_limit(Plan *lefttree, Node *limitOffset, Node *limitCount,
		   int64 offset_est, int64 count_est);
extern SetOp *make_setop(SetOpCmd cmd, Plan *lefttree,
		   List *distinctList, AttrNumber flagColIdx);
extern Result *make_result(PlannerInfo *root, List *tlist,
			Node *resconstantqual, Plan *subplan);
extern Repeat *make_repeat(List *tlist,
						   List *qual,
						   Expr *repeatCountExpr,
						   uint64 grouping,
						   Plan *subplan);
extern bool is_projection_capable_plan(Plan *plan);
extern Plan *add_sort_cost(PlannerInfo *root, Plan *input, 
						   int numCols, 
						   AttrNumber *sortColIdx, Oid *sortOperators,
						   double limit_tuples);
extern Plan *add_agg_cost(PlannerInfo *root, Plan *plan, 
		 List *tlist, List *qual,
		 AggStrategy aggstrategy, 
		 bool streaming, 
		 int numGroupCols, AttrNumber *grpColIdx,
		 long numGroups, int num_nullcols,
		 int numAggs, int transSpace);
extern Plan *plan_pushdown_tlist(PlannerInfo *root, Plan *plan, List *tlist);      /*CDB*/

/*
 * prototypes for plan/initsplan.c
 */
extern int	from_collapse_limit;
extern int	join_collapse_limit;

extern void add_base_rels_to_query(PlannerInfo *root, Node *jtnode);
extern void build_base_rel_tlists(PlannerInfo *root, List *final_tlist);
extern void add_IN_vars_to_tlists(PlannerInfo *root);
extern void add_vars_to_targetlist(PlannerInfo *root, List *vars,
					   Relids where_needed);
extern List *deconstruct_jointree(PlannerInfo *root);
extern void distribute_restrictinfo_to_rels(PlannerInfo *root,
								RestrictInfo *restrictinfo);
extern void process_implied_equality(PlannerInfo *root,
						 Oid opno,
						 Expr *item1,
						 Expr *item2,
						 Relids qualscope,
						 Relids nullable_relids,
						 bool below_outer_join,
						 bool both_const);
extern RestrictInfo *build_implied_join_equality(Oid opno,
							Expr *item1,
							Expr *item2,
							Relids qualscope,
							Relids nullable_relids);

/*
 * prototypes for plan/setrefs.c
 */
extern Plan *set_plan_references(PlannerGlobal *glob,
					Plan *plan,
					List *rtable);
extern List *set_returning_clause_references(PlannerGlobal *glob,
								List *rlist,
								Plan *topplan,
								Index resultRelation);

extern void extract_query_dependencies(List *queries,
						   List **relationOids,
						   List **invalItems);
extern void fix_opfuncids(Node *node);
extern void set_opfuncid(OpExpr *opexpr);
extern void set_sa_opfuncid(ScalarArrayOpExpr *opexpr);
extern void record_plan_function_dependency(PlannerGlobal *glob, Oid funcid);
extern void extract_query_dependencies(List *queries,
									   List **relationOids,
									   List **invalItems);
extern void cdb_extract_plan_dependencies(PlannerGlobal *glob, Plan *plan);

extern int num_distcols_in_grouplist(List *gc);

#endif   /* PLANMAIN_H */
