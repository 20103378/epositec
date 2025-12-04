
#pragma once
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include "utils/singleton.h"

#define DB_LABEL_NTRIP_EXPIRES_TIME "db_ntrip_expires_time"
#define DB_LABEL_NTRIP_INFO "db_ntrip_info"

//sqlite3 viot.db "SELECT label, value FROM viotTable WHERE label LIKE 'db_ntrip%';"
