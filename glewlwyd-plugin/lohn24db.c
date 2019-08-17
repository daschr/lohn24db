#include <string.h>
#include <jansson.h>
#include <yder.h>
#include <orcania.h>
#include "glewlwyd-common.h"

#include "lohn24db.h"
#include "pg_auth.h"
#include "helper.h"

const char *user_repl=USER_REPL;
const char *hash_repl=HASH_REPL;

#define DBG(X) y_log_message(Y_LOG_LEVEL_DEBUG, X);

/**
 *
 * Note on the user module
 *
 * The JSON object of the user has the following format concerning the reserved properties:
 * {
 *   "username": string, username of the user (its login), must be at most 128 characters and unique among the current instance
 *   "name": string, full name
 *   "scope": array of strings, scopes available for the user, each scope must be a string of at most 128 characters
 *   "enabled": boolean, if false, the user won't be able to connect
 * }
 *
 * - The username shouldn't be updated after creation
 * - How the password is stored and encrypted is up to the implementation.
 *   Although the password encrypted or not SHOULDN'T be returned in the user object
 * - The scope values mustn't be updated in profile mode, to avoid a user to change his or her own credential
 * - The "enabled" property is mandatory in the returned values of user_module_get_list or user_module_get
 *   If a user doesn't have "enabled":true set, then it will be unavailable for connection
 * 
 * The only mandatory values are username and anabled, other values are optional
 * Other values can be handled by the module, it's up to the implementation
 *
 * struct config_module {
 *   const char              * external_url;    // Absolute url of the glewlwyd service
 *   const char              * login_url;       // Relative url of the login page
 *   const char              * admin_scope;     // Value of the g_admin scope
 *   const char              * profile_scope;   // Value of the g_profile scope
 *   struct _h_connection    * conn;            // Hoel structure to access to the database
 *   digest_algorithm          hash_algorithm;  // Hash algorithm used in Glewlwyd
 *   struct config_elements  * glewlwyd_config; // Pointer to the global config structure
 *                          // Function used to return a user object
 *   json_t               * (* glewlwyd_module_callback_get_user)(struct config_module * config, const char * username);
 *                          // Function used to update a user
 *   int                    (* glewlwyd_module_callback_set_user)(struct config_module * config, const char * username, json_t * j_user);
 *                          // Function used to check the validity of a user's password
 *   int                    (* glewlwyd_module_callback_check_user_password)(struct config_module * config, const char * username, const char * password);
 * };
 *
 */

/**
 * 
 * user_module_load
 * 
 * Executed once when Glewlwyd service is started
 * Used to identify the module and to show its parameters on init
 * You can also use it to load resources that are required once for all
 * instance modules for example
 * 
 * @return value: a json_t * value with the following pattern:
 *                {
 *                  result: number (G_OK on success, another value on error)
 *                  name: string, mandatory, name of the module, must be unique among other scheme modules
 *                  display_name: string, optional, long name of the module
 *                  description: string, optional, description for the module
 *                  parameters: object, optional, parameters description for the module
 *                }
 * 
 *                Example:
 *                {
 *                  result: G_OK,
 *                  name: "mock",
 *                  display_name: "Mock scheme module",
 *                  description: "Mock scheme module for glewlwyd tests",
 *                  parameters: {
 *                    mock-value: {
 *                      type: "string",
 *                      mandatory: true
 *                    }
 *                  }
 *                }
 * 
 * @parameter config: a struct config_module with acess to some Glewlwyd
 *                    service and data
 * 
 */
json_t * user_module_load(struct config_module * config) {
	UNUSED(config);
	y_log_message(Y_LOG_LEVEL_DEBUG, "called load");
	return json_pack("{sisssssss{s{sssb}s{sssb}}}",
                   "result",
                   G_OK,
                   "name",
                   "lohn24db",
                   "display_name",
                   "Lohn24 Database",
                   "description",
                   "Lohn24 Database connection",
                   "parameters",
                     "connectionparams",
                       "type",
                       "string",
                       "mandatory",
                       1,
                     "sql_cmd",
                       "type",
                       "string",
                       "mandatory",
                       1);
}

/**
 * 
 * user_module_unload
 * 
 * Executed once when Glewlwyd service is stopped
 * You can use it to release resources that are required once for all
 * instance modules for example
 * 
 * @return value: G_OK on success, another value on error
 * 
 * @parameter config: a struct config_module with acess to some Glewlwyd
 *                    service and data
 * 
 */
