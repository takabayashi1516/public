
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libmongo_util.h>


int main(int argc, char *argv[])
{
	CMongoUtil::initialize();

	CMongoUtil db("test_db", "test_collection");
	CMongoUtil db2;

	bool rc;

	char tes1[] = "{\"test\": {\"data\": \"11111\", \"timestamp\": {\"$date\": \"2019-02-19T01:00:00.000Z\"}}}";
	rc = db.insertDocument((const unsigned char*)tes1);
	printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);

	char tes2[] = "{\"test\": {\"data\": \"22222\", \"timestamp\": {\"$date\": \"2019-02-19T02:00:00.000Z\"}}}";
	rc = db.insertDocument((const unsigned char*)tes2);
	printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);

	char tes_[] = "{\"test_1\": \"33333\", \"test_a\": \"55555\"}";
#if 0
	rc = db.updateDocument(argv[1], (const unsigned char*)tes_);
	printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);

	rc = db.deleteDocument(argv[2]);
	printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);
#else
	char tes3[] = "{\"test\": {\"data\": \"33333\", \"timestamp\": {\"$date\": \"2019-02-19T03:00:00.000Z\"}}}";
	rc = db.insertDocument((const unsigned char*)tes3);
	printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);

	char tes4[] = "{\"test\": {\"data\": \"44444\", \"timestamp\": {\"$date\": \"2019-02-19T04:00:00.000Z\"}}}";
	rc = db.insertDocument((const unsigned char*)tes4);
	printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);
#endif
	char qry[] = "{}";
	char **doc;
	size_t cnt = db.find((const uint8_t *)qry, doc, (const uint8_t *)"{\"test.timestamp\": -1}");
	printf("l(%4d): %s: cnt=%d\n", __LINE__, __FUNCTION__, cnt);
	for (size_t i = 0; i < cnt; i++) {
		printf("l(%4d): %s: doc[%d]=%s\n", __LINE__, __FUNCTION__, i, doc[i]);
	}
	db.freeFindResult(doc, cnt);

	// -------------------------------------------------------------------------

	rc = db2.getDatabase("test2_db");
	printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);

	rc = db2.createCollection("test2_collection");
	printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);

	rc = db2.insertDocument((const unsigned char*)tes1);
	printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);

	rc = db2.insertDocument((const unsigned char*)tes2);
	printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);

	rc = db2.updateDocument(argv[1], (const unsigned char*)tes_);
	printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);

	rc = db2.deleteDocument(argv[2]);
	printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);

	cnt = db2.find((const unsigned char*)qry, doc);
	printf("l(%4d): %s: cnt=%d\n", __LINE__, __FUNCTION__, cnt);
	for (size_t i = 0; i < cnt; i++) {
		printf("l(%4d): %s: doc[%d]=%s\n", __LINE__, __FUNCTION__, i, doc[i]);
	}
	db2.freeFindResult(doc, cnt);

#if 1
	rc = db2.dropCollection();
	printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);

	rc = db2.dropDatabase();
	printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);
#endif

	CMongoUtil::finalize();

	return 0;
}
