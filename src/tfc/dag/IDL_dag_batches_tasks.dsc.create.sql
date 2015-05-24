-- It had generated by DirectStruct v1.4.4

CREATE TABLE dag_batches_tasks
(
	schedule_name	CHARACTER VARYING(64) ,
	batch_name	CHARACTER VARYING(64) ,
	program_and_params	CHARACTER VARYING(256) ,
	timeout	INTEGER
) ;

CREATE INDEX dag_batches_tasks_idx1 ON dag_batches_tasks ( schedule_name , batch_name ) ;