int user_module_unload(struct config_module * config) {
  UNUSED(config);

	y_log_message(Y_LOG_LEVEL_DEBUG, "called unload");
  return G_OK;
}

/**
 * 
 * user_module_init
 * 
 * Initialize an instance of this module declared in Glewlwyd service.
 * If required, you must dynamically allocate a pointer to the configuration
 * for this instance and pass it to *cls
 * 
 * @return value: a json_t * value with the following pattern:
 *                {
 *                  result: number (G_OK on success, G_ERROR_PARAM on input parameters error, another value on error)
 *                  error: array of strings containg the list of input errors, mandatory on result G_ERROR_PARAM, ignored otherwise
 *                }
 * 
 * @parameter config: a struct config_module with acess to some Glewlwyd
 *                    service and data
 * @parameter j_parameters: used to initialize an instance in JSON format
 *                          The module must validate itself its parameters
 * @parameter cls: will contain an allocated void * pointer that will be sent back
 *                 as void * in all module functions
 * 
 */
json_t * user_module_init(struct config_module * mod_conf, int readonly, json_t * j_parameters, void ** cls) {
	UNUSED(readonly);
	UNUSED(mod_conf);

	y_log_message(Y_LOG_LEVEL_DEBUG, "called init");
	conf *config=malloc(sizeof(conf));

	json_t * j_return;
		
	if(json_object_get(j_parameters, "error") == NULL){
		
		size_t slen;
		if((slen=json_string_length(json_object_get(j_parameters, "connectionparams")))){
			char s[slen+1];
			strcpy(s,json_string_value(json_object_get(j_parameters, "connectionparams")));
			for(char *r=strtok(s,";");r != NULL;r=strtok(NULL,";")){
				if(!parse_pg_param(config,r)){
					free_config(config);
					
	y_log_message(Y_LOG_LEVEL_DEBUG, "called connectionparam");
					return json_pack("{sis[s]}", "result", G_ERROR_PARAM, "error", 
						"[connectionparams] cannot parse connectionparam");
				}
			}
			
		}

		if(json_string_length(json_object_get(j_parameters, "sql_cmd"))){
			const char *s = json_string_value(json_object_get(j_parameters, "sql_cmd"));
			if(strlen(s) > BUFSIZE){
				free_config(config);

	y_log_message(Y_LOG_LEVEL_DEBUG, "error sql_cmd");
				return json_pack("{sis[s]}", "result", G_ERROR_PARAM, "error", 
						"length of sql_cmd too great");
			}
			strncpy(config->sql_cmd,s,BUFSIZE);
		}

	
	}else
		j_return=json_pack("{sis[s]}", "result", G_ERROR_PARAM, "error", 
						"Error input parameters");
	
	if(!connect_db(config))
		return json_pack("{sis[s]}", "result", G_ERROR_PARAM, "error", 
						"Could not connect to database");
	

	
	*cls=config;	
	j_return=json_pack("{si}", "result", G_OK);
	return j_return;
}

/**
 * 
 * user_module_close
 * 
 * Close an instance of this module declared in Glewlwyd service.
 * You must free the memory previously allocated in
 * the user_module_init function as void * cls
 * 
 * @return value: G_OK on success, another value on error
 * 
 * @parameter config: a struct config_module with acess to some Glewlwyd
 *                    service and data
 * @parameter cls: pointer to the void * cls value allocated in user_module_init
 * 
 */
int user_module_close(struct config_module * config, void * cls) {
  UNUSED(config);
  y_log_message(Y_LOG_LEVEL_DEBUG, "user_module_close - here");
  free_config((conf *)cls);
  y_log_message(Y_LOG_LEVEL_DEBUG, "user_module_close - success");
  return G_OK;
}

/**
 *
 * user_module_count_total
 *
 * Return the total number of users handled by this module corresponding
 * to the given pattern
 *
 * @return value: The total of corresponding users
 *
 * @parameter config: a struct config_module with acess to some Glewlwyd
 *                    service and data
 * @parameter pattern: The pattern to match for the users. How the
 *                     pattern is used is up to the implementation.
 *                     Glewlwyd recommends to match the pattern with the
 *                     username, name and e-mail value for each users
 * @parameter cls: pointer to the void * cls value allocated in user_module_init
 * 
 */
size_t user_module_count_total(struct config_module * config, const char * pattern, void * cls) {
      	UNUSED(config);
	UNUSED(pattern);
	UNUSED(cls);
	DBG("called count total");
	return 0;
}

