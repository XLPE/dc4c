/* It had generated by DirectStruct v1.4.4 */

#include "IDL_dag_schedule.dsc.ESQL.eh"

EXEC SQL BEGIN DECLARE SECTION ;
	int dag_schedule_order_index ;	short dag_schedule_order_index_id ;
	char dag_schedule_schedule_name[ 64 + 1 ] ;	short dag_schedule_schedule_name_id ;
	char dag_schedule_schedule_desc[ 256 + 1 ] ;	short dag_schedule_schedule_desc_id ;
EXEC SQL END DECLARE SECTION ;

void DSCINITV_dag_schedule()
{
	dag_schedule_order_index = 0 ;
	dag_schedule_order_index_id = 0 ;
	memset( dag_schedule_schedule_name , 0x00 , sizeof(dag_schedule_schedule_name) );
	dag_schedule_schedule_name_id = 0 ;
	memset( dag_schedule_schedule_desc , 0x00 , sizeof(dag_schedule_schedule_desc) );
	dag_schedule_schedule_desc_id = 0 ;
	return;
}

void DSCVTOS_dag_schedule( dag_schedule *pst )
{
	pst->order_index = dag_schedule_order_index ;
	strcpy( pst->schedule_name , dag_schedule_schedule_name );
	strcpy( pst->schedule_desc , dag_schedule_schedule_desc );
	return;
}

void DSCSTOV_dag_schedule( dag_schedule *pst )
{
	dag_schedule_order_index = pst->order_index ;
	strcpy( dag_schedule_schedule_name , pst->schedule_name );
	strcpy( dag_schedule_schedule_desc , pst->schedule_desc );
	return;
}

#ifndef TRIM
#define TRIM(_str_) {char *p=(_str_)+strlen(_str_)-1; for(;((*p)==' ')&&(p>=(_str_));p--){(*p)='\0';}}
#endif

void DSCTRIM_dag_schedule( dag_schedule *pst )
{
	TRIM( pst->schedule_name )
	TRIM( pst->schedule_desc )
	return;
}

void DSCSQLACTION_SELECT_A_FROM_dag_schedule_WHERE_schedule_name_E( dag_schedule *pst )
{
	DSCSTOV_dag_schedule( pst );
	
	EXEC SQL
		SELECT	*
		INTO	DBVLLIST_dag_schedule
		FROM	dag_schedule
		WHERE	schedule_name = :dag_schedule_schedule_name
		;
	if( SQLCODE )
		return;
	
	DSCVTOS_dag_schedule( pst );
	
	return;
}

void DSCSQLACTION_OPEN_CURSOR_dag_schedule_cursor1_SELECT_A_FROM_dag_schedule_ORDER_BY_order_index( dag_schedule *pst )
{
	DSCSTOV_dag_schedule( pst );
	
	EXEC SQL
		DECLARE	dag_schedule_dag_schedule_cursor1 CURSOR FOR
		SELECT	*
		FROM	dag_schedule
		ORDER BY  order_index 
		;
	if( SQLCODE )
		return;
	
	EXEC SQL
		OPEN	dag_schedule_dag_schedule_cursor1
		;
	if( SQLCODE )
		return;
	
	return;
}

void DSCSQLACTION_FETCH_CURSOR_dag_schedule_cursor1( dag_schedule *pst )
{
	EXEC SQL
		FETCH	dag_schedule_dag_schedule_cursor1
		INTO	DBVLLIST_dag_schedule
		;
	if( SQLCODE )
		return;
	
	DSCVTOS_dag_schedule( pst );
	
	return;
}

void DSCSQLACTION_CLOSE_CURSOR_dag_schedule_cursor1()
{
	EXEC SQL
		CLOSE	dag_schedule_dag_schedule_cursor1
		;
	return;
}

EXEC SQL BEGIN DECLARE SECTION ;
	char    DBNAME[ 1024 + 1 ] ;
	char    DBUSER[ 128 + 1 ] ;
	char    DBPASS[ 128 + 1 ] ;
EXEC SQL END DECLARE SECTION ;

void DSCDBCONN( char *host , int port , char *dbname , char *user , char *pass )
{
	strcpy( DBNAME , dbname );
	if( host )
	{
		strcat( DBNAME , "@" );
		strcat( DBNAME , host );
	}
	if( port )
	{
		strcat( DBNAME , ":" );
		sprintf( DBNAME + strlen(DBNAME) , "%d" , port );
	}
	strcpy( DBUSER , user );
	strcpy( DBPASS , pass );
	
	EXEC SQL
		CONNECT TO	:DBNAME
		USER		:DBUSER
		IDENTIFIED BY	:DBPASS ;
	
	return;
}
void DSCDBDISCONN()
{
	EXEC SQL
		DISCONNECT ;
	
	return;
}
void DSCDBBEGINWORK()
{
	EXEC SQL
		BEGIN WORK ;
	
	return;
}
void DSCDBCOMMIT()
{
	EXEC SQL
		COMMIT WORK ;
	
	return;
}
void DSCDBROLLBACK()
{
	EXEC SQL
		ROLLBACK WORK ;
	
	return;
}
