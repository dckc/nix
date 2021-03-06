#pragma once

#include "types.hh"

#include <string>

namespace nix {

struct DownloadOptions
{
    string expectedETag;
    bool verifyTLS{true};
    bool forceProgress{false};
};

struct DownloadResult
{
    bool cached;
    string data, etag;
};

class Store;

DownloadResult downloadFile(string url, const DownloadOptions & options);

Path downloadFileCached(ref<Store> store, const string & url, bool unpack);

MakeError(DownloadError, Error)

bool isUri(const string & s);

}
