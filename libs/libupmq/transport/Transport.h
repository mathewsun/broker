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

#ifndef UPMQ_TRANSPORT_TRANSPORT_H_
#define UPMQ_TRANSPORT_TRANSPORT_H_

#include <decaf/io/Closeable.h>
#include <decaf/io/IOException.h>
#include <decaf/util/List.h>

#include <decaf/lang/Pointer.h>
#include <decaf/lang/exceptions/UnsupportedOperationException.h>
#include <transport/Command.h>
#include <transport/Config.h>
#include <transport/FutureResponse.h>
#include <transport/Response.h>
#include <transport/ResponseCallback.h>
#include <transport/Service.h>
#include <typeinfo>

namespace upmq {
namespace transport {
class WireFormat;
}
namespace transport {

// Forward declarations.
class TransportListener;

using decaf::lang::Pointer;
using upmq::transport::Command;
using upmq::transport::Response;

/**
 * Interface for a transport layer for command objects.  Callers can
 * send oneway messages or make synchronous requests.  Non-response
 * messages will be delivered to the specified listener object upon
 * receipt.  A user of the Transport can set an exception listener
 * to be notified of errors that occurs in Threads that the Transport
 * layer runs.  Transports should be given an instance of a WireFormat
 * object when created so that they can turn the built in Commands to /
 * from the required wire format encoding.
 */
class UPMQCPP_API Transport : public upmq::transport::Service, public decaf::io::Closeable {
 public:
  ~Transport() override = default;;

  /**
   * Starts the Transport, the send methods of a Transport will throw an exception
   * if used before the Transport is started.
   *
   * @throw IOException if and error occurs while starting the Transport.
   */
  void start() override = 0;

  /**
   * Stops the Transport.
   *
   * @throw IOException if an error occurs while stopping the transport.
   */
  void stop() override = 0;

  /**
   * Sends a one-way command.  Does not wait for any response from the
   * broker.
   *
   * @param command
   *      The command to be sent.
   *
   * @throws IOException if an exception occurs during writing of the command.
   * @throws UnsupportedOperationException if this method is not implemented
   *         by this transport.
   */
  virtual void oneway(Pointer<Command> command) = 0;

  /**
   * Sends a commands asynchronously, returning a FutureResponse object that the caller
   * can use to check to find out the response from the broker.
   *
   * @param command
   *      The Command object that is to sent out.
   * @param responseCallback
   *      A callback object that will be notified once a response to the command is received.
   *
   * @return A FutureResponse instance that can be queried for the Response to the Command.
   *
   * @throws IOException if an exception occurs during the read of the command.
   * @throws UnsupportedOperationException if this method is not implemented
   *         by this transport.
   */
  virtual Pointer<FutureResponse> asyncRequest(Pointer<Command> command, Pointer<ResponseCallback> responseCallback) = 0;

  /**
   * Sends the given command to the broker and then waits for the response.
   *
   * @param command the command to be sent.
   *
   * @return the response from the broker.
   *
   * @throws IOException if an exception occurs during the read of the command.
   * @throws UnsupportedOperationException if this method is not implemented
   *         by this transport.
   */
  virtual Pointer<Response> request(Pointer<Command> command) = 0;

  /**
   * Sends the given command to the broker and then waits for the response.
   *
   * @param command
   *      The command to be sent.
   * @param timeout
   *      The time to wait for this response.
   *
   * @return the response from the broker.
   *
   * @throws IOException if an exception occurs during the read of the command.
   * @throws UnsupportedOperationException if this method is not implemented
   *         by this transport.
   */
  virtual Pointer<Response> request(Pointer<Command> command, unsigned int timeout) = 0;

  /**
   * Gets the WireFormat instance that is in use by this transport.  In the case of
   * nested transport this method delegates down to the lowest level transport that
   * actually maintains a WireFormat info instance.
   *
   * @return The WireFormat the object used to encode / decode commands.
   */
  virtual Pointer<transport::WireFormat> getWireFormat() const = 0;

  /**
   * Sets the WireFormat instance to use.
   * @param wireFormat
   *      The WireFormat the object used to encode / decode commands.
   */
  virtual void setWireFormat(Pointer<transport::WireFormat> wireFormat) = 0;

  /**
   * Sets the observer of asynchronous events from this transport.
   * @param newListener the listener of transport events.
   */
  virtual void setTransportListener(TransportListener *newListener) = 0;

  /**
   * Gets the observer of asynchronous events from this transport.
   * @return the listener of transport events.
   */
  virtual TransportListener *getTransportListener() const = 0;

  /**
   * Narrows down a Chain of Transports to a specific Transport to allow a
   * higher level transport to skip intermediate Transports in certain
   * circumstances.
   *
   * @param typeId - The type_info of the Object we are searching for.
   *
   * @return the requested Object. or NULL if its not in this chain.
   */
  virtual Transport *narrow(const std::type_info &typeId) = 0;

  /**
   * Is this Transport fault tolerant, meaning that it will reconnect to
   * a broker on disconnect.
   *
   * @return true if the Transport is fault tolerant.
   */
  virtual bool isFaultTolerant() const = 0;

  /**
   * Is the Transport Connected to its Broker.
   *
   * @return true if a connection has been made.
   */
  virtual bool isConnected() const = 0;

  /**
   * Has the Transport been shutdown and no longer usable.
   *
   * @return true if the Transport
   */
  virtual bool isClosed() const = 0;

  /**
   * @return true if reconnect is supported.
   */
  virtual bool isReconnectSupported() const = 0;

  /**
   * @return true if updating uris is supported.
   */
  virtual bool isUpdateURIsSupported() const = 0;

  /**
   * @return the remote address for this connection
   */
  virtual std::string getRemoteAddress() const = 0;

  /**
   * reconnect to another location
   *
   * @param uri
   *      The new URI to connect this Transport to.
   *
   * @throws IOException on failure or if reconnect is not supported.
   */
  // virtual void reconnect(const decaf::net::URI &uri) = 0;

  /**
   * Updates the set of URIs the Transport can connect to.  If the Transport
   * doesn't support updating its URIs then an IOException is thrown.
   *
   * @param rebalance
   *      Indicates if a forced reconnection should be performed as a result of the update.
   * @param uris
   *      The new list of URIs that can be used for connection.
   *
   * @throws IOException if an error occurs or updates aren't supported.
   */
  // virtual void updateURIs(bool rebalance, const decaf::util::List<decaf::net::URI> &uris) = 0;
};
}  // namespace transport
}  // namespace upmq

#endif /*_UPMQ_TRANSPORT_TRANSPORT_H_*/
