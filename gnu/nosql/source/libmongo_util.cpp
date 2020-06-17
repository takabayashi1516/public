
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <libmongo_util.h>

typedef bson_t *pbson_t;
typedef char *pchar;


/**
 */
CMongoUtil::CMongoUtil(const char a_szDatabase[], const char a_szCollection[])
	:	CMongoUtilBase("mongodb://localhost:27017", a_szDatabase, a_szCollection)
{
}

/**
 */
CMongoUtil::CMongoUtil() : CMongoUtilBase("mongodb://localhost:27017")
{
}

/**
 */
CMongoUtil::~CMongoUtil()
{
}

/**
 */
void CMongoUtilBase::initialize()
{
	::mongoc_init();
}

/**
 */
void CMongoUtilBase::finalize()
{
	::mongoc_cleanup();
}

/**
 */
bool CMongoUtilBase::insertDocument(const uint8_t *a_pszJson)
{
	bool result = false;
	if (!m_pobjCollection) {
		return result;
	}
	bson_error_t err;
	bson_t *doc = ::bson_new_from_json(a_pszJson, -1, &err);
	if (!doc) {
		::printf("l(%4d): %s: %s\n", __LINE__, __PRETTY_FUNCTION__, err.message);
		return result;
	}
	result = ::mongoc_collection_insert_one(m_pobjCollection, doc, NULL, NULL, &err);
	if (!result) {
		::printf("l(%4d): %s: %s\n", __LINE__, __PRETTY_FUNCTION__, err.message);
	}
	::bson_destroy(doc);
	return result;
}

/**
 */
bool CMongoUtilBase::insertDocumentArray(const uint8_t *a_pszJsons[], size_t a_nDocuments)
{
	bool result = false;
	if (a_nDocuments <= 0u) {
		return result;
	}
	if (!m_pobjCollection) {
		return result;
	}
	pbson_t *array = new pbson_t[a_nDocuments];
	bson_error_t err;
	size_t i;
	for (i = 0u; i < a_nDocuments; i++) {
		if (!a_pszJsons[i]) {
			break;
		}
		array[i] = ::bson_new_from_json(a_pszJsons[i], -1, &err);
		if (!array[i]) {
			break;
		}
	}
	result = ::mongoc_collection_insert_many(m_pobjCollection, (const bson_t **)array, a_nDocuments, NULL, NULL, &err);
	if (!result) {
		::printf("l(%4d): %s: %s\n", __LINE__, __PRETTY_FUNCTION__, err.message);
	}
	for (i = 0u; i < a_nDocuments; i++) {
		if (array[i]) {
			::bson_destroy(array[i]);
		}
	}
	return result;
}

/**
 */
bool CMongoUtilBase::updateDocument(const char a_szOid[], const uint8_t *a_pszJson)
{
	bool result = false;
	if (!m_pobjCollection) {
		return result;
	}
	if (!::bson_oid_is_valid(a_szOid, ::strlen(a_szOid))) {
		return result;
	}
	bson_oid_t oid;
	::bson_oid_init_from_string(&oid, a_szOid);

	bson_t *query = ::bson_new();
	BSON_APPEND_OID(query, "_id", &oid);
	bson_error_t err;
	bson_t *update = ::bson_new_from_json(a_pszJson, -1, &err);
	BSON_APPEND_OID(update, "_id", &oid);

//	result = ::mongoc_collection_update_one(m_pobjCollection, query, update, NULL, NULL, &err);
	result = ::mongoc_collection_replace_one(m_pobjCollection, query, update, NULL, NULL, &err);
	if (!result) {
		::printf("l(%4d): %s: %s\n", __LINE__, __PRETTY_FUNCTION__, err.message);
	}

	::bson_destroy(query);
	::bson_destroy(update);

	return result;
}

/**
 */
bool CMongoUtilBase::deleteDocument(const char a_szOid[])
{
	bool result = false;
	if (!m_pobjCollection) {
		return result;
	}
	if (!::bson_oid_is_valid(a_szOid, ::strlen(a_szOid))) {
		return result;
	}
	bson_oid_t oid;
	::bson_oid_init_from_string(&oid, a_szOid);

	bson_t *doc = ::bson_new();
	BSON_APPEND_OID(doc, "_id", &oid);

	bson_error_t err;
	result = ::mongoc_collection_delete_one(m_pobjCollection, doc, NULL, NULL, &err);
	if (!result) {
		::printf("l(%4d): %s: %s\n", __LINE__, __PRETTY_FUNCTION__, err.message);
	}
	::bson_destroy(doc);

	return result;
}

/**
 */
