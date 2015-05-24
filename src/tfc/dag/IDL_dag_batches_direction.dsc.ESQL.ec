/* It had generated by DirectStruct v1.4.4 */

#include "IDL_dag_batches_direction.dsc.ESQL.eh"

EXEC SQL BEGIN DECLARE SECTION ;
	char dag_batches_direction_schedule_name[ 64 + 1 ] ;	short dag_batches_direction_schedule_name_id ;
	char dag_batches_direction_from_batch[ 64 + 1 ] ;	short dag_batches_direction_from_batch_id ;
	char dag_batches_direction_to_batch[ 64 + 1 ] ;	short dag_batches_direction_to_batch_id ;
EXEC SQL END DECLARE SECTION ;

void DSCINITV_dag_batches_direction()
{
	memset( dag_batches_direction_schedule_name , 0x00 , sizeof(dag_batches_direction_schedule_name) );
	dag_batches_direction_schedule_name_id = 0 ;
	memset( dag_batches_direction_from_batch , 0x00 , sizeof(dag_batches_direction_from_batch) );
	dag_batches_direction_from_batch_id = 0 ;
	memset( dag_batches_direction_to_batch , 0x00 , sizeof(dag_batches_direction_to_batch) );
	dag_batches_direction_to_batch_id = 0 ;
	return;
}

void DSCVTOS_dag_batches_direction( dag_batches_direction *pst )
{
	strcpy( pst->schedule_name , dag_batches_direction_schedule_name );
	strcpy( pst->from_batch , dag_batches_direction_from_batch );
	strcpy( pst->to_batch , dag_batches_direction_to_batch );
	return;
}

void DSCSTOV_dag_batches_direction( dag_batches_direction *pst )
{
	strcpy( dag_batches_direction_schedule_name , pst->schedule_name );
	strcpy( dag_batches_direction_from_batch , pst->from_batch );
	strcpy( dag_batches_direction_to_batch , pst->to_batch );
	return;
}

#ifndef TRIM
#define TRIM(_str_) {char *p=(_str_)+strlen(_str_)-1; for(;((*p)==' ')&&(p>=(_str_));p--){(*p)='\0';}}
#endif

void DSCTRIM_dag_batches_direction( dag_batches_direction *pst )
{
	TRIM( pst->schedule_name )
	TRIM( pst->from_batch )
	TRIM( pst->to_batch )
	return;
}

void DSCSQLACTION_OPEN_CURSOR_dag_batches_direction_cursor1_SELECT_A_FROM_dag_batches_direction_WHERE_schedule_name_E( dag_batches_direction *pst )
{
	DSCSTOV_dag_batches_direction( pst );
	
	EXEC SQL
		DECLARE	dag_batches_direction_dag_batches_direction_cursor1 CURSOR FOR
		SELECT	*
		FROM	dag_batches_direction
		WHERE	schedule_name = :dag_batches_direction_schedule_name
		;
	if( SQLCODE )
		return;
	
	EXEC SQL
		OPEN	dag_batches_direction_dag_batches_direction_cursor1
		;
	if( SQLCODE )
		return;
	
	return;
}

void DSCSQLACTION_FETCH_CURSOR_dag_batches_direction_cursor1( dag_batches_direction *pst )
{
	EXEC SQL
		FETCH	dag_batches_direction_dag_batches_direction_cursor1
		INTO	DBVLLIST_dag_batches_direction
		;
	if( SQLCODE )
		return;
	
	DSCVTOS_dag_batches_direction( pst );
	
	return;
}

void DSCSQLACTION_CLOSE_CURSOR_dag_batches_direction_cursor1()
{
	EXEC SQL
		CLOSE	dag_batches_direction_dag_batches_direction_cursor1
		;
	return;
}
