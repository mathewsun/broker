/*
 * Copyright 2014-present IVK JSC. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Exchange.h"
#include <Poco/String.h>
#include <Poco/StringTokenizer.h>
#include <sstream>
#include <fake_cpp14.h>
#include "Broker.h"
#include "MiscDefines.h"

namespace upmq {
namespace broker {

Exchange::Exchange()
    : _destinationsT("\"" + BROKER::Instance().id() + "_destinations\""),
      _mutexDestinations(THREADS_CONFIG.subscribers),
      _conditionDestinations(_mutexDestinations.size()),
      _threadPool("exchange", 1, static_cast<int>(_mutexDestinations.size()) + 1) {
  std::stringstream sql;
  sql << "create table if not exists " << _destinationsT << "("
      << " id text not null primary key"
      << ",name text not null"
      << ",type int not null"
      << ",create_time timestamp not null default current_timestamp"
      << ",subscriptions_count int not null default 0"
      << ",constraint \"" << BROKER::Instance().id() << "_destinations_index\" unique (name, type)"
      << ")"
      << ";";
  TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't init exchange", sql.str(), ERROR_STORAGE);
  sql.str("");
  sql << " create table if not exists " << STORAGE_CONFIG.messageJournal() << "("
      << "    message_id text not null primary key"
      << "   ,uri text not null"
      << "   ,body_type int"
      << "   ,subscribers_count int not null default 0"
      << ");";
  TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't init exchange", sql.str(), ERROR_STORAGE);
}
Exchange::~Exchange() = default;
Destination &Exchange::destination(const std::string &uri, Exchange::DestinationCreationMode creationMode) const {
  std::string mainDP;
  if (uri.find("://") != std::string::npos) {
    mainDP = mainDestinationPath(uri);
  } else {
    mainDP = uri;
  }
  switch (creationMode) {
    case DestinationCreationMode::NO_CREATE: {
      upmq::ScopedReadRWLock readRWLock(_destinationsLock);
      return getDestination(mainDP);
    }
    case DestinationCreationMode::CREATE: {
      {
        upmq::ScopedReadRWLock readRWLock(_destinationsLock);
        auto it = _destinations.find(mainDP);
        if (it != _destinations.end()) {
          return *it->second;
        }
      }

      upmq::ScopedWriteRWLock writeRwLock(_destinationsLock);

      auto it = _destinations.find(mainDP);
      if (it == _destinations.end()) {
        // FIXME: if mainDP isn't uri then createDestination throw exception
        _destinations.insert(std::make_pair(mainDP, DestinationFactory::createDestination(*this, uri)));
        it = _destinations.find(mainDP);
      }

      return *it->second;
    }
  }
  throw EXCEPTION("invalid creation mode", std::to_string(static_cast<int>(creationMode)), ERROR_UNKNOWN);
}
Destination &Exchange::getDestination(const std::string &id) const {
  auto it = _destinations.find(id);
  if (it == _destinations.end()) {
    throw EXCEPTION("destination not found", id, ERROR_UNKNOWN);
  }
  return *it->second;
}
void Exchange::deleteDestination(const std::string &uri) {
  upmq::ScopedWriteRWLock writeRWLock(_destinationsLock);
  std::string mainDP = mainDestinationPath(uri);
  auto it = _destinations.find(mainDP);
  if (it != _destinations.end()) {
    _destinations.erase(it);
  }
}
std::string Exchange::mainDestinationPath(const std::string &uri) {
  Poco::StringTokenizer URI(uri, ":", Poco::StringTokenizer::TOK_TRIM);
  return DestinationFactory::destinationTypePrefix(uri) + DestinationFactory::destinationName(uri);
}
void Exchange::saveMessage(const Session &session, const MessageDataContainer &sMessage) {
  std::stringstream sql;
  const Proto::Message &message = sMessage.message();
  Destination &dest = destination(message.destination_uri(), DestinationCreationMode::NO_CREATE);
  sql << "insert into " << STORAGE_CONFIG.messageJournal() << "("
      << "message_id, uri, body_type, subscribers_count"
      << ")"
      << " values "
      << "("
      << " \'" << message.message_id() << "\'"
      << ",\'" << message.destination_uri() << "\'"
      << "," << message.body_type() << "," << dest.subscriptionsCount() << ")"
      << ";";
  session.currentDBSession = dbms::Instance().dbmsSessionPtr();
  session.currentDBSession->beginTX(message.message_id());
  TRY_POCO_DATA_EXCEPTION { (*session.currentDBSession) << sql.str(), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION("can't save message", sql.str(), session.currentDBSession.reset(nullptr), ERROR_ON_SAVE_MESSAGE)
  dest.save(session, sMessage);
}
const std::string &Exchange::destinationsT() const { return _destinationsT; }
void Exchange::removeConsumer(const std::string &sessionID, const std::string &destinationID, const std::string &subscriptionID, size_t tcpNum) {
  upmq::ScopedReadRWLock readRWLock(_destinationsLock);
  Destination &destination = getDestination(destinationID);
  destination.removeConsumer(sessionID, subscriptionID, tcpNum);
}
void Exchange::removeConsumer(const MessageDataContainer &sMessage, size_t tcpNum) {
  const Unsubscription &unsubscription = sMessage.unsubscription();
  const std::string destinationID = Exchange::mainDestinationPath(unsubscription.destination_uri());
  removeConsumer(unsubscription.session_id(), destinationID, unsubscription.subscription_name(), tcpNum);
}
void Exchange::begin(const upmq::broker::Session &session, const std::string &destinationID) {
  //  upmq::ScopedReadRWLock readRWLock(_destinationsLock);
  Destination &dest = destination(destinationID, DestinationCreationMode::NO_CREATE);
  dest.begin(session);
}
void Exchange::commit(const upmq::broker::Session &session, const std::string &destinationID) {
  upmq::ScopedReadRWLock readRWLock(_destinationsLock);
  Destination &destination = getDestination(destinationID);
  destination.commit(session);
  destination.postNewMessageEvent();
}
void Exchange::abort(const upmq::broker::Session &session, const std::string &destinationID) {
  upmq::ScopedReadRWLock readRWLock(_destinationsLock);
  Destination &destination = getDestination(destinationID);
  destination.abort(session);
  destination.postNewMessageEvent();
}
bool Exchange::isDestinationTemporary(const std::string &id) {
  upmq::ScopedReadRWLock readRWLock(_destinationsLock);
  Destination &destination = getDestination(id);
  return destination.isTemporary();
}

void Exchange::dropDestination(const std::string &id, DestinationOwner *owner) {
  upmq::ScopedWriteRWLock writeRWLock(_destinationsLock);
  auto it = _destinations.find(id);
  if (it != _destinations.end()) {
    if ((owner == nullptr) || (it->second->hasOwner() && owner->clientID == it->second->owner().clientID)) {
      _destinations.erase(it);
    }
  }
}
void Exchange::addSubscription(const upmq::broker::Session &session, const MessageDataContainer &sMessage) {
  Destination &dest = destination(sMessage.subscription().destination_uri(), DestinationCreationMode::NO_CREATE);
  if (dest.isBindToSubscriber(sMessage.clientID)) {
    dest.subscription(session, sMessage);
    std::stringstream sql;
    sql << "update " << _destinationsT << " set subscriptions_count = " << dest.subscriptionsTrueCount() << ";";
    TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
    CATCH_POCO_DATA_EXCEPTION_PURE_NO_EXCEPT("can't update subscriptions count", sql.str(), ERROR_ON_SUBSCRIPTION)
  } else {
    throw EXCEPTION("this destination was bound to another subscriber", dest.name() + " : " + sMessage.clientID, ERROR_ON_SUBSCRIPTION);
  }
}
void Exchange::addSender(const upmq::broker::Session &session, const MessageDataContainer &sMessage) {
  Destination &dest = destination(sMessage.sender().destination_uri(), DestinationCreationMode::NO_CREATE);
  if (dest.isBindToPublisher(sMessage.clientID)) {
    dest.addSender(session, sMessage);
  } else {
    throw EXCEPTION("this destination was bound to another publisher", dest.name() + " : " + sMessage.clientID, ERROR_ON_SUBSCRIPTION);
  }
}
void Exchange::removeSender(const upmq::broker::Session &session, const MessageDataContainer &sMessage) {
  const Unsender &unsender = sMessage.unsender();
  if (unsender.destination_uri().empty()) {
    removeSenderFromAnyDest(session, unsender.sender_id());
  } else {
    try {
      Destination &dest = destination(unsender.destination_uri(), DestinationCreationMode::NO_CREATE);
      dest.removeSender(session, sMessage);
    } catch (Exception &) {  // do nothing -V565
    }
  }
}
void Exchange::removeSenders(const upmq::broker::Session &session) {
  upmq::ScopedReadRWLock readRWLock(_destinationsLock);
  for (const auto &dest : _destinations) {
    dest.second->removeSenders(session);
  }
}
void Exchange::removeSenderFromAnyDest(const upmq::broker::Session &session, const std::string &senderID) {
  upmq::ScopedReadRWLock readRWLock(_destinationsLock);
  for (const auto &dest : _destinations) {
    dest.second->removeSenderByID(session, senderID);
  }
}
void Exchange::start() {
  _threadAdapter = std::make_unique<Poco::RunnableAdapter<Exchange>>(*this, &Exchange::run);
  int count = _threadPool.capacity() - 1;
  _isRunning = true;
  for (int i = 0; i < count; ++i) {
    _threadPool.start(*_threadAdapter);
  }
}
void Exchange::stop() {
  if (_isRunning) {
    _isRunning = false;
    _threadPool.joinAll();
  }
}
void Exchange::postNewMessageEvent(const std::string &uri) const {
  const int count = _threadPool.capacity() - 1;

  if (!uri.empty()) {
    _destinationEvents.enqueue(mainDestinationPath(uri));
  }

  for (size_t i = 0; i < static_cast<size_t>(count); ++i) {
    Poco::ScopedLock<Poco::FastMutex> lock(_mutexDestinations[i]);
    _conditionDestinations[i].signal();
  }
}
void Exchange::run() {
  size_t num = _thrNum++;

  bool result = false;
  std::string queueId;
  while (_isRunning) {
    do {
      result = false;
      {
        upmq::ScopedReadRWLock readRWLock(_destinationsLock);
        do {
          queueId.clear();
          if (_destinationEvents.try_dequeue(queueId)) {
            if (!queueId.empty()) {
              auto item = _destinations.find(queueId);
              if (item != _destinations.end()) {
                item->second->getNexMessageForAllSubscriptions();
              }
            }
          }
        } while (!queueId.empty());
      }

      {
        upmq::ScopedReadRWLock readRWLock(_destinationsLock);
        for (const auto &dest : _destinations) {
          if (dest.second->getNexMessageForAllSubscriptions() && !result) {
            result = true;
          }
        }
      }
    } while (result);

    auto &mut = _mutexDestinations[num];

    Poco::ScopedLock<Poco::FastMutex> lock(mut);
    _conditionDestinations[num].tryWait(mut, 1000);
  }
}
std::vector<Destination::Info> Exchange::info() const {
  std::vector<Destination::Info> infos;
  std::map<size_t, std::vector<Destination::Info>> infosGroup;

  auto containDigit = [](const std::string &s) {
    bool has = false;
    std::for_each(s.begin(), s.end(), [&](const char &c) {
      if (std::isdigit(c)) has = true;
    });
    return has;
  };

  {
    upmq::ScopedReadRWLock readRWLock(_destinationsLock);
    for (const auto &dest : _destinations) {
      auto info = dest.second->info();
      size_t sz = 0;
      if (containDigit(info.name)) {
        sz = info.name.size();
      }
      infosGroup[sz].emplace_back(std::move(info));
    }
  }
  std::stringstream sql;
  sql << "select id, name, type, create_time from " << _destinationsT;
  // if (!infos.empty()) {
  //   sql << " where ";
  //   size_t lastIndex = infos.size() - 1;
  //   for (size_t i = 0; i < infos.size(); ++i) {
  //     sql << " id <> '" << infos[i].id << "'";
  //     if (i < lastIndex) {
  //       sql << " and ";
  //     }
  //   }
  // }
  storage::DBMSSession dbSession = dbms::Instance().dbmsSession();
  Poco::Data::Statement select(dbSession());
  Destination::Info destInfo;
  TRY_POCO_DATA_EXCEPTION {
    select << sql.str(), Poco::Data::Keywords::into(destInfo.id), Poco::Data::Keywords::into(destInfo.name), Poco::Data::Keywords::into(*((int *)&destInfo.type)),
        Poco::Data::Keywords::into(destInfo.created), Poco::Data::Keywords::range(0, 1);
    while (!select.done()) {
      select.execute();
      if (!destInfo.name.empty() && !destInfo.id.empty()) {
        if (destInfo.name.find(TEMP_QUEUE_PREFIX "/") != std::string::npos) {
          Poco::replaceInPlace(destInfo.name, TEMP_QUEUE_PREFIX "/", "");
        } else if (destInfo.name.find(TEMP_TOPIC_PREFIX "/") != std::string::npos) {
          Poco::replaceInPlace(destInfo.name, TEMP_TOPIC_PREFIX "/", "");
        } else if (destInfo.name.find(QUEUE_PREFIX "/") != std::string::npos) {
          Poco::replaceInPlace(destInfo.name, QUEUE_PREFIX "/", "");
        } else if (destInfo.name.find(TOPIC_PREFIX "/") != std::string::npos) {
          Poco::replaceInPlace(destInfo.name, TOPIC_PREFIX "/", "");
        }

        destInfo.uri = Poco::toLower(Destination::typeName(destInfo.type)) + "://" + destInfo.name;
        destInfo.dataPath = Exchange::mainDestinationPath(destInfo.uri);
        size_t sz = 0;
        if (containDigit(destInfo.name)) {
          sz = destInfo.name.size();
        }
        auto resultInfo = std::find_if(infosGroup[sz].begin(), infosGroup[sz].end(), [&destInfo](const Destination::Info &info) { return info.name == destInfo.name; });
        if (resultInfo == infosGroup[sz].end()) {
          infosGroup[sz].emplace_back(destInfo);
        }
      }
    }
  }
  CATCH_POCO_DATA_EXCEPTION_PURE_NO_EXCEPT("can't get destinations info", sql.str(), ERROR_STORAGE)

  for (auto &item : infosGroup) {
    std::sort(item.second.begin(), item.second.end(), [](const Destination::Info &l, const Destination::Info &r) { return (l.name < r.name); });
    std::for_each(item.second.begin(), item.second.end(), [&](const Destination::Info &info) { infos.emplace_back(info); });
  }

  return infos;
}
}  // namespace broker
}  // namespace upmq