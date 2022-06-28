#include "test_utils.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

void tstDebugStep(const char* msgOrTemplate, const void *param1, const void *param2, const void *param3, const void *param4, const void *param5) {

    if (!DEBUG_TEST_ENABLE)
		return;

    const char prefix[] = "\n[log: test] ";

    if (param5 != NULL) {
        assert(param1 != NULL && param2 != NULL && param3 != NULL && param4 != NULL);
        printf(prefix);
        printf(msgOrTemplate, *(int *)param1, *(int *)param2, *(int *)param3, *(int *)param4, *(int *)param5);
    
    } else if (param4 != NULL) {
        assert(param1 != NULL && param2 != NULL && param3 != NULL);
        printf(prefix);
        printf(msgOrTemplate, *(int *)param1, *(int *)param2, *(int *)param3, *(int *)param4);

    } else if (param3 != NULL) {
        assert(param1 != NULL && param2 != NULL);
        printf(prefix);
        printf(msgOrTemplate, *(int *)param1, *(int *)param2, *(int *)param3);

    } else if (param2 != NULL) {
        assert(param1 != NULL);
        printf(prefix);
        printf(msgOrTemplate, *(int *)param1, *(int *)param2);

    } else if (param1 != NULL) {
        printf(prefix);
        printf(msgOrTemplate, *(int *)param1);

    } else {
        printf(prefix);
        printf("%s", msgOrTemplate);
    }
}