/**
 *
 * user_module_get_list
 *
 * Return a list of users handled by this module corresponding
 * to the given pattern between the specified offset and limit
 * These are the user objects returned to the administrator
 *
 * @return value: A list of corresponding users or an empty list
 *                using the following JSON format: {"result":G_OK,"list":[{user object}]}
 *                On error, this function must return another value for "result"
 *
 * @parameter config: a struct config_module with acess to some Glewlwyd
 *                    service and data
 * @parameter pattern: The pattern to match for the users. How the
 *                     pattern is used is up to the implementation.
 *                     Glewlwyd recommends to match the pattern with the
 *                     username, name and e-mail value for each users
 * @pattern offset: The offset to reduce the returned list among the total list
 * @pattern limit: The maximum number of users to return
 * @parameter cls: pointer to the void * cls value allocated in user_module_init
 * 
 */
json_t * user_module_get_list(struct config_module * config, const char * pattern, size_t offset, size_t limit, void * cls) {
	UNUSED(config);
	UNUSED(pattern);
	UNUSED(offset);
	UNUSED(limit);
	UNUSED(cls);
	DBG("get list...");
	return json_pack("{sis[]}", "result", G_OK, "list"); 
}

/**
 *
 * user_module_get
 *
 * Return a user object handled by this module corresponding
 * to the username specified
 * This is the user object returned to the administrator
 *
 * @return value: G_OK and the corresponding user
 *                G_ERROR_NOT_FOUND if username is not found
 *                The returned format is {"result":G_OK,"user":{user object}}
 *                On error, this function must return another value for "result"
 *
 * @parameter config: a struct config_module with acess to some Glewlwyd
 *                    service and data
 * @parameter username: the username to match, must be case insensitive
 * @parameter cls: pointer to the void * cls value allocated in user_module_init
 * 
 */
json_t * user_module_get(struct config_module * config, const char * username, void * cls) {
	UNUSED(config);
	UNUSED(username);
	UNUSED(cls);
	DBG("module get");
	return json_pack("{sis{sssOso}}", "result", G_OK, "user", "username", username, "scope", config->profile_scope, "default-scope"), "enabled", json_true());
}

/**
 *
 * user_module_get_profile
 *
 * Return a user object handled by this module corresponding
 * to the username specified.
 * This is the user object returned to the connected user, may be different from the 
 * user_module_get object format if a connected user must have access to different data
 *
 * @return value: G_OK and the corresponding user
 *                G_ERROR_NOT_FOUND if username is not found
 *                The returned format is {"result":G_OK,"user":{user object}}
 *                On error, this function must return another value for "result"
 *
 * @parameter config: a struct config_module with acess to some Glewlwyd
 *                    service and data
 * @parameter username: the username to match, must be case insensitive
 * @parameter cls: pointer to the void * cls value allocated in user_module_init
 * 
 */
json_t * user_module_get_profile(struct config_module * config, const char * username, void * cls) {
	UNUSED(config);
	UNUSED(username);
	UNUSED(cls);
	DBG("get profile");
      	return json_pack("{si}", "result", G_ERROR_NOT_FOUND);
}

/**
 *
 * user_module_is_valid
 *
 * Validate if a user is valid to save for the specified mode
 *
 * @return value: G_OK if the user is valid
 *                G_ERROR_PARAM and an array containing the errors in string format
 *                The returned format is {"result":G_OK} on success
 *                {"result":G_ERROR_PARAM,"error":["error 1","error 2"]} on error
 *
 * @parameter config: a struct config_module with acess to some Glewlwyd
 *                    service and data
 * @parameter username: the username to match, must be case insensitive
 * @parameter j_user: The user to validate
 * @parameter mode: The mode corresponding to the context, values available are:
 *                  - GLEWLWYD_IS_VALID_MODE_ADD: Add a user by an administrator
 *                    Note: in this mode, the module musn't check for already existing user,
 *                          This is already handled by Glewlwyd
 *                  - GLEWLWYD_IS_VALID_MODE_UPDATE: Update a user by an administrator
 *                  - GLEWLWYD_IS_VALID_MODE_UPDATE_PROFILE: Update a user by him or 
 *                                                           herself in the profile context
 * @parameter cls: pointer to the void * cls value allocated in user_module_init
 * 
 */
