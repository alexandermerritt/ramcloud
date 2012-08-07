/* Copyright (c) 2009-2012 Stanford University
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR(S) DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL AUTHORS BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef RAMCLOUD_LOGCABINHELPER_H
#define RAMCLOUD_LOGCABINHELPER_H

#include <Client/Client.h>

#include "Common.h"

namespace RAMCloud {

using LogCabin::Client::Entry;
using LogCabin::Client::EntryId;
using LogCabin::Client::NO_ID;

/**
 * Helper class that provides higher level abstractions
 * to interact with LogCabin.
 */

class LogCabinHelper {
  PUBLIC:
    explicit LogCabinHelper(LogCabin::Client::Log& logCabinLog)
        : logCabinLog(logCabinLog) {}

    template<typename M>
    EntryId
    appendProtoBuf(M& message,
                   const vector<EntryId>& invalidates = vector<EntryId>()) {
        string data;
        message.SerializeToString(&data);

        Entry stateEntry(data.c_str(),
                         uint32_t(data.length() + 1),
                         invalidates);
        EntryId entryId = logCabinLog.append(stateEntry);

        return entryId;
    };

    template<typename M>
    void getProtoBufFromEntryId(EntryId entryId, M& message) {
        vector<Entry> entriesRead = logCabinLog.read(entryId);
        message.ParseFromArray(entriesRead[0].getData(),
                               entriesRead[0].getLength());
    };

  PRIVATE:
    /**
     * Handle to the log interface provided by LogCabin.
     */
    LogCabin::Client::Log& logCabinLog;

    DISALLOW_COPY_AND_ASSIGN(LogCabinHelper);
};

} // namespace RAMCloud

#endif // RAMCLOUD_LOGCABINHELPER_H