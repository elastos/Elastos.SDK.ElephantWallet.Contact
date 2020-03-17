#include <RemoteStorageManager.hpp>
#include "RemoteStorageManager.hpp"

#include <Log.hpp>
#include <ErrCode.hpp>

namespace elastos {

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */

/* =========================================== */
/* === static function implement ============= */
/* =========================================== */

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */
void RemoteStorageManager::addClient(ClientType type, std::shared_ptr<RemoteStorageClient> client)
{
    mRemoteStorageClientMap[type] = client;
}

int RemoteStorageManager::cacheProperty(const char* key, const std::string& value,
                                        const std::string& savedAt, const std::string& extra)
{
    for (const auto& [type, client]: mRemoteStorageClientMap){
        int ret = client->cacheProperty(key, value, savedAt, extra);
        CHECK_ERROR(ret);
    }

    return 0;
}

int RemoteStorageManager::uploadCachedProp()
{
    for (const auto& [type, client]: mRemoteStorageClientMap){
        int ret = client->uploadCachedProp();
        CHECK_ERROR(ret);
    }

    return 0;
}

/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */


/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */


} // namespace elastos
