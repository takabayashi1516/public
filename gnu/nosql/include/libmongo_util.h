#ifndef __LIBMONGO_UTIL_H
#define __LIBMONGO_UTIL_H

#include <mongoc.h>
#include <bcon.h>

#ifdef __cplusplus


/**
 */
class CMongoUtilBase {
private:
	mongoc_client_t		*m_pobjClient;		///<
	mongoc_uri_t		*m_pobjUri;			///<
	mongoc_database_t	*m_pobjDatabase;	///<
	mongoc_collection_t	*m_pobjCollection;	///<

	const char			*m_cpszUri;			///<

public:
	///
	static void initialize();
	///
	static void finalize();

	///
	bool insertDocument(const uint8_t *a_pszJson);
	///
	bool insertDocumentArray(const uint8_t *a_pszJsons[], size_t a_nDocuments);
	///
	bool updateDocument(const char a_szOid[], const uint8_t *a_pszJson);
	///
	bool deleteDocument(const char a_szOid[]);

	///
	size_t find(const uint8_t *a_pszJsonQuery, char **&a_ppszDocuments,
			const uint8_t *a_pszSortDocument = NULL);
	///
	void freeFindResult(char **&a_pszDocuments, size_t a_nDocuments);

	///
	bool createCollection(const char a_szCollection[]);
	///
	bool dropCollection();

	///
	bool getDatabase(const char a_szDatabase[]);
	///
	bool dropDatabase();

protected:
	///
	CMongoUtilBase(const char *a_cpszUri, const char a_szDatabase[],
			const char a_szCollection[]);
	///
	CMongoUtilBase(const char *a_cpszUri);
	///
	virtual ~CMongoUtilBase();

	///
	virtual int prepare(const char *a_pszDatabase, const char *a_pszCollection);
};

/**
 */
class CMongoUtil : public CMongoUtilBase {
public:
	///
	CMongoUtil();
	///
	CMongoUtil(const char a_szDatabase[], const char a_szCollection[]);
	///
	virtual ~CMongoUtil();
};

#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */


#endif /* __LIBMONGO_UTIL_H */