size_t CMongoUtilBase::find(const uint8_t *a_pszJsonQuery,
		char **&a_ppszDocuments, const uint8_t *a_pszSortDocument)
{
	size_t result = 0u;
	if (!m_pobjCollection) {
		return result;
	}
	bson_error_t err;
	bson_t *query = ::bson_new_from_json(a_pszJsonQuery, -1, &err);
	if (!query) {
		return result;
	}
	bson_t *opt = NULL;
	if (a_pszSortDocument) {
		char szOpt[512];
		::sprintf(szOpt, "{\"sort\": %s}", a_pszSortDocument);
		opt = ::bson_new_from_json(reinterpret_cast<uint8_t *>(const_cast<char *>(szOpt)), -1, &err);
		if (!opt) {
			return result;
		}
	}
	mongoc_cursor_t *cursor = ::mongoc_collection_find_with_opts(m_pobjCollection, query, opt, NULL);
	if (opt) {
		::bson_destroy(opt);
	}
	bson_t *doc;
	while (::mongoc_cursor_next(cursor, (const bson_t **)(&doc))) {
//		char *str = ::bson_as_canonical_extended_json(doc, NULL);
		char *str = ::bson_as_json(doc, NULL);
		if (!str) {
			break;
		}
		if (result <= 0u) {
			a_ppszDocuments = new pchar[result + 1];
		} else {
			pchar *tmp = a_ppszDocuments;
			a_ppszDocuments = new pchar[result + 1];
			::memcpy(a_ppszDocuments, tmp, sizeof(pchar) * result);
			delete [] tmp;
		}
		a_ppszDocuments[result] = str;
		result++;
//		bson_free(str);	/* -> freeFindResult() */
	}
	::bson_destroy(query);
	::mongoc_cursor_destroy(cursor);
	return result;
}

/**
 */
void CMongoUtilBase::freeFindResult(char **&a_pszDocuments, size_t a_nDocuments)
{
	for (size_t i = 0u; i < a_nDocuments; i++) {
		if (a_pszDocuments[i]) {
			::bson_free(a_pszDocuments[i]);
		}
	}
	delete [] a_pszDocuments;
}

/**
 */
bool CMongoUtilBase::createCollection(const char a_szCollection[])
{
	bool result = false;
	if (m_pobjCollection) {
		return result;
	}
	if (!m_pobjDatabase) {
		return result;
	}
	bson_error_t err;
	m_pobjCollection = ::mongoc_database_create_collection(m_pobjDatabase, a_szCollection, NULL, &err);
	if (m_pobjCollection) {
		result = true;
	}
	return result;
}

/**
 */
bool CMongoUtilBase::dropCollection()
{
	bool result = false;
	if (!m_pobjCollection) {
		return result;
	}
	bson_error_t err;
	result = ::mongoc_collection_drop(m_pobjCollection, &err);
	if (!result) {
		::printf("l(%4d): %s: %s\n", __LINE__, __PRETTY_FUNCTION__, err.message);
	} else {
		m_pobjCollection = NULL;
		result = true;
	}
	return result;
}

/**
 */
bool CMongoUtilBase::getDatabase(const char a_szDatabase[])
{
	bool result = false;
	if (m_pobjDatabase) {
		return result;
	}
	if (!m_pobjClient) {
		return result;
	}
	m_pobjDatabase = ::mongoc_client_get_database(m_pobjClient, a_szDatabase);
	if (m_pobjDatabase) {
		result = true;
	}
	return result;
}

/**
 */
bool CMongoUtilBase::dropDatabase()
{
	bool result = false;
	if (!m_pobjDatabase) {
		return result;
	}
	bson_error_t err;
	result = ::mongoc_database_drop(m_pobjDatabase, &err);
	if (!result) {
		::printf("l(%4d): %s: %s\n", __LINE__, __PRETTY_FUNCTION__, err.message);
	}
	m_pobjDatabase = NULL;
	m_pobjCollection = NULL;
	return result;
}

/**
 */
CMongoUtilBase::CMongoUtilBase(const char *a_cpszUri, const char a_szDatabase[],
			const char a_szCollection[])
	:	m_pobjClient(NULL), m_pobjUri(NULL), m_pobjDatabase(NULL),
			m_pobjCollection(NULL), m_cpszUri(a_cpszUri)
{
	prepare(a_szDatabase, a_szCollection);
}

/**
 */
CMongoUtilBase::CMongoUtilBase(const char *a_cpszUri)
	:	m_pobjClient(NULL), m_pobjUri(NULL), m_pobjDatabase(NULL),
			m_pobjCollection(NULL), m_cpszUri(a_cpszUri)
{
	prepare(NULL, NULL);
}

/**
 */
CMongoUtilBase::~CMongoUtilBase()
{
	if (m_pobjCollection) {
		::mongoc_collection_destroy(m_pobjCollection);
	}
	if (m_pobjDatabase) {
		::mongoc_database_destroy(m_pobjDatabase);
	}
	if (m_pobjUri) {
		::mongoc_uri_destroy(m_pobjUri);
	}
	if (m_pobjClient) {
		::mongoc_client_destroy(m_pobjClient);
	}
}

/**
 */
int CMongoUtilBase::prepare(const char *a_pszDatabase, const char *a_pszCollection)
{
	int result = -1;

	bson_error_t err;
	m_pobjUri = ::mongoc_uri_new_with_error(m_cpszUri, &err);
	if (!m_pobjUri) {
		::printf("l(%4d): %s: %s\n", __LINE__, __PRETTY_FUNCTION__, err.message);
		assert(0);
		return result;
	}

	m_pobjClient = ::mongoc_client_new_from_uri(m_pobjUri);
	if (!m_pobjClient) {
		assert(0);
		return result;
	}

	if (!a_pszDatabase) {
		result = 0;
		return result;
	}

	m_pobjDatabase = ::mongoc_client_get_database(m_pobjClient, a_pszDatabase);
	if (!m_pobjDatabase) {
		assert(0);
		return result;
	}

	if (!a_pszCollection) {
		result = 0;
		return result;
	}

	m_pobjCollection = ::mongoc_client_get_collection(m_pobjClient, a_pszDatabase, a_pszCollection);
	if (!m_pobjCollection) {
		assert(0);
		return result;
	}
	result = 0;

	return result;
}
