#include "viot_event_commercial.h"
#include "viot_log.h"
#include "viot_http_commercial.h"
#include "viot_shared_commercial.h"
#include "viot_db_commercial.h"
#include "viot_ntrip_commercial.h"
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <string>
#include <third_party/nlohmann/json.hpp>

typedef struct
{
    pthread_t evid;
    viot_event_status_t ev_status;
    pthread_mutex_t mutex;
} viot_ev_ctx;

static viot_ev_ctx gEv;
static viot_event_status_t viot_event_status_get()
{
    viot_ev_ctx *ctx = &gEv;
    return ctx->ev_status;
}
static std::string viot_event_status_str(viot_event_status_t status)
{
    switch (status)
    {
    case VIOT_EVENT_NONE:
        return "VIOT_EVENT_NONE";
    case VIOT_EVENT_NEED_NTRIP_INFO:
        return "VIOT_EVENT_NEED_NTRIP_INFO";
    case VIOT_EVENT_FINISH:
        return "VIOT_EVENT_FINISH";
    case VIOT_EVENT_NTRIP_CONNECT:
        return "VIOT_EVENT_NTRIP_CONNECT";
    case VIOT_EVENT_NTRIP_DISCONNECT:
        return "VIOT_EVENT_NTRIP_DISCONNECT";
    case VIOT_EVENT_NTRIP_DATA_RECEIVED:
        return "VIOT_EVENT_NTRIP_DATA_RECEIVED";
    case VIOT_EVENT_GGA_QUALITY_CHANGED:
        return "VIOT_EVENT_GGA_QUALITY_CHANGED";
    case VIOT_EVENT_SERVICE_SWITCH:
        return "VIOT_EVENT_SERVICE_SWITCH";
    case VIOT_EVENT_MOWER_STATUS_CHANGED:
        return "VIOT_EVENT_MOWER_STATUS_CHANGED";
    case VIOT_EVENT_CUSTOM:
        return "VIOT_EVENT_CUSTOM";
    default:
        return "UNKNOWN_STATUS";
    }
}
void viot_event_set_status(viot_event_status_t status)
{
    viot_ev_ctx *ctx = &gEv;
    pthread_mutex_lock(&ctx->mutex);
    ctx->ev_status = status;
    VIOT_PrintSaveLOG("Event status set to:" + viot_event_status_str(status));
    pthread_mutex_unlock(&ctx->mutex);
}

static void *viot_event_worker(void *arg)
{
    VIOT_PrintSaveLOG("viot_event_worker started");
    viot_event_status_t status_old = VIOT_EVENT_NONE;
    while (1)
    {
        int rc = 0;
        viot_event_status_t status = viot_event_status_get();
        if (status != status_old)
        {
            VIOT_PrintSaveLOG("Event status changed: %d -> %d", status_old, status);
            status_old = status;
        }
        switch (status)
        {
        case VIOT_EVENT_NONE:
        case VIOT_EVENT_FINISH:
        {
            usleep(100000); // 100ms
            break;
        }
        case VIOT_EVENT_NEED_NTRIP_INFO:
        {
            viot::ntrip::viot_get_ntrip_info();
            viot_event_set_status(VIOT_EVENT_FINISH);
            break;
        }

        default:
            break;
        }
    }
    return NULL;
}

int viot_event_start()
{
    VIOT_PrintSaveLOG("viot_event_start called");
    int rc;
    viot_ev_ctx *pEv = &gEv;
    memset(pEv, 0, sizeof(viot_ev_ctx));
    rc = pthread_mutex_init(&pEv->mutex, NULL);
    if (rc != 0)
    {
        VIOT_PrintSaveLOG("pthread_mutex_init failed: %d", rc);
        return -1;
    }
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    rc = pthread_create(&pEv->evid, &attr, viot_event_worker, NULL);
    if (rc != 0)
    {
        VIOT_PrintSaveLOG("pthread_create failed: %d", rc);
        pthread_mutex_destroy(&pEv->mutex);
        return -1;
    }
    pthread_attr_destroy(&attr);

    return rc;
}
