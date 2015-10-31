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
void hash_table_key_value(HashTable *table, char *name TSRMLS_DC){
	HashPosition pos;
	zval *var;

	pos = table->pListHead;
	php_printf("%s: Array( ",name);
	while(pos){
		php_printf("'%s'",pos->arKey);
		php_printf(" => ");
		var = (zval*)pos->pDataPtr?pos->pDataPtr:pos->pData; 
		switch(Z_TYPE_P(var)){
			case IS_NULL:
				php_printf("NULL");
				break;
			case IS_LONG:
				php_printf("%ld",Z_LVAL_P(var));
				break;
			case IS_DOUBLE:
				php_printf("%.8f",Z_DVAL_P(var));
				break;
			case IS_BOOL:
				Z_BVAL_P(var)?php_printf("TRUE"):php_printf("FALSE");
				break;	
			case IS_ARRAY:
				php_printf("Array");
				break;
			case IS_OBJECT:
				php_printf("OBJECT");
				break;
			case IS_STRING:
				php_printf("'%s'",Z_STRVAL_P(var));
				break;
			case IS_RESOURCE:
				php_printf("RESOURCE");
				break;
			case IS_CONSTANT:
				php_printf("CONSTANT");
				break;
			case IS_CONSTANT_AST:
				php_printf("CONSTANT_AST");
				break;
			case IS_CALLABLE:
				php_printf("CALLABLE");
				break;
			default:
				php_printf("UNKNOW");
		}
		if(pos->pListNext) php_printf(", ");
		pos = pos->pListNext;
	}
	php_printf(" )\n");
}

void hash_table_key(HashTable *table, char *name TSRMLS_DC){
	HashPosition pos;

	pos = table->pListHead;
	php_printf("%s: Array( ",name);
	while(pos){
		php_printf("'%s'",pos->arKey);
		if(pos->pListNext) php_printf(", ");
		pos = pos->pListNext;
	}
	php_printf(" )\n");
}

PHP_FUNCTION(idebug_symbol_table)
{
	hash_table_key_value(&EG(symbol_table), "symbol_table" TSRMLS_CC);
}

PHP_FUNCTION(idebug_active_symbol_table)
{
	if(!EG(active_symbol_table)){
		zend_rebuild_symbol_table(TSRMLS_C);
	}
	if(EG(active_symbol_table)){
		hash_table_key_value(EG(active_symbol_table), "active_symbol_table" TSRMLS_CC);
	}else{
		php_printf("active_symbol_table: NULL\n");
	}
}

PHP_FUNCTION(idebug_function_table)
{
	if(EG(function_table)){
		hash_table_key(EG(function_table), "function_table" TSRMLS_CC);
	}else{
		php_printf("function_table: NULL\n");
	}
}

PHP_FUNCTION(idebug_class_table)
{
	if(EG(class_table)){
		hash_table_key(EG(class_table), "class_table" TSRMLS_CC);
	}else{
		php_printf("class_table: NULL\n");
	}
}

PHP_FUNCTION(idebug_constant_table)
{
	if(EG(zend_constants)){
		hash_table_key_value(EG(zend_constants), "constant_table" TSRMLS_CC);
	}else{
		php_printf("constant_table: NULL\n");
	}
}

PHP_FUNCTION(idebug_included_files)
{
	hash_table_key(&EG(included_files), "included_files_table" TSRMLS_CC);
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
	php_printf("function_stack: Stack( ");
	while(p){
		q = p;
		php_printf("'%s'",((zend_execute_data*)p->data)->function_state.function->common.function_name);
		if(p->next) php_printf(" => ");
		p= p->next;
		efree(q);
	}
	php_printf(" )\n");
}

PHP_FUNCTION(idebug_function_args)
{
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

	php_printf("function_args: Args( ");
	while(num_args>0){
		arg = *(zval**)(p-num_args);
		switch(Z_TYPE_P(arg)){
			case IS_NULL:
				php_printf("NULL");
				break;
			case IS_LONG:
				php_printf("%ld",Z_LVAL_P(arg));
				break;
			case IS_DOUBLE:
				php_printf("%.8f",Z_DVAL_P(arg));
				break;
			case IS_BOOL:
				Z_BVAL_P(arg)?php_printf("TRUE"):php_printf("FALSE");
				break;	
			case IS_ARRAY:
				php_printf("Array");
				break;
			case IS_OBJECT:
				php_printf("OBJECT");
				break;
			case IS_STRING:
				php_printf("'%s'",Z_STRVAL_P(arg));
				break;
			case IS_RESOURCE:
				php_printf("RESOURCE");
				break;
			case IS_CONSTANT:
				php_printf("CONSTANT");
				break;
			case IS_CONSTANT_AST:
				php_printf("CONSTANT_AST");
				break;
			case IS_CALLABLE:
				php_printf("CALLABLE");
				break;
			default:
				php_printf("UNKNOW");
		}
		if(--num_args>0) php_printf(" , ");
	}
	php_printf(" )\n");
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