json_t * user_module_is_valid(struct config_module * config, const char * username, json_t * j_user, int mode, void * cls) {
	UNUSED(config);
	UNUSED(username);
	UNUSED(j_user);
	UNUSED(mode);
	UNUSED(cls);
	DBG("is valid");
	return json_pack("{si}", "result", G_ERROR_PARAM);
}

/**
 *
 * user_module_add
 *
 * Add a new user by an administrator
 *
 * @return value: G_OK on success
 *                Another value on error
 *
 * @parameter config: a struct config_module with acess to some Glewlwyd
 *                    service and data
 * @parameter j_user: The user to add
 * @parameter cls: pointer to the void * cls value allocated in user_module_init
 * 
 */
int user_module_add(struct config_module * config, json_t * j_user, void * cls) {
	UNUSED(config);
	UNUSED(j_user);
	UNUSED(cls);
	DBG("add");
	return G_ERROR_PARAM;
}

/**
 *
 * user_module_update
 *
 * Update an existing user by an administrator
 *
 * @return value: G_OK on success
 *                Another value on error
 *
 * @parameter config: a struct config_module with acess to some Glewlwyd
 *                    service and data
 * @parameter username: the username to match, must be case insensitive
 * @parameter j_user: The user to update. If this function must replace all values or 
 *                    only the given ones or any other solution is up to the implementation
 * @parameter cls: pointer to the void * cls value allocated in user_module_init
 * 
 */
int user_module_update(struct config_module * config, const char * username, json_t * j_user, void * cls) {
	UNUSED(config);
	UNUSED(username);
	UNUSED(j_user);
	UNUSED(cls);
	DBG("update");
	return G_ERROR_PARAM;
}

/**
 *
 * user_module_update_profile
 *
 * Update an existing user in the profile context
 *
 * @return value: G_OK on success
 *                Another value on error
 *
 * @parameter config: a struct config_module with acess to some Glewlwyd
 *                    service and data
 * @parameter username: the username to match, must be case insensitive
 * @parameter j_user: The user to update. If this function must replace all values or 
 *                    only the given ones or any other solution is up to the implementation
 * @parameter cls: pointer to the void * cls value allocated in user_module_init
 * 
 */
int user_module_update_profile(struct config_module * config, const char * username, json_t * j_user, void * cls) {
	UNUSED(config);
	UNUSED(username);
	UNUSED(j_user);
	UNUSED(cls);
	DBG("update profile");
	return G_ERROR_PARAM;
}

/**
 *
 * user_module_delete
 *
 * Delete an existing user by an administrator
 *
 * @return value: G_OK on success
 *                Another value on error
 *
 * @parameter config: a struct config_module with acess to some Glewlwyd
 *                    service and data
 * @parameter username: the username to match, must be case insensitive
 * @parameter cls: pointer to the void * cls value allocated in user_module_init
 * 
 */
int user_module_delete(struct config_module * config, const char * username, void * cls) {
	UNUSED(config);
	UNUSED(username);
	UNUSED(cls);
	DBG("mod delete");
	return G_ERROR_PARAM;
}

/**
 *
 * user_module_check_password
 *
 * Validate the password of an existing user
 *
 * @return value: G_OK on success
 *                Another value on error
 *
 * @parameter config: a struct config_module with acess to some Glewlwyd
 *                    service and data
 * @parameter username: the username to match, must be case insensitive
 * @parameter password: the password to validate
 * @parameter cls: pointer to the void * cls value allocated in user_module_init
 * 
 */
int user_module_check_password(struct config_module * mod_conf, const char * username, const char * password, void * cls) {
	conf *config=(conf *) cls;
	

	char esc_user[512];
	char esc_user2[512];
	
	str_repl(esc_user2,512,username,"\\","\\\\");
	str_repl(esc_user,512,esc_user2,"'","''");

	return check_password(config,(const char *)esc_user,password) ?  G_OK : G_ERROR_UNAUTHORIZED;

}

/**
 *
 * user_module_update_password
 *
 * Update the password only of an existing user
 *
 * @return value: G_OK on success
 *                Another value on error
 *
 * @parameter config: a struct config_module with acess to some Glewlwyd
 *                    service and data
 * @parameter username: the username to match, must be case insensitive
 * @parameter new_password: the new password
 * @parameter cls: pointer to the void * cls value allocated in user_module_init
 * 
 */
int user_module_update_password(struct config_module * config, const char * username, const char * new_password, void * cls) {
	UNUSED(config);
	UNUSED(username);
	UNUSED(new_password);
	UNUSED(cls);
	return G_ERROR_PARAM;
}
