/* It had generated by DirectStruct v1.4.5 */

#include "IDL_dag_batches_info.dsc.ESQL.eh"

EXEC SQL BEGIN DECLARE SECTION ;
	char dag_batches_info_schedule_name[ 64 + 1 ] ;	short dag_batches_info_schedule_name_id ;
	char dag_batches_info_batch_name[ 64 + 1 ] ;	short dag_batches_info_batch_name_id ;
	char dag_batches_info_batch_desc[ 64 + 1 ] ;	short dag_batches_info_batch_desc_id ;
	int dag_batches_info_view_pos_x ;	short dag_batches_info_view_pos_x_id ;
	int dag_batches_info_view_pos_y ;	short dag_batches_info_view_pos_y_id ;
	char dag_batches_info_begin_datetime[ 19 + 1 ] ;	short dag_batches_info_begin_datetime_id ;
	char dag_batches_info_end_datetime[ 19 + 1 ] ;	short dag_batches_info_end_datetime_id ;
	int dag_batches_info_progress ;	short dag_batches_info_progress_id ;
EXEC SQL END DECLARE SECTION ;

void DSCINITV_dag_batches_info()
{
	memset( dag_batches_info_schedule_name , 0x00 , sizeof(dag_batches_info_schedule_name) );
	dag_batches_info_schedule_name_id = 0 ;
	memset( dag_batches_info_batch_name , 0x00 , sizeof(dag_batches_info_batch_name) );
	dag_batches_info_batch_name_id = 0 ;
	memset( dag_batches_info_batch_desc , 0x00 , sizeof(dag_batches_info_batch_desc) );
	dag_batches_info_batch_desc_id = 0 ;
	dag_batches_info_view_pos_x = 0 ;
	dag_batches_info_view_pos_x_id = 0 ;
	dag_batches_info_view_pos_y = 0 ;
	dag_batches_info_view_pos_y_id = 0 ;
	memset( dag_batches_info_begin_datetime , 0x00 , sizeof(dag_batches_info_begin_datetime) );
	dag_batches_info_begin_datetime_id = 0 ;
	memset( dag_batches_info_end_datetime , 0x00 , sizeof(dag_batches_info_end_datetime) );
	dag_batches_info_end_datetime_id = 0 ;
	dag_batches_info_progress = 0 ;
	dag_batches_info_progress_id = 0 ;
	return;
}

void DSCVTOS_dag_batches_info( dag_batches_info *pst )
{
	strcpy( pst->schedule_name , dag_batches_info_schedule_name );
	strcpy( pst->batch_name , dag_batches_info_batch_name );
	strcpy( pst->batch_desc , dag_batches_info_batch_desc );
	pst->view_pos_x = dag_batches_info_view_pos_x ;
	pst->view_pos_y = dag_batches_info_view_pos_y ;
	strcpy( pst->begin_datetime , dag_batches_info_begin_datetime );
	strcpy( pst->end_datetime , dag_batches_info_end_datetime );
	pst->progress = dag_batches_info_progress ;
	return;
}

void DSCSTOV_dag_batches_info( dag_batches_info *pst )
{
	strcpy( dag_batches_info_schedule_name , pst->schedule_name );
	strcpy( dag_batches_info_batch_name , pst->batch_name );
	strcpy( dag_batches_info_batch_desc , pst->batch_desc );
	dag_batches_info_view_pos_x = pst->view_pos_x ;
	dag_batches_info_view_pos_y = pst->view_pos_y ;
	strcpy( dag_batches_info_begin_datetime , pst->begin_datetime );
	strcpy( dag_batches_info_end_datetime , pst->end_datetime );
	dag_batches_info_progress = pst->progress ;
	return;
}

#ifndef TRIM
#define TRIM(_str_) {char *p=(_str_)+strlen(_str_)-1; for(;((*p)==' ')&&(p>=(_str_));p--){(*p)='\0';}}
#endif

void DSCTRIM_dag_batches_info( dag_batches_info *pst )
{
	TRIM( pst->schedule_name )
	TRIM( pst->batch_name )
	TRIM( pst->batch_desc )
	TRIM( pst->begin_datetime )
	TRIM( pst->end_datetime )
	return;
}

void DSCSQLACTION_OPEN_CURSOR_dag_batches_info_cursor1_SELECT_A_FROM_dag_batches_info_WHERE_schedule_name_E( dag_batches_info *pst )
{
	DSCSTOV_dag_batches_info( pst );
	
	EXEC SQL
		DECLARE	dag_batches_info_dag_batches_info_cursor1 CURSOR FOR
		SELECT	*
		FROM	dag_batches_info
		WHERE	schedule_name = :dag_batches_info_schedule_name
		;
	if( SQLCODE )
		return;
	
	EXEC SQL
		OPEN	dag_batches_info_dag_batches_info_cursor1
		;
	if( SQLCODE )
		return;
	
	return;
}

void DSCSQLACTION_FETCH_CURSOR_dag_batches_info_cursor1( dag_batches_info *pst )
{
	EXEC SQL
		FETCH	dag_batches_info_dag_batches_info_cursor1
		INTO	DBVLLIST_dag_batches_info
		;
	if( SQLCODE )
		return;
	
	DSCVTOS_dag_batches_info( pst );
	
	return;
}

void DSCSQLACTION_CLOSE_CURSOR_dag_batches_info_cursor1()
{
	EXEC SQL
		CLOSE	dag_batches_info_dag_batches_info_cursor1
		;
	return;
}

void DSCSQLACTION_UPDATE_dag_batches_info_SET_begin_datetime_end_datetime_progress_WHERE_schedule_name_E_AND_batch_name_E( dag_batches_info *pst )
{
	DSCSTOV_dag_batches_info( pst );
	
	EXEC SQL
		UPDATE	dag_batches_info
		SET	begin_datetime = :dag_batches_info_begin_datetime, end_datetime = :dag_batches_info_end_datetime, progress = :dag_batches_info_progress
		WHERE	schedule_name = :dag_batches_info_schedule_name AND batch_name = :dag_batches_info_batch_name
		;
	return;
}