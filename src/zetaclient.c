
#include "zetaclient.h"
#include <string.h>
#include "cometd_internal.h"

zeta_handshake_manager_t * zeta_create_handshake_manager(const char * businessId, const char * deploymentId, const char * authType, const char * authVersion, zeta_handshake_fields handshakeFields) {
	CMTD_TRACE_IN
	CALLOC(zeta_handshake_manager_t, man);
	man->businessId = businessId;
	man->deploymentId = deploymentId;
	man->authType = authType;
	man->authVersion = authVersion;
	man->handshakeFields = handshakeFields;
	CMTD_RETURN(man);
}
static json_t * zeta_common_handshake_fields(zeta_client_t * zetaClient) {
	json_t * handshakeFields = json_object();
	json_t * ext = json_object();
	json_t * auth = json_object();
	json_t * data = json_object();
	json_object_set_new(auth, "data", data);
	json_object_set_new(auth, "action", json_string_nocheck("authenticate"));
	zeta_handshake_manager_t * hm = zetaClient->handshakeManager;
	char * type = cometd_malloc_fn(strlen(hm->businessId) + strlen(hm->deploymentId) + strlen(hm->authType) + 3);
	strcpy(type, hm->businessId);
	strcat(type, ".");
	strcat(type, hm->deploymentId);
	strcat(type, ".");
	strcat(type, hm->authType);
	json_object_set_new(auth, "type", json_string_nocheck(type));
	json_object_set_new(auth, "resource", json_string_nocheck(zetaClient->resource));
	json_object_set_new(auth, "version", json_string_nocheck("none"));
	json_object_set_new(ext, "authentication", auth);
	json_object_set_new(handshakeFields, "ext", ext);
	return handshakeFields;
}
static json_t * zeta_mem_handshake_fields(zeta_client_t * zetaClient) {
	json_t * handshakeFields = zeta_common_handshake_fields(zetaClient);
	json_t * data = json_object_get(json_object_get(json_object_get(handshakeFields, "ext"), "authentication"), "data");
	cometd_array_t* mydata = zetaClient->handshakeManager->data;
	json_object_set_new(data, "login", json_string_nocheck(mydata->array[0]));
	json_object_set_new(data, "password", json_string_nocheck(mydata->array[1]));
	return handshakeFields;
}
zeta_handshake_manager_t * zeta_create_mem_handshake_manager(const char * businessId, const char * deploymentId, const char * login, const char * password) {
	CMTD_TRACE_IN
	zeta_handshake_manager_t * man = zeta_create_handshake_manager(businessId, deploymentId, "simple", "none", &zeta_mem_handshake_fields);
	cometd_array_add((void*)login, &man->data);
	cometd_array_add((void*)password, &man->data);
	CMTD_RETURN(man);
}

zeta_client_t * zeta_create_client(const char * serverUrl, cometd_transport_t * transport, const char * businessId, zeta_handshake_manager_t * handshakeManager) {
	CMTD_TRACE_IN
	CALLOC(zeta_client_t, client);
	client->cometClient = cometd_create_client(serverUrl, transport);
	client->businessId = businessId;
	client->handshakeManager = handshakeManager;
	client->resource = "todo";

	CMTD_RETURN(client);
}
bool zeta_handshake(zeta_client_t * client) {
	CMTD_TRACE_IN
	CMTD_RETURN(cometd_handshake(client->cometClient, client->handshakeManager->handshakeFields(client)));
}
