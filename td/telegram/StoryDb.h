//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2023
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "td/telegram/DialogId.h"
#include "td/telegram/NotificationId.h"
#include "td/telegram/StoryFullId.h"

#include "td/utils/buffer.h"
#include "td/utils/common.h"
#include "td/utils/Promise.h"
#include "td/utils/Status.h"

#include <memory>
#include <utility>

namespace td {

class SqliteConnectionSafe;
class SqliteDb;

class StoryDbSyncInterface {
 public:
  StoryDbSyncInterface() = default;
  StoryDbSyncInterface(const StoryDbSyncInterface &) = delete;
  StoryDbSyncInterface &operator=(const StoryDbSyncInterface &) = delete;
  virtual ~StoryDbSyncInterface() = default;

  virtual void add_story(StoryFullId story_full_id, int32 expires_at, NotificationId notification_id,
                         BufferSlice data) = 0;

  virtual void delete_story(StoryFullId story_full_id) = 0;

  virtual Result<BufferSlice> get_story(StoryFullId story_full_id) = 0;

  virtual vector<BufferSlice> get_expiring_stories(int32 expires_till, int32 limit) = 0;

  virtual vector<BufferSlice> get_stories_from_notification_id(DialogId dialog_id, NotificationId from_notification_id,
                                                               int32 limit) = 0;

  virtual Status begin_write_transaction() = 0;
  virtual Status commit_transaction() = 0;
};

class StoryDbSyncSafeInterface {
 public:
  StoryDbSyncSafeInterface() = default;
  StoryDbSyncSafeInterface(const StoryDbSyncSafeInterface &) = delete;
  StoryDbSyncSafeInterface &operator=(const StoryDbSyncSafeInterface &) = delete;
  virtual ~StoryDbSyncSafeInterface() = default;

  virtual StoryDbSyncInterface &get() = 0;
};

class StoryDbAsyncInterface {
 public:
  StoryDbAsyncInterface() = default;
  StoryDbAsyncInterface(const StoryDbAsyncInterface &) = delete;
  StoryDbAsyncInterface &operator=(const StoryDbAsyncInterface &) = delete;
  virtual ~StoryDbAsyncInterface() = default;

  virtual void add_story(StoryFullId story_full_id, int32 expires_at, NotificationId notification_id, BufferSlice data,
                         Promise<Unit> promise) = 0;

  virtual void delete_story(StoryFullId story_full_id, Promise<Unit> promise) = 0;

  virtual void get_story(StoryFullId story_full_id, Promise<BufferSlice> promise) = 0;

  virtual void get_expiring_stories(int32 expires_till, int32 limit, Promise<vector<BufferSlice>> promise) = 0;

  virtual void get_stories_from_notification_id(DialogId dialog_id, NotificationId from_notification_id, int32 limit,
                                                Promise<vector<BufferSlice>> promise) = 0;

  virtual void close(Promise<Unit> promise) = 0;
  virtual void force_flush() = 0;
};

Status init_story_db(SqliteDb &db, int version) TD_WARN_UNUSED_RESULT;
Status drop_story_db(SqliteDb &db, int version) TD_WARN_UNUSED_RESULT;

std::shared_ptr<StoryDbSyncSafeInterface> create_story_db_sync(std::shared_ptr<SqliteConnectionSafe> sqlite_connection);

std::shared_ptr<StoryDbAsyncInterface> create_story_db_async(std::shared_ptr<StoryDbSyncSafeInterface> sync_db,
                                                             int32 scheduler_id = -1);

}  // namespace td
