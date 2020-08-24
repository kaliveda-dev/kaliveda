#ifndef KVZMQMESSAGE_H
#define KVZMQMESSAGE_H
#include "TMessage.h"
#include "TClass.h"
#include "zmq.hpp"
#include "KVConfig.h"

/**
 * \class KVZMQMessage
 * \ingroup Infrastructure
 * \brief Allow to send/receive ROOT/KV objects between ZeroMQ sockets
 *
 * ## Examples of use
 * ### Sending an object
 *
 * ~~~~~~~~~~~~~~{.cpp}
 * // ZMQ socket
 * zmq::socket_t socket;
 *
 * // pointer to object
 * TObject* obj;
 *
 * KVZMQMessage message;
 * message.WriteObject(obj);
 * socket.send(message.GetMessage());
 * ~~~~~~~~~~~~~~
 *
 * ### Receiving an object
 *
 * ~~~~~~~~~~~~~~{.cpp}
 * // ZMQ socket
 * zmq::socket_t socket;
 *
 * // ZMQ message
 * zmq::message_t zmq_message;
 *
 * if(socket.recv(&zmq_message))
 * {
 *    KVZMQMessage message(zmq_message);
 *    TObject* object = message.GetObject();
 * }
 * ~~~~~~~~~~~~~~
 */

class KVZMQMessage : public TMessage {
   unique_ptr<TObject> fObject;
   Bool_t fReadNewObject;
   zmq::message_t fMessage;

public:
   KVZMQMessage()
      : TMessage(kMESS_OBJECT)
   {
      fReadNewObject = kFALSE;
   }
   KVZMQMessage(zmq::message_t& g)
      : TMessage(g.data(), g.size())
   {
      // stop the underlying TBuffer from deleting the buffer
      // which in our case belongs to the zmq::message_t object
      ResetBit(kIsOwner);
      fReadNewObject = kTRUE;
   }
   virtual ~KVZMQMessage() {}
#if ROOT_VERSION_CODE < ROOT_VERSION(6,12,0)
   virtual void WriteObject(const TObject* obj)
   {
      TMessage::WriteObject(obj);
#else
   virtual void WriteObject(const TObject* obj, Bool_t cacheReuse = kTRUE)
   {
      TMessage::WriteObject(obj, cacheReuse);
#endif
      SetLength();
   }
   zmq::message_t& GetMessage()
   {
      fMessage.rebuild(Buffer(), Length());
      return fMessage;
   }
   TObject* GetObject()
   {
      if (fReadNewObject) {
         fObject.reset(ReadObject(GetClass()));
         fReadNewObject = kFALSE;
      }
      return fObject.get();
   }
   template <class T>
   T* GetObject()
   {
      if (GetClass()->InheritsFrom(T::Class())) {
         return (T*)(GetObject());
      }
      return nullptr;
   }
};

#endif // KVMESSAGE_H
