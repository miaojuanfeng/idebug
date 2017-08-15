/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_idebug.h"

/* If you declare any globals in php_idebug.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(idebug)
*/

/* True global resources - no need for thread safety here */
static int le_idebug;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("idebug.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_idebug_globals, idebug_globals)
    STD_PHP_INI_ENTRY("idebug.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_idebug_globals, idebug_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_idebug_compiled(string arg)
   Return a string to confirm that the module is compiled in */
zval* hash_table_key_value(HashTable *table TSRMLS_DC){
	HashPosition pos;
	zval *value;
	zval *retval;

	MAKE_STD_ZVAL(retval);
	array_init(retval);
	if( table ){
		pos = table->pListHead;
		while(pos){
			value = (zval*)pos->pDataPtr?pos->pDataPtr:pos->pData;
			switch(Z_TYPE_P(value)){
				case IS_NULL:
				case IS_LONG:
				case IS_DOUBLE:
				case IS_BOOL:
				case IS_ARRAY:
				case IS_OBJECT:
				case IS_STRING:
					Z_ADDREF_P(value);
					zend_hash_update(Z_ARRVAL_P(retval), pos->arKey, pos->nKeyLength+1, &value, sizeof(zval *), NULL);
					break;
				case IS_RESOURCE:
				case IS_CONSTANT:
				case IS_CONSTANT_AST:
				case IS_CALLABLE:
				default:
					break;
			}
			pos = pos->pListNext;
		}
	}
	return retval;
}

zval* hash_table_key(HashTable *table TSRMLS_DC){
	HashPosition pos;
	zval *key;
	zval *retval;

	MAKE_STD_ZVAL(retval);
	array_init(retval);
	if( table ){
		pos = table->pListHead;
		while(pos){
			MAKE_STD_ZVAL(key);
			ZVAL_STRINGL(key, pos->arKey, pos->nKeyLength, 1);
			zend_hash_next_index_insert(Z_ARRVAL_P(retval), &key, sizeof(zval *), NULL);
			pos = pos->pListNext;
		}
	}
	return retval;
}

PHP_FUNCTION(idebug_symbol_table)
{
	if( return_value_used ){
		RETURN_ZVAL(hash_table_key_value(&EG(symbol_table) TSRMLS_CC), 0, 1);
	}
}

PHP_FUNCTION(idebug_active_symbol_table)
{
	if( return_value_used ){
		if(!EG(active_symbol_table)){
			zend_rebuild_symbol_table(TSRMLS_C);
		}
		RETURN_ZVAL(hash_table_key_value(EG(active_symbol_table) TSRMLS_CC), 0, 1);
	}
}

PHP_FUNCTION(idebug_function_table)
{
	if( return_value_used ){
		RETURN_ZVAL(hash_table_key(EG(function_table) TSRMLS_CC), 0, 1);
	}
}

PHP_FUNCTION(idebug_class_table)
{
	if( return_value_used ){
		RETURN_ZVAL(hash_table_key(EG(class_table) TSRMLS_CC), 0, 1);
	}
}

PHP_FUNCTION(idebug_constant_table)
{
	if( return_value_used ){
		RETURN_ZVAL(hash_table_key_value(EG(zend_constants) TSRMLS_CC), 0, 1);
	}
}

PHP_FUNCTION(idebug_included_files)
{
	if( return_value_used ){
		RETURN_ZVAL(hash_table_key(&EG(included_files) TSRMLS_CC), 0, 1);
	}
	/* just test : pData point to a integer which value is 1
	HashPosition pos;
	pos = EG(included_files).pListHead;
	php_printf("included_files_table: Array( ");
	while(pos){
		php_printf("'%s'",pos->arKey);
		php_printf(" => %d",*(int*)pos->pData);
		if(pos->pListNext) php_printf(", ");
		pos = pos->pListNext;
	}
	php_printf(" )\n");*/
}

PHP_FUNCTION(idebug_function_call_stack)
{
	if( return_value_used ){
		zval *value;
		zval *retval;
		/*void **p;
		zend_execute_data *ex = EG(current_execute_data)->prev_execute_data;
		zend_execute_data *cx = EG(current_execute_data);
		p = ex->function_state.arguments;
		php_printf("current function_name:%s\n",cx->function_state.function->common.function_name);
		php_printf("prev function_name:%s\n",ex->function_state.function->common.function_name);
		php_printf("num_args:%d\n",(int)(zend_uintptr_t)*p);*/
		typedef struct _idebug_data_stack
		{
			void *data;
			struct _idebug_data_stack *next;
		}idebug_data_stack;

		idebug_data_stack *p = NULL, *q = NULL;
		zend_execute_data *ex = EG(current_execute_data)->prev_execute_data;

		while(ex){
			p = emalloc(sizeof(idebug_data_stack));
			p->data = (void*)ex;
			p->next = q;
			q = p;
			ex = ex->prev_execute_data;
		}
		
		MAKE_STD_ZVAL(retval);
		array_init(retval);
		while(p){
			q = p;
			MAKE_STD_ZVAL(value);
			ZVAL_STRING(value, ((zend_execute_data*)p->data)->function_state.function->common.function_name, 1);
			zend_hash_next_index_insert(Z_ARRVAL_P(retval), &value, sizeof(zval *), NULL);
			p= p->next;
			efree(q);
		}
		RETURN_ZVAL(retval, 0, 1);
	}
}

PHP_FUNCTION(idebug_function_args)
{
	if( return_value_used ){
		zval* retval;

		void **p;
		int num_args = 0;
		int i;
		zval *arg;
		zend_execute_data *ex;

		if(EG(current_execute_data)->prev_execute_data){
			ex = EG(current_execute_data)->prev_execute_data;

			p = ex->function_state.arguments;
			num_args = (int)(zend_uintptr_t)*p;
		}

		MAKE_STD_ZVAL(retval);
		array_init(retval);
		while( num_args > 0 ){
			arg = *(zval**)( p - num_args );
			zend_hash_next_index_insert(Z_ARRVAL_P(retval), &arg, sizeof(zval *), NULL);
			--num_args;
		}
		RETURN_ZVAL(retval, 0, 1);
	}
}

PHP_FUNCTION(idebug_compile_file)
{
	zval *inc_filename;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &inc_filename) == FAILURE) {
		return;
	}

	Bucket *func_bucket_start = EG(function_table)->pListTail;
	// Bucket *symbol_bucket_start = EG(zend_constants)->pListHead;
	// php_printf("%d\n", EG(zend_constants)->nNumOfElements);

	zend_file_handle file_handle;
	char *resolved_path;
	zend_op_array *new_op_array=NULL;
	zend_bool failure_retval = 0;
	resolved_path = zend_resolve_path(Z_STRVAL_P(inc_filename), Z_STRLEN_P(inc_filename) TSRMLS_CC);
	if (resolved_path) {
		failure_retval = zend_hash_exists(&EG(included_files), resolved_path, strlen(resolved_path)+1);
	} else {
		resolved_path = Z_STRVAL_P(inc_filename);
	}
	if (failure_retval) {
		// do nothing, file already included
	} else if (SUCCESS == zend_stream_open(resolved_path, &file_handle TSRMLS_CC)) {
		if (!file_handle.opened_path) {
			file_handle.opened_path = estrdup(resolved_path);
		}
		if (zend_hash_add_empty_element(&EG(included_files), file_handle.opened_path, strlen(file_handle.opened_path)+1)==SUCCESS) {
			new_op_array = zend_compile_file(&file_handle, ZEND_INCLUDE TSRMLS_CC);
			zend_destroy_file_handle(&file_handle TSRMLS_CC);
		} else {
			zend_file_handle_dtor(&file_handle TSRMLS_CC);
			failure_retval=1;
		}
	} else {
		zend_message_dispatcher(ZMSG_FAILED_INCLUDE_FOPEN, Z_STRVAL_P(inc_filename) TSRMLS_CC);
	}
	if (resolved_path != Z_STRVAL_P(inc_filename)) {
		efree(resolved_path);
	}

	int j;
	for(j=0;j<new_op_array->last;j++){
		zend_op *op = &new_op_array->opcodes[j];
		php_printf("%d\n", op->opcode);
	}

	Bucket *func_bucket_end = EG(function_table)->pListTail;
	// Bucket *symbol_bucket_end = EG(zend_constants)->pListTail;
	// php_printf("%d\n", EG(zend_constants)->nNumOfElements);

	zval *retval;
	MAKE_STD_ZVAL(retval);
	array_init(retval);

	if( func_bucket_start != func_bucket_end ){
		zend_uint index = 0;
		Bucket *p = func_bucket_start;

		zval *func_arr;
		MAKE_STD_ZVAL(func_arr);
		array_init(func_arr);
		do{
			p = p->pListNext;
			zend_function *func = (zend_function *)p->pData;
			if( func->type == ZEND_USER_FUNCTION ){
				// func_obj
				zval *func_obj;
				MAKE_STD_ZVAL(func_obj);
				object_init(func_obj);
				// function_name
				zend_update_property_string(NULL, func_obj, "function_name", 13, func->common.function_name TSRMLS_CC);
				// type
				zend_update_property_string(NULL, func_obj, "type", 4, "ZEND_USER_FUNCTION" TSRMLS_CC);
				// num_args
				zend_update_property_long(NULL, func_obj, "num_args", 8, func->common.num_args TSRMLS_CC);
				// required_num_args
				zend_update_property_long(NULL, func_obj, "required_num_args", 17, func->common.required_num_args TSRMLS_CC);
				// arg_info
				int i;
				zval *arg_arr;
				MAKE_STD_ZVAL(arg_arr);
				array_init(arg_arr);
				for(i=0;i<func->common.num_args;i++){
					zval *arg_info;
					MAKE_STD_ZVAL(arg_info);
					object_init(arg_info);
					zend_update_property_string(NULL, arg_info, "name", 4, func->common.arg_info[i].name TSRMLS_CC);
					zend_update_property_long(NULL, arg_info, "name_len", 8, func->common.arg_info[i].name_len TSRMLS_CC);
					if( func->common.arg_info[i].class_name_len ){
						zend_update_property_string(NULL, arg_info, "class_name", 10, func->common.arg_info[i].class_name TSRMLS_CC);
					}else{
						/*
						*	之所以不赋值为空字符串，是因为指针不是指向一个空的字符串，而是为空
						*/
						zend_update_property_null(NULL, arg_info, "class_name", 10 TSRMLS_CC);
					}
					zend_update_property_long(NULL, arg_info, "class_name_len", 14, func->common.arg_info[i].class_name_len TSRMLS_CC);
					zend_update_property_bool(NULL, arg_info, "type_hint", 9, func->common.arg_info[i].type_hint TSRMLS_CC);
					zend_update_property_bool(NULL, arg_info, "pass_by_reference", 17, func->common.arg_info[i].pass_by_reference TSRMLS_CC);
					zend_update_property_bool(NULL, arg_info, "allow_null", 10, func->common.arg_info[i].allow_null TSRMLS_CC);
					zend_update_property_bool(NULL, arg_info, "is_variadic", 11, func->common.arg_info[i].is_variadic TSRMLS_CC);
					//
					zend_hash_index_update(Z_ARRVAL_P(arg_arr), i, &arg_info, sizeof(zval *), NULL);
				}
				zend_update_property(NULL, func_obj, "arg_info", 8, arg_arr TSRMLS_CC);
				zval_ptr_dtor(&arg_arr);
				//
				zend_hash_index_update(Z_ARRVAL_P(func_arr), index, &func_obj, sizeof(zval *), NULL);
				index++;
			}
		}while( p != func_bucket_end );

		zend_hash_update(Z_ARRVAL_P(retval), "function", 9, &func_arr, sizeof(zval *), NULL);
	}

	// if( symbol_bucket_start != symbol_bucket_end ){
	// 	zend_uint index = 0;
	// 	Bucket *p = symbol_bucket_start;

	// 	zval *symbol_arr;
	// 	MAKE_STD_ZVAL(symbol_arr);
	// 	array_init(symbol_arr);
	// 	do{
	// 		p = p->pListNext;
	// 		zval *symbol;
	// 		MAKE_STD_ZVAL(symbol);
	// 		ZVAL_STRING(symbol, p->arKey, 1);
	// 		//
	// 		zend_hash_index_update(Z_ARRVAL_P(symbol_arr), index, &symbol, sizeof(zval *), NULL);
	// 		index++;
	// 	}while( p != symbol_bucket_end );

	// 	zend_hash_update(Z_ARRVAL_P(retval), "symbol", 7, &symbol_arr, sizeof(zval *), NULL);
	// }

	if (EXPECTED(new_op_array != NULL)) {
		destroy_op_array(new_op_array TSRMLS_CC);
		efree(new_op_array);
	}

	RETURN_ZVAL(retval, 0 , 1);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_idebug_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_idebug_init_globals(zend_idebug_globals *idebug_globals)
{
	idebug_globals->global_value = 0;
	idebug_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(idebug)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(idebug)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(idebug)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(idebug)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(idebug)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "idebug support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ idebug_functions[]
 *
 * Every user visible function must have an entry in idebug_functions[].
 */
const zend_function_entry idebug_functions[] = {
	PHP_FE(idebug_symbol_table,	NULL)		/* For testing, remove later. */
	PHP_FE(idebug_active_symbol_table,	NULL)
	PHP_FE(idebug_function_table,	NULL)
	PHP_FE(idebug_class_table,	NULL)
	PHP_FE(idebug_constant_table,	NULL)
	PHP_FE(idebug_included_files,	NULL)
	PHP_FE(idebug_function_call_stack, NULL)
	PHP_FE(idebug_function_args, NULL)
	PHP_FE(idebug_compile_file, NULL)
	PHP_FE_END	/* Must be the last line in idebug_functions[] */
};
/* }}} */

/* {{{ idebug_module_entry
 */
zend_module_entry idebug_module_entry = {
	STANDARD_MODULE_HEADER,
	"idebug",
	idebug_functions,
	PHP_MINIT(idebug),
	PHP_MSHUTDOWN(idebug),
	PHP_RINIT(idebug),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(idebug),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(idebug),
	PHP_IDEBUG_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_IDEBUG
ZEND_GET_MODULE(idebug)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
